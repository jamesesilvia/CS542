/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 3
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
Memory_manager::Memory_manager(string table) : code() {
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
int Memory_manager::read(void *buffer, int location, int length) {

    /* raw read from memory map */
    if (map) {
        memcpy(buffer, (void *)(map + location), length);
    } else {
        cout << __func__ << "(): memory map does not exist" << endl;   
        return -1;
    }

    return 0;
}

/* write directly to memory map */
int Memory_manager::write(void *buffer, int location, int length) {

    /* raw write to memory map and sync to file */
    if (map) {
        memcpy((void *)(map + location), buffer, length);
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
int Memory_manager::get_free_memory_block(int *location) {

    int index = 0;
   
    /* if table is empty return offset zero */
    if (table.empty()) {
        *location = 0;
        return index;
    }

    /* find open block of memory of given req_size */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        // Loop until iter list done
        if (index != (*iter)){
            // 0, 128, 216...etc.
            *location = (index*CONTAINER_LENGTH);
            return index;
        }
        else{
            index++;
        }
    }
    
    /* list is full, return the end of list block */
    *location = (index*CONTAINER_LENGTH);
    return index;
}

/* check to see if an index exists */
bool Memory_manager::index_exist(int index) {

    /* see if this index exists */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter) == index) {
            return true;
        }
    }
    return false;
}

/* write new index into table that keeps track of what is in the database */
void Memory_manager::write_to_table(int index) {

    if (table.empty()) {
        table.push_front(index);
        return;
    }

    /* we need to put the new element in the correct spot,
     * we order the elements by memory offset
     */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter) > index) {
            table.insert(iter,index);
            return;
        }
    }

    /* we must belong at the end then */
    table.push_back(index);
    return;
}

#if 0
int Memory_manager::get_by_population(int pop, list<container_t>& container_list) {

    /* Get index from b+ tree */
    record *data = population.find(pop);
    container_t container;

    while (data) {

        /* This Fails if the index is removed, do stuff in relation */
        if (read_index((void *)&container, data->value, CONTAINER_LENGTH) == -1){
            cout << __func__ << "(): get failed" << endl;
            return -1;
        }
        container_list.push_back(container); 

        data = data->next;
    }

    return 0;
}

int Memory_manager::get_by_city_name(string name, list<container_t>& container_list) {

    /* Get index from b+ tree */
    record *data = city_name.find(name);
    container_t container;

    while (data) {

        /* This Fails if the index is removed, do stuff in relation */
        if (read_index((void *)&container, data->value, CONTAINER_LENGTH) == -1){
            cout << __func__ << "(): get failed" << endl;
            return -1;
        }
        container_list.push_back(container); 

        data = data->next;
    }

    return 0;
}
#endif

int Memory_manager::get_by_code(string name, list<container_t>& container_list) {

    /* Get index from b+ tree */
    record *data = code.find(name);
    container_t container;

    while (data) {

        /* This Fails if the index is removed, do stuff in relation */
        if (read_index((void *)&container, data->value, CONTAINER_LENGTH) == -1){
            cout << __func__ << "(): get failed" << endl;
            return -1;
        }
        container_list.push_back(container); 

        data = data->next;
    }

    return 0;
}


/* read an index from the database of container sizer*/
int Memory_manager::read_index(void *buffer, int index, int length) {
    int offset = 0;

    /* read index from database
     */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if ((*iter) == index) {
            // Read at location of said index
            if (read(buffer, (index*length), length) == -1) {
                cout << __func__ << "(): read failure" << endl;
                return -1;
            }
            return 0;
        }
    }
    return -1;
}


int Memory_manager::put(int pop, const char *location) {

    /* Malloc and fill packed container for storage */
    container_t *item = (container_t *)malloc(CONTAINER_LENGTH);
    item->population = pop;
    // We protect for null at client entry
    strcpy(item->name, location);

    write_index(item);
}

