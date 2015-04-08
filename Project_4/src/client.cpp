/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 4
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

//Global for client
ifstream infile;
ofstream outfile;
//bool print_to_file;
string output_file;

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
        bool got_size = false;
        int len_to_read = BUFFER_LEN;
        string cmd, key, data_len, data;

        do {
            memset(buffer, 0, sizeof(buffer));
            
            //Shall we get some data?
            len = read(sock, buffer, BUFFER_LEN);

            //Some sort of error
            if (len < 0)
                error("Unable to read from socket.");

            //Disconnected
            if (len == 0)
                error("Server disconnected!");
            
            if (!got_size) {
                //Stringstream of received data
                stringstream strstr(buffer);
                //Get control data from buffer
                if (!(strstr >> cmd))
                    continue;
                if (!(strstr >> data_len))
                    continue;
                len_to_read = atoi(data_len.c_str());
                int pos = strstr.tellg();
                data = strstr.str().substr(pos+1,len_to_read);
                
                got_size = true;
            }
            else {
                data = data + string(buffer);
                len_to_read = len_to_read - BUFFER_LEN;
            }
        } while(len_to_read > BUFFER_LEN);

        // Response
        stringstream response;
        response << cmd << " with response..." << endl;

        //if (print_to_file){
        //    outfile.open(output_file.c_str());
        //    outfile << data;
        //    outfile.close();
        //}
        //else {
        cout << response.str() << data << endl;
        //}
        cout << "Done." << endl;
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
    bool done, fail;
    int i, index;

    while (true) {

        //Clear Everything
        str.clear();
        cmd.clear();
        to_send.clear();
        output_file.clear();
        //print_to_file = false;
        
        //Print out a command line
        cout << "-> "; 
        cout.flush();
        getline(cin, str); //Get a full line of text

        //Grab the command portion
        stringstream strstr(str);
        if (!(strstr >> cmd))
            continue;
        
        /* Query command */
        if (cmd == "query") {
            // Get percentage of country population to use in query
            done = false;
            while (!done) {
                str.clear();
                cout << "Please enter percentage of country population to use in query: ";
                cout.flush();
                getline(cin, str);
                if (str.empty()) {
                    continue;
                }
                fail = false;
                for (i = 0; i < str.length(); i++) {
                    if (!isdigit(str[i])) {
                        cout << "Please enter positive integers from 0 to 100 only!" << endl;
                        fail = true;
                        break;
                    }
                }
                if (fail) {
                    continue;
                }
                int percentage = atoi(str.c_str());
                if (percentage < 0) {
                    cout << "Please enter positive integers from 0 to 100 only!" << endl;
                    continue;
                }
                if (percentage > 100) {
                    cout << "Please enter an integer less than 100!" << endl;
                    continue;
                }
                to_send = cmd + " " + str;
                done = true;
            }            
            // All done
            break;
        }
        /* Update population command */
        if (cmd == "update-population") {
            // Get percentage of population to change all database entries by
            done = false;
            while (!done) {
                str.clear();
                cout << "Please enter percentage of population change: ";
                cout.flush();
                getline(cin, str);
                if (str.empty()) {
                    continue;
                }
                fail = false;
                for (i = 0; i < str.length(); i++) {
                    if (!isdigit(str[i])) {
                        cout << "Please enter integers from -10 to -1 or 1 to 10 only!" << endl;
                        fail = true;
                        break;
                    }
                }
                if (fail) {
                    continue;
                }
                int percentage = atoi(str.c_str());
                if (percentage < -10) {
                    cout << "Please enter integers from -10 to -1 or 1 to 10 only!" << endl;
                    continue;
                }
                if (percentage > 10) {
                    cout << "Please enter integers from -10 to -1 or 1 to 10 only!" << endl;
                    continue;
                }
                if (percentage == 0) {
                    cout << "Please enter integers from -10 to -1 or 1 to 10 only!" << endl;
                    continue;
                }
                to_send = cmd + " " + str;
                done = true;
            }            
            // All done
            break;
        }
        /* Restore log command */
        else if (cmd == "restore-log") {
            to_send = cmd;
            break;
        }
        /* Remove command, debug only */
        else if (cmd == "print") {
            to_send = cmd;
            break;
        }
        /* Force close server, slams clients */
        else if (cmd == "close_hidden_server") {
            to_send = "close";
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
            cout << "   query             - query database for a city whose " << endl;
            cout << "                       population is a specified percentage " << endl;
            cout << "                       of the country population" << endl;
            cout << "   update-population - change population of all data " << endl;
            cout << "                       by a specified percentage " << endl;
            cout << "   restore-log       - restore data from all logfiles " << endl;
            cout << "   quit              - exit the application" << endl;
            continue;
        }

    }
    return to_send;
}
