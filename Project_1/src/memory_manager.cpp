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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

using namespace std;

#include "memory_manager.hpp"

Memory_manager::Memory_manager(string file) {
    filename = file;
    map = NULL;
    size = 0;
}

int Memory_manager::map_to_memory() {

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
    if (size < START_TABLE_SIZE) {
        if (ftruncate(fd, START_TABLE_SIZE) == -1) {
            cout << __func__ << "(): could not expand database file" << endl;   
        } else {
            size = START_TABLE_SIZE;
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

int Memory_manager::unmap_from_memory() {

    /* unmap file from memory */
    if (munmap(map, size) == -1) {
        cout << __func__ << "(): memory unmap failed" << endl;   
        return -1;
    }

    return 0;
}

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

bool Memory_manager::index_exist(int index) {

    /* see if this index exists */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter).index == index) {
            return true;
        }
    }
    return false;
}

list<value>::iterator Memory_manager::write_to_table(int index, int offset, int length, int fragment, value *next_frag) {
    value val;
    val.index = index;
    val.offset = offset;
    val.length = length;
    val.fragment = fragment;
    val.next_frag = next_frag;
    list<value>::iterator i;

    if (table.empty()) {
        cout << __func__ << "(): table is empty, adding to front" << endl;
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

int Memory_manager::write_index(char *buffer, int index, int len) {
    int free_block_size;
    int free_block_offset;
    int left_to_write = len;
    int fragment = 1;
    int buffer_offset = 0;
    list<value>::iterator i;
    list<value>::iterator i_last;
    value *ptr = NULL;
    value *ptr_last = NULL;
    
    /* see if this index already exists */
    if (index_exist(index)) {
        return -1;
    }

    while (left_to_write > 0) {

        /* get free block */
        free_block_offset = get_free_memory_block(left_to_write, &free_block_size);
        cout << __func__ << "(): found block at offset: " << free_block_offset << ", len: " << free_block_size << endl;
       
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
        fragment += 1;

    }

    return 0;
}

void Memory_manager::print_memory_map() {
    
    cout << "********** Memory Map **********" << endl;

    for (iter = table.begin(); iter != table.end(); ++iter) {
        cout << "Index: " << (*iter).index
             << ", Fragment: " << (*iter).fragment
             << ", Offset: " << (*iter).offset
             << ", Length: " << (*iter).length
             << endl;
    }
    
    cout << "********************************" << endl;
}
