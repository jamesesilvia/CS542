/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * isolation_manager.cpp
 *
 **********************************************************************/

#include "isolation_manager.hpp"


/*
 * put_request_for_service
 *
 * Place client request in to queue for servicing.
 * 
 * Only the relation should do this.
 *
 * Return 0 on success, 1 on failure.
 *
 */
int put_request_for_service(request_t req)
{
    int ret = 0;

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


/*
 * get_request_to_service
 *
 * Grab request to service and remove from queue.
 *
 * Only the isolation manager should do this.
 *
 * Return request_t on success, NULL on failure.
 * 
 */
const request_t *get_request_for_service()
{
    const request_t *req = NULL;

    // Get mutex
    pthread_mutex_lock(&q_lock);

    // Get first request and pop it
    req = &(queue.front());
    queue.pop_front();

    // Release mutex
    pthread_mutex_unlock(&q_lock);

    return req;
}
