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

#define MAX_TABLE_SIZE      2000000000  /* 2GB */
#define START_TABLE_SIZE    30        /* 1000 Bytes */

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
    int map_to_memory();
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
    void load_memory_map();
    void rebuild_links();

private:
    /* variables */
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

