/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * relation.cpp
 *
 **********************************************************************/

#include "relation.hpp"

using namespace std;

Relation::Relation(string _tablename) {
    tablename = _tablename;
    pthread_mutex_init(&q_lock, NULL);
}

int Relation::put(int key, string data, int client) {

    int ret = 0;

    // Create req
    request_t req = { key, data, client, PUT };

    cout << req.data << endl;

    // Get mutex
    pthread_mutex_lock(&q_lock);

    // Add to end of list
    queue.push_back(req);

    // Did that fail?
    if (queue.empty())
        ret = 1;

    // Release mutex
    pthread_mutex_unlock(&q_lock);

    return ret;
}


int Relation::get(int key, int client) {
    
    int ret = 0;

    // Create req
    request_t req = { key, "", client, GET };

    // Get mutex
    pthread_mutex_lock(&q_lock);

    // Add to end of list
    queue.push_back(req);

    // Did that fail?
    if (queue.empty())
        ret = 1;

    // Release mutex
    pthread_mutex_unlock(&q_lock);

    return ret;
}


int Relation::remove(int key, int client) {
 
    int ret = 0;

    // Create req
    request_t req = { key, "", client, REMOVE };

    // Get mutex
    pthread_mutex_lock(&q_lock);

    // Add to end of list
    queue.push_back(req);

    // Did that fail?
    if (queue.empty())
        ret = 1;

    // Release mutex
    pthread_mutex_unlock(&q_lock);

    return ret;
}


request_t Relation::get_req_for_service(pthread_mutex_t lock) {

    // Create req
    request_t req;

    // Get mutex
    pthread_mutex_lock(&q_lock);

    // Get first request and pop it
    if (!queue.empty()) {
        req = queue.front();
        queue.pop_front();
    }

    // Release mutex
    pthread_mutex_unlock(&q_lock);

    return req;
}

int Relation::check_if_queue_empty() {
    if (queue.empty())
        return 1;
}


/*
 * isolation_manager()
 *
 * Gets requests from queue and services them.
 * 
 * Uses pipes to talk back to client handler
 *
 * Should not return. False on failure.
 */
bool Relation::isolation_manager() {
    request_t req;
    while(true) {
        // Do stuff if queue not empty
        if (!check_if_queue_empty()) {
            // Clear req.action
            req.action = 0;
            // Get the request at front of queue
            req = get_req_for_service(q_lock);

            /* Handle request from user */
            switch(req.action){
                case(PUT):
                    break;
                case(GET):
                    break;
                case(REMOVE):
                    break;
                default:
                    continue;
            }
        }
    }
    return false;
}


/*
 * start_isolation_manager()
 *
 * Used to initialize the Isolation Manager
 * 
 * If fails, exits server. 
 */
void *Relation::start_isolation_manager() {
    if (!isolation_manager()) {
        // Failed to start
        cout << "Isolation Manager failed to start, exiting..." << endl;
        exit(1);
    }
}


void Relation::print_queue() {
    // Iter list and print queue
    for (iter = queue.begin(); iter != queue.end(); ++iter) {
        cout << "trying to iter the list" << endl;
        cout << "Key: " << (*iter).key
            << " Data: " << (*iter).data
            << " Client: " << (*iter).client
            << " Action: " << (*iter).action << endl;

    }

}
