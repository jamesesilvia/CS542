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
#include <fstream>

#define PORT 1433

using namespace std;

void error(const char * str);
int hostname_to_ip(const char * hostname , char* ip);
void printv(char *format, ...);
bool check_for_file(const char *fileName);

#endif
