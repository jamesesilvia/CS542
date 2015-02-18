/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 1
 *
 * helpers.cpp
 *
 **********************************************************************/

#include "helpers.hpp"
#include "dirent.h"
#include <arpa/inet.h>
#include <netdb.h>

/* Error without errno set */
void error(const char * str)
{
    cerr << str << endl;
}

//Converting the hostname to an IP address
int hostname_to_ip(const char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL)
        error("Unable to get the hostname!");

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}

//Check if file exists
bool check_for_file(const char *fileName) {
    ifstream infile(fileName);
    return infile.good();
}
