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
    if (map) {
        memcpy(buffer, (void *)map[offset], len);
    } else {
        cout << __func__ << "(): memory map does not exist" << endl;   
        return -1;
    }

    return 0;
}

int Memory_manager::write(char *buffer, int offset, int len) {
    if (map) {
        memcpy((void *)map[offset], buffer, len);
        if (msync((void *)offset, len, MS_SYNC) == -1) {
            cout << __func__ << "(): msync failed" << endl;   
            return -1;
        }
    } else {
        cout << __func__ << "(): memory map does not exist" << endl;   
        return -1;
    }

    return 0;

}
