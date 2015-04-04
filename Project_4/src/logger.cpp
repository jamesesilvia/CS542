/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS542 Project 4
 *
 * logger.cpp
 *
 **********************************************************************/

#include "logger.hpp"

using namespace std;

Logger::Logger(string _tablename) {
    tablename = _tablename;
    // transaction count
    transaction = 0;
    // Log file for transactions
    string fileName = _tablename + ".log";
    log_file.open(fileName.c_str(), ios_base::app);
}


void Logger::start_commit() {
    stringstream out;
    out << "START " << transaction << endl;
    log_file << out.str();
    log_file.flush();
}


void Logger::update_entry(int ID, string old_val, string new_val) {
    stringstream out;
    out << transaction << " " << ID << " " << 
            " " << old_val << " " << new_val << endl;
    log_file << out.str();
    log_file.flush();
}


void Logger::end_commit() {
    stringstream out;
    out << "END " << transaction << endl;
    log_file << out.str();
    log_file.flush();
    // Increment transaction count
    transaction++;
}
