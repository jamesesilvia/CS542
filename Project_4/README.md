# Project 4
A Database with Indexing, Query Execution and Undo/Redo Logging

Project 4 uses a client/server model to represent users interacting with a database.

The server is multi-threaded and uses a serial access isolation manager to maintain
the integrity of user requests.

B+ Trees have been implemented to index the stored data for efficient lookup times.

The application offers three commands. The first command 'query' accepts a percentage. The query
will return a list of cities and corresponding country where the cities population
makes up the given percentage of the country. The second command 'update-population' accepts
a percentage and will update all populations in the database according to that percentage.
The third command 'restore-log' will take a change log from another instance of the database
and apply it to the current database.

The DB is loaded with data on first initialization, and can be lengthy. Once loaded,
the data will remain persistent on shutdown.

The schema used for this project is the world database from MySQL.

To see the full design document, please review the folder "docs".

## Breakdown

include/

- header files

src

- source files

test/

- expect scripts and usage README

docs/

- design document

## Usage
While in Project_4 directory:

make clean

- removes build/ directory
- removes database files (all dat, txt and log files)

make restore

- removes database files except for logs (all dat and txt files)
- leaves build/ directory and log files so log can be applied to new instance of database

make

- builds client and server in build/

make client

make server

- builds each application individually

./build/server

- launches the server

./build/client <server ip>

- launches the client

## Client
The client provides interfaces to the database through the commands:
	
query

- accepts a percentage as input
- return list of cities, countries that match the query...
- if the city size makes up the given percentage of country

update-population

- accepts a percentage as input
- increases or decreases all populations in the database by the given percentage

restore-log

- applies all commits of a change log from past instance of database to current database

## Server
The server is currently configured in verbose mode. It will print actions
that it receives from the client, but will not display any data.

The database is persistent if the server is ever closed. 

Do not alter any database assocated files, otherwise bad things can happen.

make clean WILL remove the database.

make restore WILL remove the database but leave the change log so it can be applied to a future instance.

## Tests
The test/ directory contains the output from our client application as well as
the results for the SQL Query with the world database. These two were compared 
for validation.

## Notes
- This code has been developed and test on the WPI CCC machines.
- It is fully functional, and we do not guarantee functionality
	besides on the machines above.

- Enjoy!!

Tyler, James, and Tommy.


