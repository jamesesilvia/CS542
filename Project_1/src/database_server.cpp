/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 1
 *
 * database_server.cpp
 *
 **********************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

#include "helpers.hpp"

//The backlog for listening
#define BACKLOG 5

//The buffer length for receiving commands
#define BUFFER_LEN 1024

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
    int len;
    string cmd, index, to_send;

    while (true) {
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
            break;
        }

        printv("Received: %s\n", buffer);
        //Stringstream of received data
        stringstream strstr(buffer);
        if (!(strstr >> cmd))
            continue;

        /* Put request received */
        if (cmd == "put") {
            to_send = "We got your PUT request brah";
        } 
        /* Get request received */
        else if (cmd == "get") {
            //Get the entry
            to_send = "We got your GET request brah";
        } 
        /* Remove request received */
        else if (cmd == "remove") {
           //Remove the entry
           to_send = "We got your REMOVE request brah";
        } 
        /* Junk catch all */
        else {
            continue;
        }

        //Send the data
        len = write(socket, to_send.c_str(), to_send.length());
        if (len != to_send.length())
            error("Unable to send the data!");
    }

    return true;
}


//functino to handle new client as a new thread
void *handleclient_thread(void *newsock ){

    int *sock = (int*)(newsock);
    
    handleclient(*sock);

    close(*sock);
}


//Main function!
int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    pthread_t thread;

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
    return 0;
}
