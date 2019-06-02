/*****************************************************************
 * Name: Chelsea Egan
 * Course: CS 372-400
 * Program: ftserver.c
 * Description: This is the server for a file transfer system. It
 * opens a connection for clients and, once a request is received,
 * connects to a control socket. It gets commands from the client
 * and sends the response over a separate data connection.
 * Last Modified: June 2, 2019
*****************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ftutilities.h"

int main(int argc, char *argv[]) {
    int welcomeSocket, controlSocket, dataSocket;
    int sockets[] = {welcomeSocket, controlSocket, dataSocket};
    char clientHostName[MAXBUFFERSIZE] = {0};
    char *controlPort = argv[1];

    // Create welcoming socket
    startUp(argc, controlPort, sockets);

    // Run until program is terminated
    while(1) {
        // Listen for connection requests
        acceptConnections(sockets, clientHostName);

        // Respond to any commands issued by client
        handleRequests(sockets, clientHostName);
    }

    return 0;
}