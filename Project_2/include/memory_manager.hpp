/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 2
 *
 * memory_manager.hpp
 *
 **********************************************************************/

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <sys/fcntl.h> 
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

#include "bpt.hpp"
#include "bpt_string.hpp"

using namespace std;

#define MAX_TABLE_SIZE      2000000000   /* 2GB */
#define START_TABLE_SIZE       1000000   /* 1MB */
#define EXPAND_TABLE_SIZE         1000   /* 1kB */

/* structure to store the data */
struct container {
    int index;
    int population;
    char name[101];
} __attribute__((__packed__));

typedef struct container container_t;

#define CONTAINER_LENGTH sizeof(container_t)

/* Map table file into memory and keep track of indices in memory */
class Memory_manager {
public:
    /* functions */
    Memory_manager(string table);
    int map_to_memory(int database_size);
    int unmap_from_memory();
    int read(void *buffer, int location, int length);
    int write(void *buffer, int location, int length);
    int get_free_memory_block(int *location);
    bool index_exist(int index);
    void write_to_table(int index);
    int get_by_population(int pop, list<container_t>& container_list);
    int read_index(void *buffer, int index, int length);
    int put(int pop, const char *location);
    int write_index(container_t *container);
    int remove_index(int index);
    void rebuild_bptrees();
    void print_memory_map();
    void print_bpt();
    void print_db();
    void save_memory_map();
    int load_memory_map();
    int get_free_space();
    int expand_database(double request_size);

    static Memory_manager *instance() {
        if (!s_instance)
            s_instance = new Memory_manager("database");
        return s_instance;
    }
private:
    /* variables */
    static Memory_manager *s_instance;
    string name;
    string filename;
    string map_loc;
    struct stat filestat;
    char *map;
    int size;
    int filled;
    list<int> table;
    list<int>::iterator iter;

    /* b+ trees for indexing */
    Bpt population;
    Bpt city_name;
};

#endif
