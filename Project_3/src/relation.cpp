/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 3
 *
 * relation.cpp
 *
 **********************************************************************/

#include "relation.hpp"

using namespace std;

Relation::Relation(string _tablename) : db(_tablename) {
    tablename = _tablename;
    // init locks
    pthread_mutex_init(&s_lock, NULL);
    pthread_mutex_init(&d_lock, NULL);
    pthread_mutex_init(&r_lock, NULL);
    // spawn isolation manager
    spawn_isolation_manager();
    // init request ID to zero
    request_id = 0;
    // init next index ID to zero
    next_index = 0;
}

int Relation::get_next_request_id() {

    int ret;

    // Get mutex
    pthread_mutex_lock(&r_lock);
    
    ret = ++request_id;

    // Give mutex
    pthread_mutex_unlock(&r_lock);

    return ret;
}

int Relation::query(int percentage) {

    int id = get_next_request_id();

    // Create req
    request_t req = { 0, percentage, "", id, QUERY };

    // Add to service queue
    if (!add_to_queue(&s_lock, req, &service_queue))
        return -1;

    return id;
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


request_t *Relation::remove_req_by_id(int id, pthread_mutex_t *lock, list <request_t> *queue) {
    request_t *req = new request_t;
    req->action = 0;
    list <request_t>::iterator i = queue->begin();

    // Get mutex
    pthread_mutex_lock(lock);

    while (i != queue->end()) {
        if (i->id == id) {
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

    return ret;
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



string Relation::wait_for_service(int req_id) {
    
    request_t *req = NULL;
    stringstream to_send;
    int data_len;

    /* Poll done queue for our request */
    while(true) {
        if (!done_queue.empty()){
            to_send.str("");
            req = remove_req_by_id(req_id, &d_lock, &done_queue);
            data_len = req->data.length();
            /* Respond to user */
            switch (req->action) {
                case(QUERY):
                    to_send << "Query: " << data_len << 
                                " " << req->data << endl;
                    break;
                // Bogus action, do it again.
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

bool Relation::init_db() {

    if (tablename == "city"){
        if (!check_for_file("city.dat")) {
            fprintf(stderr, "Initializing city database...");
            open();
            load_city_data();
            close();
            cout << "Done!" << endl;
        } else {
            db.load_memory_map();
            open();
            db.rebuild_bptrees();
            close();
        }
    }
    else if (tablename == "country") {
        if (!check_for_file("country.dat")) {
            fprintf(stderr, "Initializing country database...");
            open();
            load_country_data();
            close();
            cout << "Done!" << endl;
        } else {
            db.load_memory_map();
            open();
            db.rebuild_bptrees();
            close();
        }
    }

    return true;
}

bool Relation::load_city_data() {
    ifstream fin;
    string inFileName = "schema/city_table.csv";

    fin.open(inFileName.c_str());
    if (!fin.good()) {
        string str = "Could not open " + inFileName;
        error(str.c_str());
    }
    
    char buffer[MAX_CHARS];
    while (fin.getline(buffer, MAX_CHARS)) {
        // Parse line
        string id, name, c_code, dist, population;
        id = strtok(buffer, DELIMITER);
        name = strtok(0, DELIMITER);
        c_code = strtok(0, DELIMITER);
        dist = strtok(0, DELIMITER);
        population = strtok(0, DELIMITER);

        // Store line
        container_t *data = (container_t *)malloc(CONTAINER_LENGTH);
        data->population = atoi(population.c_str());
        strncpy(data->name, name.c_str(), MAX_NAME);
        strncpy(data->code, c_code.c_str(), MAX_CODE);
        db.write_index(data);

        // Empty buffer for next line
        memset(buffer, 0, MAX_CHARS);
    }
    return true;
}

bool Relation::load_country_data() {
    ifstream fin;
    string inFileName = "schema/country_table.csv";

    fin.open(inFileName.c_str());
    if (!fin.good()) {
        string str = "Could not open " + inFileName;
        error(str.c_str());
    }

    char buffer[MAX_CHARS];
    while (fin.getline(buffer, MAX_CHARS)) {
        // Parse line
        string c_code, name, population;
        c_code = strtok(buffer, DELIMITER);
        name = strtok(0, DELIMITER);
        population = strtok(0, DELIMITER);

        // Store line
        container_t *data = (container_t *)malloc(CONTAINER_LENGTH);
        data->population = atoi(population.c_str());
        strncpy(data->name, name.c_str(), MAX_NAME);
        strncpy(data->code, c_code.c_str(), MAX_CODE);
        db.write_index(data);

        // Empty buffer for next line
        memset(buffer, 0, MAX_CHARS);
    }
}
bool Relation::open() {
    if (db.map_to_memory(START_TABLE_SIZE) == -1) {
        return false;
    } else {
        return true;
    }
}

bool Relation::close() {
    if (db.unmap_from_memory() == -1) {
        return false;
    } else {
        return true;
    }
}

bool Relation::get_next(container_t& next) {
    if (db.read_index((void *)&next, next_index, CONTAINER_LENGTH) == -1) {
        return false;
    } else {
        next_index++;
        return true;
    }
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
    Relation *city_table = Relation::instance_city();
    Relation *country_table = Relation::instance_country();

    while(true) {
        // Do stuff if queue not empty
        if (!service_queue.empty()) {
            // Get the request at front of queue
            req = get_req_for_service();

            /* Handle request from user */
            switch(req.action){
                case(QUERY):
                // Need brackets for scope
                {
                    stringstream ss;
                    list<container_t> data;
                    container_t city;                    
                    float percentage = req.percentage;
                                        
                    city_table->open();
                    country_table->open();

                    //Get the next city, only continue as long as there is one left to get
                    while(city_table->get_next(city)) {

                        //Get the country related to the city's country code
                        country_table->db.get_by_code(city.code, data);
                        float countrypop = data.front().population;
                        float citypop = city.population;

                        if ((countrypop*percentage*0.01) <= citypop) {
                            ss << city.index << " " << city.code << " " << city.name << " " << city.population << endl;
                            cout <<  city.name << " " << city.population << " vs. " << data.front().name << " " << data.front().population << endl;
                        }

                        data.clear();

                    }
                    country_table->close();
                    city_table->close();                    
                    
                    if(!ss.str().empty()){
                        req.data = ss.str();
                    }
                    else{
                        req.data = "NO RECORDS RETURNED";
                    }                    
                    
                    req.action = QUERY;                    
                    next_index = 0;
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
            << " ID: " << (*iter).id
            << " Action: " << (*iter).action << endl;
    }
    cout << "**************** DONE QUEUE ****************" << endl; 
    // Iter list and print queue
    for (iter = done_queue.begin(); iter != done_queue.end(); ++iter) {
        cout << "Key: " << (*iter).key
            << " Data: " << (*iter).data
            << " ID: " << (*iter).id
            << " Action: " << (*iter).action << endl;
    }
    cout << "********************************************" << endl;
}
