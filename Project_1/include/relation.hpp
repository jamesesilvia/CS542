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

#ifndef RELATION_H
#define RELATION_H

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
    bool put(int key, string data, int client);
    bool get(int key, int client);
    bool remove(int key, int client);
    string wait_for_service(int key);
    void print_queues();
    bool isolation_manager();

    static Relation *instance() {
        if (!s_instance)
            s_instance = new Relation("my table");
        return s_instance;
    }

private:
    /* variables */
    static Relation *s_instance;
    string tablename;
    int key;
    string data;
    int client;
    pthread_mutex_t s_lock;
    pthread_mutex_t d_lock;
    pthread_t thread;
    list<request_t>::const_iterator iter;
    /* functions */
    bool add_to_queue(pthread_mutex_t *lock,
                                request_t req,
                                list <request_t> *queue);
    request_t remove_from_queue(pthread_mutex_t *lock, 
                                list <request_t> *queue);
    request_t *remove_req_by_key(int key,
                                pthread_mutex_t *lock,
                                list <request_t> *queue);
    bool check_if_queue_empty(pthread_mutex_t *lock,
                                list <request_t> *queue);
    request_t get_req_for_service();
    bool req_service_done(request_t req);
    
    void spawn_isolation_manager();
    /* queues */
    list <request_t> service_queue;
    list <request_t> done_queue;
};

#endif
