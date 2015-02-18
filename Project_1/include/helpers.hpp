/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 1
 *
 * helpers.hpp
 *
 **********************************************************************/

#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

#define PORT    1433
#define MAX_KEY 1000000

using namespace std;

void error(const char * str);
void error2(const char * str);
int hostname_to_ip(const char * hostname , char* ip);
void printv(char *format, ...);

#endif
