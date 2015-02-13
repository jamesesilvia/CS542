/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 1
 *
 * client.cpp
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

#include "helpers.hpp"

//The buffer length for receiving commands
#define BUFFER_LEN 1024

//Main function
int main(int argc, char *argv[]) {

    int sock;
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    int readlen;
    char strip[64] = {0};

    //Make sure the arguments match
    if (argc != 2) {
        fprintf(stderr, "Invalid usage: %s <target>\n", argv[0]);
        return -1;
    }

    //Clear the structures just incase
    memset(&cli_addr,  0, sizeof(cli_addr));

    //Fill in the server structure
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(PORT);
    
    if (hostname_to_ip(argv[1], strip) != 0)
        error("Unable to determine IP from hostname!");

    //Create the sin_addr from the argument
    if (inet_pton(AF_INET, strip, &cli_addr.sin_addr) != 1)
        error("Invalid IP address!");
    
    //Create the sock handle (TCP Connection)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        error("Unable to create sock!");

    //Connect to the target
    if (connect(sock, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
        error("Unable to connect to target address!");
    
    while (true) {
        string str, cmd;
        
        //Print out a command line
        cout << "-> "; 
        cout.flush();
        getline(cin, str); //Get a full line of text

        //Grab the command portion
        stringstream strstr(str);
        if (!(strstr >> cmd))
            continue;

        if (cmd == "put") {
            cout << "put bitch" << endl;
            //Get info to send data
            str.clear();
            str = cmd;
        } else if (cmd == "get") {
            cout << "get bitch" << endl;
            //Get info to send data
            str.clear();
            str = cmd;
        } else if (cmd == "remove") {
            cout << "remove bitch" << endl;
            //Get info to send data
            str.clear();
            str = cmd;
        } else if (cmd == "quit") {
            close(sock);
            return 0;
        } else {
            cout << "Acceptable commands are:" << endl;
            cout << "   put" << endl;
            cout << "   get" << endl;
            cout << "   remove" << endl;
            cout << "   quit" << endl;
            continue;
        }

        //Send the data
        int len = write(sock, str.c_str(), str.length());
        if (len != str.length())
            error("Unable to send the data!");

        //Get the response
        char buffer[BUFFER_LEN + 1] = {0};
        len = read(sock, buffer, BUFFER_LEN);

        //Some sort of error
        if (len < 0)
            error("Unable to read from socket.");

        //Disconnected
        if (len == 0)
            error("Server disconnected!");

        //Print the response
        cout << buffer << endl;
    }

    return 0;
}
