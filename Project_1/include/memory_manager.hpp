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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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
#define START_TABLE_SIZE    1000        /* 1000 Bytes */

/* structure to hold index */


/* Map table file into memory and keep track of indices in memory */
class Memory_manager {
    /* functions */
    Memory_manager(string file);
    int map_to_memory();
    int unmap_from_memory();
    int read(char *buffer, int offset, int len);
    int write(char *buffer, int offset, int len);

    /* variables */
    string filename;
    struct stat filestat;
    char *map;
    int size;

};

