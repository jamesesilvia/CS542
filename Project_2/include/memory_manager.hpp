/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 1
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

using namespace std;

#define MAX_TABLE_SIZE      2000000000   /* 2GB */
#define START_TABLE_SIZE       1000000   /* 1MB */
#define EXPAND_TABLE_SIZE         1000   /* 1kB */

/* structure to hold index */
struct value {
    int index;
    int offset;
    int length;
    int fragment;
    value *next_frag;
};

/* Map table file into memory and keep track of indices in memory */
class Memory_manager {
public:
    /* functions */
    Memory_manager(string table);
    int map_to_memory(int database_size);
    int unmap_from_memory();
    int read(char *buffer, int offset, int len);
    int write(char *buffer, int offset, int len);
    int get_free_memory_block(int size, int *ret_size);
    int get_index_length(int index);
    bool index_exist(int index);
    list<value>::iterator write_to_table(int index, int offset, int length, int fragment, value *next_frag);
    int read_index(char *buffer, int index);
    int write_index(char *buffer, int index, int len);
    int remove_index(int index);
    void print_memory_map();
    void save_memory_map();
    int load_memory_map();
    void rebuild_links();
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
    list<value> table;
    list<value>::const_iterator iter;

};

#endif
