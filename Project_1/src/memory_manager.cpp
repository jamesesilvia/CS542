/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 1
 *
 * memory_manager.cpp
 *
 **********************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

using namespace std;

#include "memory_manager.hpp"

/* constructor */
Memory_manager::Memory_manager(string table) {
    name = table;
    filename = table + ".dat";
    map_loc = table + ".txt";
    map = NULL;
    size = 0;
    filled = 0;
}

/* map database into memory */
int Memory_manager::map_to_memory(int database_size) {

    /* open database table */
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        cout << __func__ << "(): could not open database table file" << endl;
        return -1;
    }

    /* get length of table */
    if (fstat(fd, &filestat) == -1) {
        cout << __func__ << "(): could not get database file stats" << endl;   
        return -1;
    }
    size = filestat.st_size;

    /* increase file size to starting table size */
    if (size < database_size) {
        if (ftruncate(fd, database_size) == -1) {
            cout << __func__ << "(): could not expand database file" << endl;   
        } else {
            size = database_size;
        }
    }

    /* map file into memory */
    map = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        cout << __func__ << "(): memory map failed" << endl;
        close(fd);
        return -1;
    }

    /* close file descriptor, we don't need it anymore */
    if (close(fd) == -1) {
        cout << __func__ << "(): could not close file descriptor" << endl;   
        return -1;
    }

    return 0;
}

/* unmap database from memory */
int Memory_manager::unmap_from_memory() {

    /* unmap file from memory */
    if (munmap(map, size) == -1) {
        cout << __func__ << "(): memory unmap failed" << endl;   
        return -1;
    }

    return 0;
}

/* read directly from memory map */
int Memory_manager::read(char *buffer, int offset, int len) {

    /* raw read from memory map */
    if (map) {
        memcpy(buffer, (void *)(map + offset), len);
    } else {
        cout << __func__ << "(): memory map does not exist" << endl;   
        return -1;
    }

    return 0;
}

/* write directly to memory map */
int Memory_manager::write(char *buffer, int offset, int len) {

    /* raw write to memory map and sync to file */
    if (map) {
        memcpy((void *)(map + offset), buffer, len);
        if (msync((void *)(map), size, MS_SYNC) == -1) {
            cout << __func__ << "(): msync failed" << endl;  
            return -1;
        }
    } else {
        cout << __func__ << "(): memory map does not exist" << endl;   
        return -1;
    }

    return 0;
}

/* find a free block of memory in the database */
int Memory_manager::get_free_memory_block(int req_size, int *ret_size) {
   
    int last_offset = -1;
    int curr_offset = -1;
    int curr_size = -1;
    int biggest_size = -1;
    int biggest_offset = -1;
    int free_at_beg;
    int free_at_end;

    if (!ret_size) {
        cout << __func__ << "(): ret_size is NULL" << endl;
    }

    /* if table is empty return offset zero */
    if (table.empty()) {
        if (req_size < size) {
            *ret_size = req_size;
        } else {
            *ret_size = size;
        }
        return 0;
    }

    /* find open block of memory of given req_size */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if (last_offset == -1) {
            last_offset = (*iter).offset + (*iter).length;
            /* check for open space before first block */
            if ((*iter).offset != 0) {
                free_at_beg = (*iter).offset;
                if (free_at_beg >= req_size) {
                    *ret_size = req_size;
                    return 0;
                } else {
                    biggest_size = free_at_beg;
                    biggest_offset = 0;
                }
            }
        }

        curr_offset = (*iter).offset;
        curr_size = curr_offset - last_offset;

        if (curr_size >= req_size) {
            *ret_size = req_size;
            return last_offset;
        } else if (curr_size > biggest_size) {
            biggest_size = curr_size;
            biggest_offset = last_offset;
        }
        
        last_offset = (*iter).offset + (*iter).length;

    }

    /* we got to end of list but there may be more memory free at the end */
    free_at_end = size - last_offset;
    if (free_at_end >= req_size) {
        *ret_size = req_size;
        return last_offset;
    } else if (free_at_end > biggest_size) {
        biggest_size = free_at_end;
        biggest_offset = last_offset;
    }

    /* didn't find an open block of a req_size we needed, return largest */
    *ret_size = biggest_size;
    return biggest_offset;
}

