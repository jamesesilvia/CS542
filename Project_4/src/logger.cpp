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
}


void Logger::start_commit() {
    stringstream out;
    string fileName = tablename + ".log";
    
    // open log file with append
    log_file.open(fileName.c_str(), ios_base::app);
    
    // write to log file, starting transaction
    out << "START " << transaction << endl;
    log_file << out.str();

    // cleanup
    log_file.flush();
    log_file.close();
}


void Logger::update_entry(int ID, string old_val, string new_val) {
    stringstream out;
    string fileName = tablename + ".log";
    
    // open log file with append
    log_file.open(fileName.c_str(), ios_base::app);
    
    // write to log file, updating entry
    out << transaction << " " << ID << " " << 
            " " << old_val << " " << new_val << endl;
    log_file << out.str();
    
    // cleanup
    log_file.flush();
    log_file.close();
}


void Logger::end_commit() {
    stringstream out;
    string fileName = tablename + ".log";
    
    // open up log file with append
    log_file.open(fileName.c_str(), ios_base::app);
    
    // write to log file, ending transaction
    out << "END " << transaction << endl;
    log_file << out.str();

    // cleanup
    log_file.flush();
    log_file.close();
    // Increment transaction count
    transaction++;
}
