/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
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

string handle_user_interaction(int sock);

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

    while(true) {
        //Handle user interaction
        string to_send = handle_user_interaction(sock);

        //Send the data
        int len = write(sock, to_send.c_str(), to_send.length());
        if (len != to_send.length())
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

/*
 * hander_user_interaction()
 * 
 * Gather user request to send to server
 *
 * Return string to send
 * or exit the connection and application
 */
string handle_user_interaction(int sock) {
    string str, cmd, to_send;
    bool done;

    while (true) {

        //Clear Everything
        str.clear();
        cmd.clear();
        to_send.clear();
        
        //Print out a command line
        cout << "-> "; 
        cout.flush();
        getline(cin, str); //Get a full line of text

        //Grab the command portion
        stringstream strstr(str);
        if (!(strstr >> cmd))
            continue;
        
        /* Put command */
        if (cmd == "put") {
            // Get key to store data
            done = false;
            while (!done) {
                str.clear();
                cout << "Please enter key to store data: ";
                cout.flush();
                getline(cin, str);
                if (str.empty()) {
                    continue;
                }
                to_send = cmd + " " + str;
                done = true;
            }
            // Get data to store
            done = false;
            while (!done) {
                str.clear();
                cout << "Do you want to send a file [y/n] ? ";
                cout.flush();
                getline(cin, str);
                // Get data from file
                if (str == "y") {
                    string filename;
                    while (filename.empty()) {
                        cout << "Please enter the file name: ";
                        cout.flush();
                        getline(cin, filename);
                    }
                    // Do crazy shit with a file that idk yet
                    to_send = to_send + " " + filename;
                    done = true;
                }
                // Get data from command line
                else if (str == "n") {
                    string data;
                    while (data.empty()) {
                        cout << "Please enter the data to store: ";
                        cout.flush();
                        getline(cin, data);
                    }
                    // Build string to send
                    to_send = to_send + " " + data;
                    done = true;
                }
                // Do it again...
                else {
                    continue;
                }
            }
            // All done
            break;
        } 
        /* Get command */
        else if (cmd == "get") {
            // Get key to retrieve data
            done = false;
            while (!done) {
                str.clear();
                cout << "Please enter key to retrieve data: ";
                cout.flush();
                getline(cin, str);
                if (str.empty()) {
                    continue;
                }
                to_send = cmd + " " + str;
                done = true;
            }
            // All done
            break;
        }
        /* Remove command */
        else if (cmd == "remove") {
            // Get key to retrieve data
            done = false;
            while (!done) {
                str.clear();
                cout << "Please enter key to remove data: ";
                cout.flush();
                getline(cin, str);
                if (str.empty()) {
                    continue;
                }
                to_send = cmd + " " + str;
                done = true;
            }
            // All done
            break;
        }
        /* Remove command, debug only */
        else if (cmd == "print") {
            to_send = cmd;
            break;
        }
        else if (cmd == "close_server") {
            to_send = "close server";
            break;
        }
        /* User wants to close the applcation */
        else if (cmd == "quit") {
            close(sock);
            exit(0);
        } 
        /* Print Help */
        else {
            cout << "Acceptable commands are:" << endl;
            cout << "   put     - store data" << endl;
            cout << "   get     - retrieve data" << endl;
            cout << "   remove  - delete data" << endl;
            cout << "   quit    - exit the application" << endl;
            continue;
        }

    }
    return to_send;
}