/* get total of all fragmented lengths of a given index */
int Memory_manager::get_index_length(int index) {
    int total_length = 0;

    /* get the length of an index in the memory map */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter).index == index) {
            total_length += (*iter).length;
        }
    }
    return total_length;
}

/* check to see if an index exists */
bool Memory_manager::index_exist(int index) {

    /* see if this index exists */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter).index == index) {
            return true;
        }
    }
    return false;
}

/* write new index into table that keeps track of what is in the database */
list<value>::iterator Memory_manager::write_to_table(int index, int offset, int length, int fragment, value *next_frag) {
    value val;
    val.index = index;
    val.offset = offset;
    val.length = length;
    val.fragment = fragment;
    val.next_frag = next_frag;
    list<value>::iterator i;

    if (table.empty()) {
        table.push_front(val);
        return table.begin();
    }

    /* we need to put the new element in the correct spot,
     * we order the elements by memory offset
     */
    for (i = table.begin(); i != table.end(); ++i) {
        if ((*i).offset > offset) {
            return table.insert(i,val);
        }
    }

    /* we must belong at the end then */
    table.push_back(val);
    return table.end();

}

/* read an index from the database */
int Memory_manager::read_index(char *buffer, int index) {
    int offset = 0;
    value *ptr;

    /* read index from database, it may be fragmented so iterate
     * entire table until we find fragment 1 then use fragment
     * pointers to find the rest of them
     */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter).index == index && (*iter).fragment == 1) {
            ptr = (value *)&(*iter);
            while (ptr != NULL) {
                if (read((buffer + offset), ptr->offset, ptr->length) == -1) {
                    cout << __func__ << "(): read failure" << endl;
                    return -1;
                }
                offset += ptr->length;
                ptr = ptr->next_frag;
            }
            return 0;
        }
    }
    
    return -1;
}

/* write an index to the database */
int Memory_manager::write_index(char *buffer, int index, int len) {
    int free_block_size;
    int free_block_offset;
    int left_to_write = len;
    int fragment = 1;
    int buffer_offset = 0;
    int free_space_left;
    list<value>::iterator i;
    list<value>::iterator i_last;
    value *ptr = NULL;
    value *ptr_last = NULL;
    
    /* see if this index already exists */
    if (index_exist(index)) {
        return -1;
    }

    /* see if we have enough room to write this request */
    free_space_left = get_free_space();
    if (free_space_left < len) {
        if (expand_database(len) == -1) {
            cout << __func__ << "(): expand failed for request size: " << len
                 << ", free space left: " << free_space_left << endl;
        }
    }

    while (left_to_write > 0) {

        /* get free block */
        free_block_offset = get_free_memory_block(left_to_write, &free_block_size);
       
        /* add block to memory map */
        if (fragment > 1) {
            i_last = i;
        }
        i = write_to_table(index, free_block_offset, free_block_size, fragment, NULL);

        /* if fragmented add pointer from last fragment to this fragment */
        if (fragment > 1) {
            ptr = (value *)&(*i);
            ptr_last = (value *)&(*i_last);
            ptr_last->next_frag = ptr;
        }

        /* write to database */
        if (write((buffer + buffer_offset), free_block_offset, free_block_size) == -1) {
            cout << __func__ << "(): write failure" << endl;
        }

        left_to_write -= free_block_size;
        buffer_offset += free_block_size;
        filled += free_block_size;
        fragment += 1;

    }

    /* save changes to persistent memory map */
    save_memory_map();

    return 0;
}

/* remove an index from database, it just removes the index from the table,
 * the data is still in the database itself but it is now marked as free so
 * it can be overwritten by another write
 */
