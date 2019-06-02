/*****************************************************************
 * Name: Chelsea Egan
 * Course: CS 372-400
 * Program: ftutilities.c
 * Description: This file provides the initialization of functions
 * used by ftserver.c
 * Last Modified: June 2, 2019
*****************************************************************/

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ftutilities.h"

/*****************************************************************
 * name: startUp
 * Preconditions:
 * @param argNum - integer indicating number of command line args
 * @param port - port number provided on command line
 * @param sockets - array of sockets used by program
 * Postconditions: Validates number of args provided and the port
 * number then creates a welcoming socket.
 *****************************************************************/
void startUp(int argNum, char *port, int sockets[]) {
    // Set the port of the server to the one selected if valid
    validateCommandLineArguments(argNum, sockets);
    char* controlPort = validatePortNumber(port, sockets);

    // Create a socket to listen for connection requests
    createSocket(controlPort, sockets);
}

/*****************************************************************
 * name: validateCommandLineArguments
 * Preconditions:
 * @param numArgs - integer indicating number of command line args
 * @param sockets - array of sockets used by program
 * Postconditions: If the numArgs is not 2, terminates the program
 * as it was started incorrectly.
 *****************************************************************/
void validateCommandLineArguments(int numArgs, int sockets[]) {
    // User should have entered two arguments on the command line:
    // file name and port number
    if (numArgs != 2) {
        terminateProgram("TRY AGAIN WITH ./ftserver.exe <port>", sockets);
    }
}

/*****************************************************************
 * name: validatePortNumber
 * Preconditions:
 * @param port - char pointer to the command line argument
 * @param sockets - array of sockets used by program
 * Postconditions: Returns the port number if it is valid and
 * within the range of 1024-65535. Otherwise, it terminates the
 * program.
 *****************************************************************/
char* validatePortNumber(char *port, int sockets[]) {
    if(atoi(port) < 1024 || atoi(port) > 65535) {
        terminateProgram("OUT OF RANGE! USE AN INT BETWEEN 1024-65535.", sockets);
    } else {
        return port;
    }
}

/*****************************************************************
 * name: createSocket
 * Preconditions:
 * @param port - char pointer to the selected port number
 * @param sockets - array of sockets used by program
 * Postconditions: Creates a "welcoming socket" to listen for
 * incoming connection requests. If the setup fails, the program
 * terminates.
 * Source: https://beej.us/guide/bgnet/html/multi/clientserver.html#simpleserver
 * Source for handling zombie children: https://stackoverflow.com/a/18437957
 *****************************************************************/
