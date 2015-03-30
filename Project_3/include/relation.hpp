/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 3
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

#include "helpers.hpp"
#include "memory_manager.hpp"

using namespace std;

// Actions for queue
#define PUT     		    1
#define GET_INDEX_BY_NAME     	    2
#define GET_INDEX_BY_POPULATION     3
#define REMOVE  		    4

// Parse schema information
#define MAX_CHARS           1024
#define DELIMITER           ","

/* User request item placed on queue */
typedef struct request
{
    int key;
    int population;
    string data;
    int id;
    int action;

    string print() const
    {
        stringstream ss;
        ss << "Key: " << key << " Population: " << population << " Data: " << data 
            << " ID: " << id << " Action: " << action;
        return ss.str();

    }
} request_t;

/* Table object that puts requests on queue */
class Relation {
public:
    /* functions */
    Relation(string _tablename);
    int get_next_request_id();
    int put(int population, string data);
    int get_index_by_name(string data);
    int get_index_by_population(int population);
    int remove(int key);
    string wait_for_service(int req_id);
    void print_queues();
    bool init_db();
    bool open();
    bool close();
    bool isolation_manager();

    static Relation *instance_country() {
        if (!s_instance_country)
            s_instance_country = new Relation("country");
        return s_instance_country;
    }

    static Relation *instance_city() {
        if (!s_instance_city)
            s_instance_city = new Relation("city");
        return s_instance_city;
    }

private:
    /* variables */
    static Relation *s_instance_country;
    static Relation *s_instance_city;
    string tablename;
    int key;
    string data;
    int client;
    pthread_mutex_t s_lock;
    pthread_mutex_t d_lock;
    pthread_mutex_t r_lock;
    pthread_t thread;
    list<request_t>::const_iterator iter;
    int request_id;
    /* functions */
    bool add_to_queue(pthread_mutex_t *lock,
                                request_t req,
                                list <request_t> *queue);
    request_t remove_from_queue(pthread_mutex_t *lock, 
                                list <request_t> *queue);
    request_t *remove_req_by_id(int id,
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
    /* memory manager */
    Memory_manager db;
    bool load_city_data();
    bool load_country_data();
};

#endif
