# Project 1
A Value Store

Project 1 uses a client/server model to represent users interacting with a database.

The server is multi-threaded and uses a serial access isolation manager to maintain
the integrity of user requests.

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
While in Project_1 directory:

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
	
put

- stores key and corresponding data in the database
- data can be input through command line, or through a file
- put will fail if the key already exists
- to update a key value, remove then re-add
- NOTE: Use a file to send data greater than 1KB.

get

- retreives data from database based on key
- NOTE: we recommend storing received data in a file if LARGE
- otherwise it will be printed directly to terminal

remove

- remove key and data from the database

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


