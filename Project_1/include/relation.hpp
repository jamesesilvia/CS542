/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * relation.hpp
 *
 **********************************************************************/

#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <pthread.h>
#include <list>

using namespace std;

// Actions for queue
#define PUT     1
#define GET     2
#define REMOVE  3

/* User request item placed on queue */
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

/* Table object that puts requests on queue */
class Relation {
public:
    /* functions */
    Relation(string _tablename);
    int put(int key, string data, int client);
    int get(int key, int client);
    int remove(int key, int client);
    request_t get_req_for_service(pthread_mutex_t lock);
    int check_if_queue_empty();
    bool isolation_manager();
    void *start_isolation_manager();
    void print_queue();
    /* queue */
    list <request_t> queue;

private:
    /* variables */
    string tablename;
    int key;
    string data;
    int client;
    pthread_mutex_t q_lock;
    list<request_t>::const_iterator iter;
};


