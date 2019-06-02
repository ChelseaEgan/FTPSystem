/*****************************************************************
 * Name: Chelsea Egan
 * Course: CS 372-400
 * Program: ftutilities.h
 * Description: This file provides the declaration of functions
 * used by ftutilities.c
 * Last Modified: June 2, 2019
*****************************************************************/

#ifndef PROJECT_2_FTUTILITIES_H
#define PROJECT_2_FTUTILITIES_H

#include <stdbool.h>

#define BACKLOG 10          // Size of pending connections queue
#define MAXBUFFERSIZE 256   // Max message size that can be received
#define CONFIRMATION "ACK"  // Message sent for confirmation
#define REJECTION "NAK"     // Message sent if an error
#define LIST_COMMAND 1      // Client requested the directory
#define FILE_COMMAND 2      // Client requested a file

enum SocketTypes {
    WELCOME_SOCKET,
    CONTROL_SOCKET,
    DATA_SOCKET
};

void startUp(int, char [], int[]);
void validateCommandLineArguments(int, int[]);
char* validatePortNumber(char *, int[]);
void createSocket(char *, int[]);
int initDataConnection(char *, char *, int[]);
void acceptConnections(int [], char *);
void handleRequests(int [], char *);
int receiveCommand(int *);
int receiveDataPort(int *, char *);
int receiveFileName(int *, char *);
int receiveMessage(int *, char *);
bool validateFileName(char *);
int sendFile(int[], char *);
int sendMessage(char *, int *);
int sendDirectory(int[]);
void terminateProgram(char *, int[]);
void closeConnection(char *, int[]);
void closeSockets(int[], int);

#endif //PROJECT_2_FTUTILITIES_H
