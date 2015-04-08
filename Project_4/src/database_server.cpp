/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 4
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

/* Relations for each table */
Relation *Relation::s_instance_country = NULL;
Relation *Relation::s_instance_city = NULL;

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
    int len, request_id, start, end;
    string cmd, percentage, to_send;

    Relation *city_table = Relation::instance_city();
    Relation *country_table = Relation::instance_country();

    while (true) {
        //Clear strings
        cmd.clear();
        percentage.clear();
        to_send.clear();
        
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

        //Get command
        stringstream strstr(buffer);
        if (!(strstr >> cmd)) {
            continue;
        }
        /* query command from client */
        if (cmd == "query") {
            strstr >> percentage;
            printv("Getting: %s\n", percentage.c_str());
            request_id = city_table->query(atoi(percentage.c_str()));
            to_send.clear();
            // Wait for service
            to_send = city_table->wait_for_service(request_id);
        }
        else if (cmd == "update-population") {
            strstr >> percentage;
            printv("Updating with percentage: %s\n", percentage.c_str());
            // Update city table
            request_id = city_table->update(atoi(percentage.c_str()));
            // Call updater
            to_send.clear();
            // Wait for response
            to_send = city_table->wait_for_service(request_id);
        }
        else if (cmd == "restore-log") {
            fprintf(stderr, "Restoring log files\n");
            request_id = city_table->import();
            // Call logger
            to_send.clear();
            // Wait for response
            to_send = city_table->wait_for_service(request_id);
        }
        /* Smoothly close socket, will slam all clients */
        else if (cmd == "close") {
            // Yes, lets exit.
            cout << "Received force close, EXIT" << endl;
            close(socket);
            exit(0);
        }
        /* Print queue - verbose only */
        else if (cmd == "print" && verbose) {
            //table->print_queues();
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

    int sock = *((int *)(newsock));

    handleclient(sock);

    close(sock);
}


//Main function!
int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    pthread_t thread;

    Relation *city_table = Relation::instance_city();
    Relation *country_table = Relation::instance_country();

    //do database loading here, open db, load, close db
    cout << "***************************************************" << endl;
    cout << "   WARNING: First database init could be lengthy.  " << endl;
    cout << "               Please be patient.                  " << endl;
    cout << "***************************************************" << endl;
    city_table->init_db();
    country_table->init_db();

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

    return 0;
}
