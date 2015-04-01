# Project 3
A Value Store with Indexing and Query Execution

Project 3 uses a client/server model to represent users interacting with a database.

The server is multi-threaded and uses a serial access isolation manager to maintain
the integrity of user requests.

B+ Trees have been implemented to index the stored data for efficient lookup times.

The application offers a single command 'query' that accepts a percentage. The query
will return a list of cities and corresponding country where the cities population
makes up the given percentage of the country.

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
While in Project_3 directory:

make clean

- removes build/ directory
- removes database files (all dat and txt files)

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

## Server
The server is currently configured in verbose mode. It will print actions
that it receives from the client, but will not display any data.

The database is persistent if the server is ever closed. 

Do not alter any database assocated files, otherwise bad things can happen.

make clean WILL remove the database.

## Tests
The test/ directory contains several Expect scripts that were used to test
concurrent accesses to our database. I redirect you to their README for any
additional information.

## Notes
- This code has been developed and test on the WPI CCC machines.
- It is fully functional, and we do not guarantee functionality
	besides on the machines above.

- Enjoy!!

Tyler, James, and Tommy.


