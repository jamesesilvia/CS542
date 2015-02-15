/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * isolation_manager.hpp
 *
 **********************************************************************/

#include <string>
#include <sstream>
#include <iostream>
#include <list>
#include <pthread.h>

using namespace std;

// Actions for queue
#define PUT     0
#define GET     1
#define REMOVE  2

// User request item placed on queue
typedef struct request
{
    int key;
    string data;
    int client;
    int action;

    string print() const
    {
        stringstream ss;
        ss << "Key: " << key << " Data: " << data 
            << " Client: " << client << " Action: " << action;
        return ss.str();

    }
} request_t;

// Global queue that holds requests
static list<request_t> queue;

// Mutex for queue
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
