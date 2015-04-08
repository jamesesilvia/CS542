/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 4
 *
 * logger.hpp
 *
 **********************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

using namespace std;

class Logger {
public:
    Logger(string _tablename);
    void start_commit();
    void update_entry(int ID, int old_val, int new_val);
    void end_commit();

private:
    string tablename;
    int transaction;
    ofstream log_file;
};

#endif
