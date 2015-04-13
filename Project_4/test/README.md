# Testing

There are four test cases associated with this project. The team assumes that the query command was already proven to work, however
some manual regression testing was done to make sure the results are as expected because the query command is used to verify whether
the other commands work. The first three test cases are automated and can be found in the test directory, and the fourth test case is
a manual test case.

The first test case tests the update-population command with a two percent population increase. It assumes that no modification has
been made to the dataset. It is an automated test that updates the population and makes sure the query at 40% displays the population
values at 1.02 times their initial values.

The second test case tests the restore-log command. It is intended to be run after the first test case. Before the test case is run,
it assumes that the server data gets deleted and needs to be reinitialized. It is important that the log files are not deleted at
this time. After that, restart the server and run the test script. It expects the same result as the first test case, but it
achieves that result by using the restore-log command instead of the update-population command.

The third test case tests the restore-log command when logs do not exist. It assumes that log files either have never been created
or have been deleted. It is also an automated test. It expects that the server will notify the client that log files could not be
found and then allows the user to continue operating the client.

The fourth test case is a manual test. The test steps are listed below.
1)	First, from the project 4 directory do a “make clean” followed by a “make”.
2)	After that, start a server instance from that directory with the command “./build/server”.
3)	Allow the server to initialize its data, and then after the server is listening for connections, connect a client.
4)	Use a “query” command at 40% and write down the 19 population values that result from the query.
5)	Send an “update-population” command to increase the population by 10%.
6)	Before this operation completes, select the server instance of putty and press Ctrl+C to kill the process.
7)	Restart the server and connect the client to it.
8)	Use the “restore-log” command.
9)	Use another “query” command at 40% and verify that the population values are the same as the last query.

This test suite verifies that update-population works as expected and that restore-log will behave properly whether the log needs
to undo an operation, redo an operation, or does not exist.
