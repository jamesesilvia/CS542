/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 3
 *
 * database_server.hpp
 *
 **********************************************************************/

#ifndef DATABASE_SERVER_H
#define DATABASE_SERVER_H

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

#include "helpers.hpp"
#include "relation.hpp"
#include "memory_manager.hpp"

#define PORT 1433

#endif