/* write an index to the database */
int Memory_manager::write_index(container_t *container) {
    int free_block;
    int free_space_left;
    int free_block_index;
    
    /* see if we have enough room to write this request */
    free_space_left = get_free_space();
    if (free_space_left < CONTAINER_LENGTH) {
        if (expand_database(CONTAINER_LENGTH) == -1) {
            cout << __func__ << "(): expand failed for request size: " << CONTAINER_LENGTH
                 << ", free space left: " << free_space_left << endl;
        }
    }

    /* get free block */
    free_block_index = get_free_memory_block(&free_block);
    write_to_table(free_block_index);
    filled += CONTAINER_LENGTH;

    /* Build up the container */
    container->index = free_block_index;

    /* write to database */
    if (write((void *)container, free_block, CONTAINER_LENGTH) == -1) {
        cout << __func__ << "(): write failure" << endl;
        // we didnt fill anything then
        filled -= CONTAINER_LENGTH;
    }

    /* add popluation to b+tree */
    //population.insert(container->population, container->index);
    /* add name to b+tree */
    //city_name.insert(container->name, container->index);
    /* add country code to b+tree */
    code.insert(container->code, container->index);

    // Free container
    free(container);

    /* save changes to persistent memory map */
    save_memory_map();

    return 0;
}

/* remove an index from database, it just removes the index from the table,
 * the data is still in the database itself but it is now marked as free so
 * it can be overwritten by another write
 */
int Memory_manager::remove_index(int index) {
    
    if (!index_exist(index)) {
        return -1;
    }

    /* find index so we can remove data from b+ trees */
    container_t container;
    if (read_index((void *)&container, index, CONTAINER_LENGTH) == -1){
        cout << __func__ << "(): get failed" << endl;
        return -1;
    }

    /* remove from b+ tree */
    //population.delete_node(container.population, index);
    /* remove from b+ tree */
    //string temp = container.name;
    //city_name.delete_node(temp, index);
    /* remove from b+ tree */
    string temp = container.code;
    code.delete_node(temp, index);

    for (iter = table.begin(); iter != table.end(); iter++) {
        if ((*iter) == index){
            filled -= CONTAINER_LENGTH;
            table.erase(iter);
            break;
        }
    }

    /* save changes to persistent memory map */
    save_memory_map();

    return 0;
}


/* read an index from the database of container sizer*/
void Memory_manager::rebuild_bptrees() {
    container_t container;

    /* rebuild b+ trees
     */
    for (iter = table.begin(); iter != table.end(); ++iter) {
        if (read_index((void *)&container, (*iter), CONTAINER_LENGTH) == -1){
            cout << __func__ << "(): get failed" << endl;
        }

        /* add population to b+tree */
        //population.insert(container.population, container.index);
        /* add name to b+tree */
        //city_name.insert(container.name, container.index);
        /* add name to b+tree */
        code.insert(container.code, container.index);
    }
}

/* print out state of memory map for debug purposes */
void Memory_manager::print_memory_map() {
    
    cout << "********** Memory Map **********" << endl;
    cout << "Current size: " << size << ", bytes filled: " << filled << endl;
    float fill_percent = ((float)filled)/((float)size);
    cout << "Percentage filled: " << fill_percent << endl;

    for (iter = table.begin(); iter != table.end(); ++iter) {
        cout << "Index: " << (*iter) << endl;
    }
    
    cout << "********************************" << endl;
}

/* print out state of memory map for debug purposes */
void Memory_manager::print_bpt() {
    
   /* cout << "********** Population B+ tree **********" << endl;

    population.print_tree();
    
    cout << "****************************************" << endl;
    
    cout << "********** City Name B+ tree **********" << endl;

    city_name.print_tree();
    
    cout << "****************************************" << endl;*/
    
    cout << "********** Country Code B+ tree **********" << endl;

    code.print_tree();
    
    cout << "****************************************" << endl;
}

/* print out state of memory map for debug purposes */
void Memory_manager::print_db() {

    print_memory_map();
    print_bpt();

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
        text_map << (*iter) << endl;
    }

    text_map.close();
}

/* load memory map from text file */
int Memory_manager::load_memory_map() {
    string line;
    int index;
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
        index = atoi(line.c_str());
        write_to_table(index);
    }

    text_map.close();

    return (filled == 0) ? START_TABLE_SIZE : filled;
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
