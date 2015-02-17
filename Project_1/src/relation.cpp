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
#include "memory_manager.hpp"

using namespace std;

Relation::Relation(string _tablename) {
    tablename = _tablename;
    // init locks
    pthread_mutex_init(&s_lock, NULL);
    pthread_mutex_init(&d_lock, NULL);
    // spawn isolation manager
    spawn_isolation_manager();
}


bool Relation::put(int key, string data, int client) {

    int ret = false;

    // Create req
    request_t req = { key, data, client, PUT };

    // Add to service queue
    if (!add_to_queue(&s_lock, req, &service_queue))
        ret = true;

    return ret;
}


bool Relation::get(int key, int client) {
    
    int ret = false;

    // Create req
    request_t req = { key, "", client, GET };
    
    // Add to service queue
    if (!add_to_queue(&s_lock, req, &service_queue))
        ret = true;

    return ret;
}


bool Relation::remove(int key, int client) {
 
    int ret = false;

    // Create req
    request_t req = { key, "", client, REMOVE };

    // Add to service queue
    if (!add_to_queue(&s_lock, req, &service_queue))
        ret = true;    
    
    return ret;
}

bool Relation::add_to_queue(pthread_mutex_t *lock, 
                                        request_t req,
                                        list <request_t> *queue) {
    bool ret = true;

    // Get mutex
    pthread_mutex_lock(lock);

    // Add to end of list
    queue->push_back(req);

    // Did that fail?
    if (queue->empty())
        ret = false;

    // Release mutex
    pthread_mutex_unlock(lock);

    return ret;

}

request_t Relation::remove_from_queue(pthread_mutex_t *lock,
                                        list <request_t> *queue) {
    request_t req;
    req.action = 0;

    // Get mutex
    pthread_mutex_lock(lock);

    // Get first request and pop it
    if (!queue->empty()) {
        req = queue->front();
        queue->pop_front();
    }

    // Release mutex
    pthread_mutex_unlock(lock);

    return req;
}


request_t *Relation::remove_req_by_key(int key,
                                        int client,
                                        pthread_mutex_t *lock,
                                        list <request_t> *queue) {
    request_t *req = new request_t;
    req->action = 0;
    list <request_t>::iterator i = queue->begin();

    // Get mutex
    pthread_mutex_lock(lock);

    while (i != queue->end()) {
        if (i->key == key && i->client == client) {
            req->key = i->key;
            req->data = i->data;
            req->action = i->action;
            queue->erase(i);
            break;
        }
        i++;
    }

    // Release mutex
    pthread_mutex_unlock(lock);

    return req;
}


bool Relation::check_if_queue_empty(pthread_mutex_t *lock, 
                                        list <request_t> *queue) {
    bool ret = false;
    
    // Get mutex
    pthread_mutex_lock(lock);

    // Is it empty?
    if (queue->empty())
        ret = true;

    // Release mutex
    pthread_mutex_unlock(lock);

    return false;
}

request_t Relation::get_req_for_service() {

    // Create req
    request_t req;

    // Remove item for service
    req = remove_from_queue(&s_lock, &service_queue);

    return req;
}


bool Relation::req_service_done(request_t req) {
    int ret = false;

    // Add to done queue
    if (!add_to_queue(&d_lock, req, &done_queue))
        ret = true;

    return ret;
}



string Relation::wait_for_service(int key, int client) {
    
    request_t *req = NULL;
    stringstream to_send;

    /* Poll done queue for our request */
    while(true) {
        if (!done_queue.empty()){
            to_send.str("");
            req = remove_req_by_key(key, client, &d_lock, &done_queue);
            
            /* Respond to user */
            switch (req->action) {
                case(PUT):
                    to_send << "Storing key: " << req->key <<
                                " with data... " << req->data << endl;
                    break;
                case(GET):
                    to_send << "Getting key: " << req->key <<
                                " with data..." << req->data << endl;
                    break;
                case(REMOVE):
                    to_send << "Removing key: " << req->key <<
                                " with data..." << req->data << endl;
                    break;
                // Bogus key, do it again.
                default:
                    continue;
            }
            // OK, we got it.
            free(req);
            break;
        }
    }
    return to_send.str();
}

