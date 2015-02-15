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

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
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

using namespace std;

#include "relation.hpp"
#include "isolation_manager.hpp"

Relation::Relation(string table_name) {
    name = table_name;
}

int Relation::queue_command(string command) {
    
    //initialize variables
    int key;
    string data;
    int client;
    int action = 3;	//set action to an invalid number assuming that it will become valid later
    bool dataExists;

    stringstream ss(command);
    string buffer;

    //pull first substring off of command, this one is the action
    ss >> buffer;
	
    //put case
    if (buffer == "put")
    {
	action = 0;
	dataExists = true;
    }

    //get case
    if (buffer == "get")
    {
	action = 1;
	dataExists = false;
    }

    //remove case
    if (buffer == "remove")
    {
	action = 2;
	dataExists = false;
    }

    //pull next substring off of command, this one is the key
    ss >> buffer;
	
    //convert to int
    key = atoi(buffer.c_str());

    //third substring only exists in put case
    if (dataExists)
    {
	ss >> buffer;
	    
	data = buffer;
    }

    //TODO: Where does the int for client come from?

    if (action != 3)
    {
	cout << "Adding item to queue!" << endl;
	put_request_for_service(key, data, client, action);
	return 0;
    }

    else
    {
	cout << "Unable to add item to queue!" << endl;
	return 1;
    }

}
