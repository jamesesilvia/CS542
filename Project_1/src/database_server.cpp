/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * database_server.cpp
 *
 **********************************************************************/

#include "database_server.hpp"
#include "helpers.hpp"

//The backlog for listening
#define BACKLOG 5

//The buffer length for receiving commands
#define BUFFER_LEN 1024 

/* Singleton of each class */
Relation *Relation::s_instance = NULL;
Memory_manager *Memory_manager::s_instance = NULL;

//Set to false to disable
bool verbose = true;
//Verbose printing mode
void printv(char *format, ...) {

    if(verbose == false)
        return;

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


// Handles a client connection
bool handleclient(const int socket) {
    char buffer[BUFFER_LEN + 1] = {0}; //Allow for null.
    int len, request_id;
    string cmd, key, data_len, data, to_send;

    Relation *table = Relation::instance();

    while (true) {
        //Clear strings
        cmd.clear();
        data_len.clear();
        key.clear();
        data.clear();
        to_send.clear();
        bool got_size = false;
        int len_to_read = BUFFER_LEN;
        do {
            //Clear receive buffer
            memset(buffer, 0, sizeof(buffer));
            
            //Shall we get some data?
            len = read(socket, buffer, BUFFER_LEN);

            //Some sort of error
            if (len < 0) {
                cerr << "Unable to read from client socket!" << endl;
                break;
            }

            //Disconnected
            if (len == 0) {
                cerr << "The client disconnected!" << endl;
                return false;
            }
            
            //Get size of incoming data on first buffer
            if (!got_size) {
                //Stringstream of received data
                stringstream strstr(buffer);
                //Get control data from buffer
                if (!(strstr >> cmd))
                    continue;
                if (!(strstr >> key))
                    continue;
                if (!(strstr >> data_len))
                    continue;
                len_to_read = atoi(data_len.c_str());
                string temp;
                while (strstr >> temp) {
                    to_send = to_send + " " + temp;
                }
                got_size = true;
            }
            //Get buffer, decrease length of buffer left
            else {
                to_send = to_send + string(buffer);
                len_to_read = len_to_read - BUFFER_LEN;
            }
        } while (len_to_read > BUFFER_LEN);
        /* Put request received */
        if (cmd == "put") {
            printv("Putting: %s\n", key.c_str());
            // Put the entry
            request_id = table->put(atoi(key.c_str()), to_send);
            to_send.clear();
            // Wait for service
            to_send = table->wait_for_service(atoi(key.c_str()), request_id);
        } 
        /* Get request received */
        else if (cmd == "get") {
            printv("Getting: %s\n", key.c_str());
            // Get the entry
            request_id = table->get(atoi(key.c_str()));
            to_send.clear();
            // Wait for service
            to_send = table->wait_for_service(atoi(key.c_str()), request_id);
        }
        /* Remove request received */
        else if (cmd == "remove") {
            printv("Removing: %s\n", key.c_str());
            // Remove the entry
            request_id = table->remove(atoi(key.c_str()));
            to_send.clear();
            // Wait for service
            to_send = table->wait_for_service(atoi(key.c_str()), request_id);
        }
        /* Smoothly close socket, will slam all clients */
        else if (cmd == "close") {
            // Yes, lets exit.
            if (key == "server") {
                cout << "Received force close, EXIT" << endl;
                close(socket);
                exit(0);
            }
            to_send = "We got your phony CLOSE request brah";
        }
        /* Print queue - verbose only */
        else if (cmd == "print" && verbose) {
            table->print_queues();
            to_send = "We got your PRINT request brah";
        }
        /* Junk catch all */
        else {
            continue;
        }

        //Send the data
        len = write(socket, to_send.c_str(), to_send.length());
        if (len != to_send.length())
            error2("Unable to send the data!");
    }

    return true;
}


//function to handle new client as a new thread
void *handleclient_thread(void *newsock){

    int *sock = (int *)(newsock);
    
    handleclient(*sock);

    close(*sock);
}


//Main function!
int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    pthread_t thread;

    //Create memory manager for database
    Memory_manager *memory_manager = Memory_manager::instance();
    int current_size = memory_manager->load_memory_map();
    memory_manager->map_to_memory(current_size);

    //Create the sock handle
    printv("Creating socket\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        error("Unable to create sock!");
    
    //Clear the structures just incase
    memset(&cli_addr,  0, sizeof(cli_addr));
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    //Fill in the server structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    //Bind the socket to a local address
    printv("Binding socket\n");
    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Unable to bind sock!");

    //Begin listening on that port
    if (listen(sock, BACKLOG) < 0)
        error("Unable to listen on sock!");

    bool soldierOn = true;
    while (soldierOn)
    {
        //Accept a new connection!
        printv("Listening for connections...\n");
        int newsock = accept(sock, (struct sockaddr *)&cli_addr, &clilen);
        if (newsock < 0)
            error("Unable to accept connection!");

        //Start a new thread to handle clients
        printv("Connection made! Starting new thread...\n");
        if(pthread_create(&thread, NULL, handleclient_thread, (void*)&newsock) < 0)
            error("Unable to create new thread!\n");

    }

    //All done.
    memory_manager->unmap_from_memory();
    return 0;
}
