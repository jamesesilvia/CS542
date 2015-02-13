#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

#define PORT 1433

using namespace std;

void error(const char * str);
int hostname_to_ip(const char * hostname , char* ip);
void printv(char *format, ...);