void createSocket(char *port, int sockets[]) {
    struct addrinfo hints, *servInfo, *p;
    int addressInfo;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // Autofills IP address

    // Get the address information for the host
    if((addressInfo = getaddrinfo(NULL, port, &hints, &servInfo)) != 0) {
        terminateProgram("FAILED TO GET ADDRESS INFO", sockets);
    }

    for(p = servInfo; p != NULL; p = p->ai_next) {
        // Creates the welcoming socket
        if((sockets[WELCOME_SOCKET] = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // Sets options for the socket
        if(setsockopt(sockets[WELCOME_SOCKET], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            terminateProgram("FAILED TO SET SOCKET OPTIONS", sockets);
        }

        // Binds to created welcoming socket
        if(bind(sockets[WELCOME_SOCKET], p->ai_addr, p->ai_addrlen) == -1) {
            close(sockets[WELCOME_SOCKET]);
            continue;
        }

        break;
    }

    freeaddrinfo(servInfo);

    // Socket creation was unsuccessful - terminates the whole program
    if (p == NULL) {
        terminateProgram("SOCKET CREATION FAILED", sockets);
    }

    // Listening was unsuccessful - terminates the whole program
    if (listen(sockets[WELCOME_SOCKET], BACKLOG) == -1) {
        terminateProgram("FAILED TO LISTEN TO SOCKET", sockets);
    }

    // Silently reap forked children instead of turning it into a zombie
    signal(SIGCHLD, SIG_IGN);

    // Display the connection details
    char hostname[256] = {0};
    gethostname(hostname, sizeof(hostname));

    fflush(stdout);
    printf("\nListening on...\n");
    printf("Hostname: %s\n", hostname);
    printf("Port: %s\n\n", port);
}

/*****************************************************************
 * name: initDataConnection
 * Preconditions:
 * @param hostname - char pointer to hostname/IP address of client
 * @param port - char pointer to the client's port number
 * @param sockets - array of sockets used by the program
 * Postconditions:
 * If a connection cannot be made, then the program will return to
 * listening for connection requests.
 * If connection to client is made, newSocket is created and can
 * be used to send data to the client.
 * Source: https://beej.us/guide/bgnet/html/multi/clientserver.html#simpleclient
 * Source: my code from Project 1
 *****************************************************************/
int initDataConnection(char *hostname, char *port, int sockets[]) {
    int status;
    struct addrinfo hints, *servInfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          //IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP

    // Get the address information for the client, which can be used to connect
    // Failure: closes all connection with client and returns to listening
    if ((status = getaddrinfo(hostname, port, &hints, &servInfo)) != 0) {
        closeConnection("Failed to get address info for data socket", sockets);
        return -1;
    }

    // Loops through addresses until one that can be connected to is found
    for (p = servInfo; p != NULL; p = p->ai_next) {
        // Tries to create a data socket using the info from getaddrinfo
        if ((sockets[DATA_SOCKET] = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        // If data socket was created, tries to use it to connect to the client
        if (connect(sockets[DATA_SOCKET], p->ai_addr, p->ai_addrlen) == -1) {
            close(sockets[DATA_SOCKET]);
            continue;
        }
        // Socket was successfully created and connection was made
        // Stop trying
        break;
    }

    // No connection was made - return to listening for connection requests
    if (p == NULL) {
        closeConnection("Failed to create a data socket", sockets);
        return -1;
    }

    freeaddrinfo(servInfo);
    return 0;
}

/*****************************************************************
 * Name: acceptConnections
 * Preconditions:
 * @param sockets - array of sockets used by program
 * @param clientHostName - hostname of client
 * Postconditions: Creates a control socket once a request is
 * received from the client. Terminates the program if fails.
 * Source: https://beej.us/guide/bgnet/html/multi/clientserver.html#simpleserver
 *****************************************************************/
void acceptConnections(int sockets[], char *clientHostName) {
    socklen_t sinSize;
    struct sockaddr_storage theirAddressInfo;
    char theirAddress[INET6_ADDRSTRLEN];

    fflush(stdout);

    // Accept incoming connection request and create control socket
    sinSize = sizeof(theirAddressInfo);
    sockets[CONTROL_SOCKET] = accept(sockets[WELCOME_SOCKET], (struct sockaddr *)&theirAddressInfo, &sinSize);
    if(sockets[CONTROL_SOCKET] == -1) {
        terminateProgram("FAILED TO ACCEPT CONNECTIONS", sockets);
    }

    // Gets the address information about the connection
    inet_ntop(theirAddressInfo.ss_family,
              &(((struct sockaddr_in*)((struct sockaddr *)&theirAddressInfo))->sin_addr),
              theirAddress, sizeof(theirAddress));

    printf("\nConnected to: %s\n", theirAddress);

    // Store the client hostname
    strncpy(clientHostName, theirAddress, sizeof(theirAddress));
}

/*****************************************************************
 * Name: handleRequests
 * Preconditions:
 * @param sockets - array of sockets used by program
 * @param clientHostName - hostname of client
 * Postconditions: Gets necessary information from the client and,
 * if valid, calls the required functions to either send its
 * directory or send a file.
 *****************************************************************/
void handleRequests(int sockets[], char *clientHostName) {
    int command;
    char dataPort[MAXBUFFERSIZE + 1];
    char fileName[MAXBUFFERSIZE + 1];

    // Get the command from the client from the control socket
    // failure: go to beginning of while loop
    command = receiveCommand(&sockets[CONTROL_SOCKET]);
    if(command == -1) {
        closeConnection("Received invalid command", sockets);
        return;
    }

    // Get the data port for data connection from the control section
    // failure: go to beginning of while loop
    if(receiveDataPort(&sockets[CONTROL_SOCKET], dataPort) == -1) {
        closeConnection("Received invalid data port", sockets);
        return;
    }

    // Client requested to receive a file
    if (command == FILE_COMMAND) {
        // Get the requested file name
        receiveFileName(&sockets[CONTROL_SOCKET], fileName);
        fflush(stdout);
        printf("File '%s' requested on port %s\n", fileName, dataPort);

        // Wait until an ACK is received from the client
        // This indicates they are ready for a data connection
        char confirmation[MAXBUFFERSIZE] = {0};
        while(strcmp(confirmation, CONFIRMATION) != 0) {
            receiveMessage(&sockets[CONTROL_SOCKET], confirmation);
        }

        // Connect to client's data socket
        if(initDataConnection(clientHostName, dataPort, sockets) == -1) {
            return;
        }

        // Send requested file
        if(sendFile(sockets, fileName) == -1) {
            return;
        }
    } else {
        // Client requested the directory list
        fflush(stdout);
        printf("List directory requested on port %s\n", dataPort);

        // Wait until an ACK is received from the client
        // This indicates they are ready for a data connection
        char confirmation[MAXBUFFERSIZE] = {0};
        while(strcmp(confirmation, CONFIRMATION) != 0) {
            receiveMessage(&sockets[CONTROL_SOCKET], confirmation);
        }

        // Connect to client's data socket
        if(initDataConnection(clientHostName, dataPort, sockets) == -1) {
            return;
        }

        // Send the list of files in the directory
        sendDirectory(sockets);
    }

    // Close control and data sockets
    close(sockets[CONTROL_SOCKET]);
    close(sockets[DATA_SOCKET]);
}

/*****************************************************************
 * Name: receiveCommand
 * Preconditions:
 * @param socket - pointer to the socket that is connected
 * Postconditions: Reads in command from client and checks if a
 * valid command (-l or -g). Response indicates which command.
 *****************************************************************/
int receiveCommand(int *socket) {
    char command[MAXBUFFERSIZE + 1];

    // Get command from buffer
    if(receiveMessage(socket, command) == -1) {
        return -1;
    }

    // If command doesn't start with '-' it must be invalid
    if(command[0] != '-') {
        send(*socket, REJECTION, strlen(REJECTION), 0);
        return -1;
    }

    // Check if a recognized command and send confirmation/rejection to client
    switch(command[1]) {
        case 'l':
            if(send(*socket, CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
                return -1;
            }
            return LIST_COMMAND;
        case 'g':
            if(send(*socket, CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
                return -1;
            }
            return FILE_COMMAND;
        default:
            if(send(*socket, REJECTION, strlen(REJECTION), 0) == -1) {
                return -1;
            }
            return -1;
    }
}

/*****************************************************************
 * Name: receiveDataPort
 * Preconditions:
 * @param socket - pointer to the socket that is connected
 * @param port - pointer to the port provided by client
 * Postconditions: Gets provided port number from client and
 * verifies validity. Sends confirmation to client.
 *****************************************************************/
int receiveDataPort(int *socket, char *port) {
    // Get port number from buffer
    if(receiveMessage(socket, port) == -1) {
        return -1;
    }

    // Check if in a valid range of port number
    if(atoi(port) < 1024 || atoi(port) > 65535) {
        send(*socket, REJECTION, strlen(REJECTION), 0);
        return -1;
    } else {
        if(send(*socket, CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
            return -1;
        }
        return 0;
    }
}

/*****************************************************************
 * Name: receiveFileName
 * Preconditions:
 * @param socket - pointer to the socket that is connected
 * @param buffer - pointer to where the filename should be stored
 * Postconditions: Reads in requested filename from buffer and
 * stores for use by other functions.
 *****************************************************************/
int receiveFileName(int *socket, char *buffer) {
    // Get filename from buffer
    if(receiveMessage(socket, buffer) == -1) {
        return -1;
    }

    // Send confirmation to client
    if(send(*socket, CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
        return -1;
    }
    return 0;
}

/*****************************************************************
 * Name: receiveMessage
 * Preconditions:
 * @param socket - pointer to the socket that is connected
 * @param buffer - pointer to the char array that will hold the
 * incoming message
 * Postconditions: Reads from buffer
 *****************************************************************/
int receiveMessage(int *socket, char *buffer) {
    int numBytes;

    // Put bytes into buffer
    if ((numBytes = recv(*socket, buffer, MAXBUFFERSIZE, 0)) == -1) {
        return -1;
    }

    // Append with a newline
    buffer[numBytes] = '\0';
    return 0;
}

/*****************************************************************
 * Name: validateFileName
 * Preconditions:
 * @param fileName - pointer to char array holding filename
 * Postconditions: Checks if the directory has a file with the
 * requested name and returns true/false.
 * Source for looping through directory: https://stackoverflow.com/a/22623744
 *****************************************************************/
bool validateFileName(char *fileName) {
    DIR *directory;
    struct dirent *dirPtr;

    // Open local directory
    if((directory = opendir(".")) == NULL) {
        return false;
    }

    // Loop through directory and compare filenames
    while((dirPtr = readdir(directory)) != NULL) {
        if (strcmp(fileName, dirPtr->d_name) == 0) {
            return true;
        }
    }
    return false;
}

/*****************************************************************
 * Name: sendFile
 * Preconditions:
 * @param sockets - array of sockets used by program
 * @param fileName - pointer to char array holding filename
 * Postconditions: Checks if the requested filename exists in the
 * directory. If it does, sends file to client over data connection
 * References:
 * https://stackoverflow.com/questions/30440188/sending-files-from-client-to-server-using-sockets-in-c
 * https://stackoverflow.com/questions/11952898/c-send-and-receive-file
 * https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
 *****************************************************************/
int sendFile(int sockets[], char *fileName) {
    int sentBytes;
    struct stat fileStats;
    char fileSize[256];
    char sendBuffer[2048];
    FILE *sendingFile;
    char clientGoAhead[256];

    // Check if filename exists in directory
    if(validateFileName(fileName)) {
        // Send confirmation to client
        if(send(sockets[DATA_SOCKET], CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
            closeConnection("Failed to send confirmation to client", sockets);
            return -1;
        }
    } else {
        // Couldn't find file - close connection with client
        closeConnection("Invalid file name", sockets);
        return -1;
    }

    // Wait until client lets us know they're ready to receive file
    if(receiveMessage(&sockets[DATA_SOCKET], clientGoAhead) == -1) {
        closeConnection("Failed to receive go ahead from client", sockets);
        return -1;
    }
    if (strcmp(clientGoAhead, CONFIRMATION) == 0) {
        // Open the requested file
        if((sendingFile = fopen(fileName, "r")) == NULL){
            closeConnection("Failed to open file", sockets);
            return -1;
        }

        // Read the file and send over data connection
        while((sentBytes = fread(sendBuffer, 1, sizeof(sendBuffer), sendingFile)) > 0) {
            if(send(sockets[DATA_SOCKET], sendBuffer, strlen(sendBuffer), 0) == -1) {
                closeConnection("Failed to send file to client", sockets);
                return -1;
            }
            memset(sendBuffer, 0, sizeof(sendBuffer));
        }

        // Send ACK to confirm end of file
        if(send(sockets[DATA_SOCKET], CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
            closeConnection("Failed to send confirmation to client", sockets);
            return -1;
        }

        fclose(sendingFile);
        printf("File transfer complete\n");
    } else {
        // Inform client that file does not exist and close connection
        send(sockets[DATA_SOCKET], REJECTION, strlen(REJECTION), 0);
        closeConnection("Invalid filename received from client", sockets);
        return -1;
    }
}

/*****************************************************************
 * Name: sendDirectory
 * Preconditions:
 * @param sockets - array of sockets used by program
 * Postconditions: Sends a list of files stored in the current
 * directory to the client.
 * References:
 * https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/opendir.html
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/readdir.html
 * https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtgtc.htm
 *****************************************************************/
int sendDirectory(int sockets[]) {
    DIR *directory;
    struct dirent *dirPtr;
    char cwd[MAXBUFFERSIZE];
    char message[MAXBUFFERSIZE];

    // Open local directory
    if((directory = opendir(".")) == NULL) {
        closeConnection("Failed to open directory", sockets);
        return -1;
    }

    getcwd(cwd, sizeof(cwd));
    strncat(cwd, "\n", 2);

    // Send the name of the current directory to the client
    if(send(sockets[DATA_SOCKET], cwd, strlen(cwd), 0) == -1) {
        closeConnection("Failed to send directory name", sockets);
        return -1;
    }

    // Iterate over the directory and send the filename over the data connection
    while((dirPtr = readdir(directory)) != NULL) {
        strncpy(message, dirPtr->d_name, sizeof(dirPtr->d_name));
        strncat(message, "\n", 2);
        if(send(sockets[DATA_SOCKET], message, strlen(message), 0) == -1) {
            closeConnection("Failed to send directory", sockets);
            return -1;
        }
    }

    // Send 'ACK' to confirm all files have been sent
    if(send(sockets[DATA_SOCKET], CONFIRMATION, strlen(CONFIRMATION), 0) == -1) {
        closeConnection("Failed to send confirmation to client", sockets);
        return -1;
    }
}

/*****************************************************************
 * Name: terminateProgram
 * Preconditions:
 * @param errorMessage - string holding message that should be
 * printed describing the error
 * @param sockets - array of sockets used by program
 * Postconditions: The error message will be printed and then
 * all sockets closed.
 *****************************************************************/
void terminateProgram(char *errorMessage, int sockets[]) {

    fflush(stdout);
    printf("\n\n***** %s *****\n", errorMessage);
    printf("***** CLOSING CONNECTION *****\n");

    closeSockets(sockets, 3);

    exit(1);
}

/*****************************************************************
 * Name: closeConnection
 * Preconditions:
 * @param errorMessage - string holding message that should be
 * printed describing the error
 * @param sockets - array of sockets used by program
 * Postconditions: The error message will be printed and then
 * only the sockets connected to the client are closed.
 *****************************************************************/
void closeConnection(char *errorMessage, int sockets[]) {
    fflush(stdout);
    printf("\n%s\n", errorMessage);
    printf("Closing connection to client.\n");

    int socketsToClose[] = {sockets[CONTROL_SOCKET], sockets[DATA_SOCKET]};

    closeSockets(socketsToClose, 2);
}

/*****************************************************************
 * Name: closeSockets
 * Preconditions:
 * @param sockets - array of sockets to be closed
 * @param numSockets - number of sockets to be closed
 * Postconditions: Closes provided sockets
 *****************************************************************/
void closeSockets(int sockets[], int numSockets) {
    int i = 0;

    for(i; i < numSockets; i++) {
        close(sockets[i]);
    }
}