int Memory_manager::remove_index(int index) {
    list<value>::iterator i;
    
    if (!index_exist(index)) {
        return -1;
    }

    /* remove from table */
    i = table.begin();
    while (i != table.end()) {
        if ((*i).index == index) {
            filled -= (*i).length;
            i = table.erase(i);
        } else {
            ++i;
        }
    }

    /* save changes to persistent memory map */
    save_memory_map();

    return 0;
}

/* print out state of memory map for debug purposes */
void Memory_manager::print_memory_map() {
    
    cout << "********** Memory Map **********" << endl;
    cout << "Current size: " << size << ", bytes filled: " << filled << endl;
    float fill_percent = ((float)filled)/((float)size);
    cout << "Percentage filled: " << fill_percent << endl;

    for (iter = table.begin(); iter != table.end(); ++iter) {
        cout << "Index: " << (*iter).index
             << ", Fragment: " << (*iter).fragment
             << ", Offset: " << (*iter).offset
             << ", Length: " << (*iter).length
             << endl;
    }
    
    cout << "********************************" << endl;
}

/* save memory map to text file */
void Memory_manager::save_memory_map() {
    ofstream text_map;
    
    remove(map_loc.c_str());

    text_map.open(map_loc.c_str());
    if (!text_map) {
        cout << __func__ << "(): unable to open text map" << endl;
        return;
    }

    for (iter = table.begin(); iter != table.end(); ++iter) {
        text_map << (*iter).index << ","
                 << (*iter).fragment << ","
                 << (*iter).offset << ","
                 << (*iter).length
                 << endl;
    }

    text_map.close();
}

/* load memory map from text file */
int Memory_manager::load_memory_map() {
    string line;
    string val;
    int index;
    int offset;
    int length;
    int fragment;
    ifstream text_map;
    struct stat buffer;

    /* no texp map, return starting table size */
    if (stat(map_loc.c_str(), &buffer) == -1) {
        return START_TABLE_SIZE;
    }

    text_map.open(map_loc.c_str());
    if (!text_map) {
        return START_TABLE_SIZE;
    }
   
    if (!table.empty()) {
        cout << __func__ << "(): table is not empty" << endl;
    }

    /* parse information and fill it into the table */
    while (getline(text_map, line)) {
        stringstream ss(line);
        getline(ss, val, ',');
        index = atoi(val.c_str());
        getline(ss, val, ',');
        fragment = atoi(val.c_str());
        getline(ss, val, ',');
        offset = atoi(val.c_str());
        getline(ss, val, ',');
        length = atoi(val.c_str());
        filled += length;
        write_to_table(index,offset,length,fragment,NULL);
    }

    text_map.close();

    rebuild_links();

    return (filled == 0) ? START_TABLE_SIZE : filled;
}

/* after loading the table from the text file we need to relink
 * all the fragments back together, each fragment has a pointer
 * to the next fragment in line
 */
void Memory_manager::rebuild_links() {
    list<value>::iterator i;
    value *ptr = NULL;
    value *ptr_next = NULL;
    bool found;

    for (iter = table.begin(); iter != table.end(); ++iter) {
        
        for (i = table.begin(); i != table.end(); ++i) {

            if ((*iter).index == (*i).index && (*iter).fragment+1 == (*i).fragment) {
                ptr = (value *)&(*iter);
                ptr_next = (value *)&(*i);
                ptr->next_frag = ptr_next;
            }
        }
    }
}

/* get free space left in database */
int Memory_manager::get_free_space() {
    return size-filled;
}

/* expand database to new size */
int Memory_manager::expand_database(double request_size) {
    /* we expand the table in blocks, find out how many blocks
     * we need to be able to service this request */
    int blocks = (int)ceil(request_size/EXPAND_TABLE_SIZE);
    int expand_size = EXPAND_TABLE_SIZE * blocks;

    if (unmap_from_memory() == -1) {
        cout << __func__ << "(): unmap failed" << endl;
        return -1;
    }

    if (map_to_memory(size + expand_size) == -1) {
        cout << __func__ << "(): map failed" << endl;
        return -1;
    }

    return 0;
}
