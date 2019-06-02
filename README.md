# PROJECT 2
Name: Chelsea Egan

Class: CS 372-400

Last Modified: June 2, 2019

This is a file transfer system created for project 2 of the Intro to Networks class at Oregon State University. A server is started via a terminal and opens a socket for "clients" to connect to. The client is also started via a terminal (can be on the same or different host as the server) and connects to the server by providing its hostname, port number, a command, and a port number for the data connection. This creates the control connection for the commands to be handled. The server then connects to the client's data connection to transfer the requested data (either it's directory list or a file). Once the command is completely processed, the data and control connections are terminated and the client's program ends. The server will remain open for future client connections.


## Installation
For ftclient.py use the makefile to turn it into an executable file
```
make ftclient
```
For ftserver.c use the makefile to compile
```
make ftserver
```
\* NOTE: the files must be in the same directories as follows:
```
ftclient.py
makefile
```
```
ftserver.c
ftutilities.c
ftutilities.h
makefile
```

## Usage
After running the makefile commands, start the server by providing a port number. For example:
```
./ftserver.exe 30200
```
Second, start the client by providing a hostname, port number, command, and data port number. For example:
```
./ftclient.py flip2.engr.oregonstate.edu 30200 -l 20000
// OR
./ftclient.py flip2.engr.oregonstate.edu 30200 -g "alice.txt" 20000
```


## Ending
If you wish to end the program before the command is fully processed, entering Ctrl+C will terminate either the server or client program.


## Notes
- I have tested using flip1 and flip2, alternating for both between server and client. It shouldn't matter which you use.
- I have been using port 30200 and 20000 with success, but my program just asks for one between 1024 and 65535.
- I have tested with files up to 10Mb, theoretically should work with most sizes.


## Sources
PYTHON
- General socket documentation used for ftclient
	- https://docs.python.org/3/library/socket.html
	- Computer Networking by Kurose & Ross, section 2.7.2
- Input validation and error handling
	- https://www.101computing.net/number-only/
	- https://stackoverflow.com/a/16745561
	- https://stackoverflow.com/a/9015233
- File handling
	- https://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists-without-exceptions
	- https://docs.python.org/3/tutorial/inputoutput.html
- Setting up the main method
	- https://www.guru99.com/learn-python-main-function-with-examples-understand-main.html
- Catching Ctrl+C
	- https://stackoverflow.com/questions/1187970/how-to-exit-from-python-without-traceback
- My code from Project 1
	- https://github.com/level5esper/ChatClient/blob/master/chatserve.py

C
- File handling
	- https://stackoverflow.com/a/22623744
	- https://stackoverflow.com/questions/30440188/sending-files-from-client-to-server-using-sockets-in-c
	- https://stackoverflow.com/questions/11952898/c-send-and-receive-file
	- https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
	- https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
	- http://pubs.opengroup.org/onlinepubs/009695399/functions/opendir.html
	- http://pubs.opengroup.org/onlinepubs/009695399/functions/readdir.html
	- https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtgtc.htm
- TCP connections
	- https://beej.us/guide/bgnet/html/multi/clientserver.html#simpleserver	
	- https://beej.us/guide/bgnet/html/multi/clientserver.html#simpleclient
	- https://stackoverflow.com/a/18437957
- My code from Project 1
	- https://github.com/level5esper/ChatClient/blob/master/chatUtilities.c