/*
 * isolation_manager()
 *
 * Gets requests from service queue
 * Services request to memory manager
 * Places request on done queue when complete
 *
 * Should not return. False on failure.
 */
bool Relation::isolation_manager() {
    int ret = 0;
    request_t req;
    Memory_manager *memory_manager = Memory_manager::instance();

    while(true) {
        // Do stuff if queue not empty
        if (!service_queue.empty()) {
            // Get the request at front of queue
            req = get_req_for_service();

            /* Handle request from user */
            switch(req.action ){
                case(PUT):
                // Need brackets for scope
                {
                    char *buffer = (char *)malloc(req.data.length()+1);
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(buffer, req.data.c_str());
                    // Store data in database
                    ret = memory_manager->write_index(buffer, 
                                                        req.key,
                                                        req.data.length());
                    // Response based on ret
                    if (ret == -1)
                        req.data = "\nFAILURE: That key already exists";
                    req.action = PUT;
                    free(buffer);
                    break;
                }
                case(GET):
                // Need brackets for scope
                {
                    int buffer_size = memory_manager->get_index_length(req.key);
                    char *buffer = (char *)malloc(buffer_size+1);
                    memset(buffer, 0, sizeof(buffer));
                    // Read data from database
                    ret = memory_manager->read_index(buffer,
                                                        req.key);
                    // Reponse based on ret
                    (ret == -1) ?
                        req.data = "\nREAD FAILED" :
                        req.data = string(buffer);
                        
                    req.action = GET;
                    free(buffer);
                    break;
                }
                case(REMOVE):
                // Need brackets for scope
                {
                    int buffer_size = memory_manager->get_index_length(req.key);
                    char *buffer = (char *)malloc(buffer_size+1);
                    memset(buffer, 0, sizeof(buffer));
                    // Read from database
                    ret = memory_manager->read_index(buffer,
                                                        req.key);
                    if (ret == -1)
                        req.data = "\nREAD FAILED (REMOVE).";
                    // Now remove it
                    ret = memory_manager->remove_index(req.key);
                    if (ret == -1)
                        req.data = "\n" + req.data + "REMOVE FAILED.";
                    // Update done item
                    req.data = req.data + string(buffer);
                    req.action = REMOVE;
                    break;
                }
                default:
                    continue;
            }
            // Request handled
            if (req_service_done(req))
                cout << "Failed to put on done queue" << endl;
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
void *start_isolation_manager(void *ptr) {
    Relation *spawn = (Relation*)ptr; 
    if (!spawn->isolation_manager()) {
        // Failed to start
        cout << "Isolation Manager returned, exiting..." << endl;
        exit(1);
    }
}


void Relation::spawn_isolation_manager(void) {
    if (pthread_create(&thread, NULL, 
                start_isolation_manager, this) < 0) {
        cout << "Failed to spawn Isolaton Manager." << endl;
        exit(1);
    }
}

void Relation::print_queues() {
    cout << "*************** SERVICE QUEUE **************" << endl; 
    // Iter list and print queue
    for (iter = service_queue.begin(); iter != service_queue.end(); ++iter) {
        cout << "Key: " << (*iter).key
            << " Data: " << (*iter).data
            << " Client: " << (*iter).client
            << " Action: " << (*iter).action << endl;
    }
    cout << "**************** DONE QUEUE ****************" << endl; 
    // Iter list and print queue
    for (iter = done_queue.begin(); iter != done_queue.end(); ++iter) {
        cout << "Key: " << (*iter).key
            << " Data: " << (*iter).data
            << " Client: " << (*iter).client
            << " Action: " << (*iter).action << endl;
    }
    cout << "********************************************" << endl;
}
