#!/usr/bin/env python3

#######################################################################
# Name: Chelsea Egan
# Course: CS 372-400
# Program: ftclient.py
# Description: This is a client of a file transfer system. The client
# connects to the server and then can either request the listing of the
# server's directory or to receive a file from the directory.
# Source for socket functionality: 
# https://docs.python.org/3/library/socket.html
# Last Modified: June 2, 2019
#######################################################################

import socket
import sys
import errno
import os.path

CONFIRMATION = 'ACK'

######################################################
# Name: validate_args
# Preconditions: command line input
# Postconditions: If any of the command line inputs do
# not match the expected format, the program will
# display an error message and terminate.
# Returns the data_port_index to indicate the type of
# command issued.
# Source for input validation method:
# https://www.101computing.net/number-only/
# I reused some of my validation code from Project 1
######################################################
def validate_args():
    data_port_index = -1;
    
    # Check if correct number of arguments in command line
    if len(sys.argv) != 5 and len(sys.argv) != 6:
        print('\nIncorrect number of arguments provided')
        invalid_args()

    # Check type of command
    if sys.argv[3] == '-l':
        data_port_index = 4
    elif sys.argv[3] == '-g':
        data_port_index = 5
    else:
        print('\nInvalid command')
        invalid_args()

    # Check if port args are integers
    try:
        control_port = int(sys.argv[2])
        data_port = int(sys.argv[data_port_index])
    except ValueError:
        print('\nPorts must be integers')
        invalid_args()

    # Check if ports are in proper range
    if control_port < 1024 or data_port < 1024 or control_port > 65535 or data_port > 65535:
        print ('\nPorts should be an integer between 1024-65535')
        invalid_args()

    # Check that controlPort and dataPort are not the same
    if control_port == data_port:
        print('\nPorts should not match each other')
        invalid_args()

    return data_port_index

######################################################
# Name: invalid_args
# Preconditions: invalid input on the command line
# Postconditions: Prints an error message and ends
# the program
######################################################
def invalid_args():
    print('\nInvalid command line arguments.')
    print('To get the directory, try:')
    print('     ./ftclient.py <hostname> <port> -l <data port>')
    print('To get a file, try:')
    print('     ./ftclient.py <hostname> <port> -g <filename> <data port')
    print('\nGoodbye!')
    sys.exit()

######################################################
# Name: initiate_contact
# Preconditions: User entered a string and a number in
# the range of 1024-65535 on the command line on start
# Inputs have already been validated
# Postconditions: Creates a socket connection with the
# provided host/port
# Source for setting up socket connection:
# Computer Networking by Kurose & Ross, section 2.7.2
######################################################
def initiate_contact():
    hostname = sys.argv[1]
    port = int(sys.argv[2])

    try:
        # Create socket with the address family ipv4 and TCP protocol
        control_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Make connection with server passing hostname and port
        control_socket.connect((hostname, port))
        return control_socket
    except:
        print('\nError creating control socket\n')
        raise

######################################################
# Name: init_data_conn
# Preconditions: User provided a second port in the
# range of 1024-65535 on the command line on start
# Inputs have already been validated
# Postconditions: Opens a socket to be used by the
# server for the data connection
# Source for setting up socket connection:
# Computer Networking by Kurose & Ross, section 2.7.2
# I reused the socket setup code from Project 1
######################################################
def init_data_conn(data_port):
    hostname = socket.gethostname()
    host_address = socket.gethostbyname(hostname)
    port = int(sys.argv[data_port])

    try:
        # Create socket with the address family ipv4 and TCP protocol
        welcome_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Make the port resuable after it's closed
        welcome_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # Set the socket address and port according to the provided input
        welcome_socket.bind((host_address, port))

        # Listen for incoming connections
        welcome_socket.listen(1)
        
        print('\nThe client is ready to receive on\nhostname: '
              + hostname + '\nport: ' + str(port) + '\n')
        return welcome_socket
    except:
        print('\nError creating data socket\n')
        raise

######################################################
# Name: send_command
# Preconditions: socket connected to a control
# connection with the server and the type of command
# that was input on the command line. Input was
# already validated.
# Postconditions: Attempts to send the command to the
# client. If an error is encountered, the connection
# with the client is ended.
######################################################
def send_command(socket, command_type):
    command = sys.argv[3]

    try:
        # Send the command to the server
        socket.send(command.encode())

        # Server should respond with ACK
        isSuccess = receive_confirmation(socket)

        # Command: list directory
        # Send port to server
        # Get confirmation
        if command_type == 4 and isSuccess:
            port = sys.argv[4]
            socket.send(port.encode())
            return receive_confirmation(socket)
        # Command: send file
        # Send port to server
        # Send filename to server
        # Get confirmation for both
        if command_type == 5 and isSuccess:
            port = sys.argv[5]
            socket.send(port.encode())
            if receive_confirmation(socket):
                filename = sys.argv[4]
                socket.send(filename.encode())
                return receive_confirmation(socket)
            else:
                return False

    except:
        print('\nError encountered sending command to server!\n')
        raise
        
######################################################
# Name: receive_confirmation
# Preconditions: connection made with server
# Postconditions: Reads from the socket buffer
# expecting an "ACK" to indicate success. If receives
# anything other than an "ACK", indicates error.
# Source for error handling: https://stackoverflow.com/a/16745561
# I reused some code from Project 1
######################################################
def receive_confirmation(socket):
    try:
        # Will accept up to 128B of data
        response = socket.recv(1024).decode()

        # Checks if received an "ACK" from server
        if response == CONFIRMATION:
            return True
        else:
            print('\nError received from server!\n')
            return False
    except:
        print('\nError encountered receiving message from ftserver!\n')
        raise

######################################################
# Name: get_directory
# Preconditions: connection made with server and
# server will send a message that ends with "ACK"
# Postconditions: Reads from the socket buffer until
# an "ACK" is received. Returns the received message.
######################################################
def get_directory(socket):
    incoming_message = ''
    total_message = ''

    try:
        # Receives until the last three characters are "ACK"
        while incoming_message[-3:] != CONFIRMATION:
            incoming_message = socket.recv(1024).decode()
            # If contains the "ACK", stores the rest of the message
            if incoming_message[-3:] == CONFIRMATION:
                total_message += incoming_message[:-3]
            else:
                total_message += incoming_message

        # Display the server's directory
        print('\nDIRECTORY:')
        print(total_message)
    except:
        print('\nError encountered receiving message from ftserver!\n')
        raise

######################################################
# Name: check_duplicates
# Preconditions: user provided filename on command line
# Postconditions: Checks if client already has a file
# of that name in its directory. If so, asks if the
# file should be overwritten or if the incoming file
# should be given a new name. Returns file name or
# cancels based on user choice.
# Source for checking if file exists:
# https://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists-without-exceptions
######################################################
def check_duplicates():
    file_name = sys.argv[4]

    # Check if file already in directory
    if(os.path.exists(file_name)):
        print('This file already exists: ' + file_name)
        while True:
            # Option to overwrite existing
            overwrite = input('Overwrite? Y/N\n')
            if overwrite.upper() == 'Y':
                return file_name
            elif overwrite.upper() == 'N':
                # Option to rename new file
                rename = input('Rename file? Y/N\n')
                if rename.upper() == 'Y':
                    # Adds(#) to name- cycles until a unique name is found
                    # E.g. "file(1).txt" already exists, would try "file(2).txt"
                    index = 0
                    new_file_name = file_name
                    while(os.path.exists(new_file_name)):
                        index += 1
                        new_file_name = file_name[:-4] + '(' + str(index) + ')' + file_name[-4:]
                    print('File renamed as: ' + new_file_name)
                    return new_file_name
                # User chooses to not rename or overwrite
                # Cancels operation
                elif rename.upper() == 'N':
                    print('Cancelling command to get file\n')
                    return None
    return file_name

######################################################
# Name: get_file
# Preconditions: connection made with server and
# server will send a file
# Postconditions: If server sends an error, indicates
# that requested file does not exist and closes
# connection. Else, reads in file from buffer until
# 'ACK' is received to indicate EOF. Saves the file.
# Source for writing to a file:
# https://docs.python.org/3/tutorial/inputoutput.html
######################################################           
def get_file(socket):
    incoming_message = ''
    total_message = ''

    # Confirm that the server has the requested file
    response = socket.recv(1024).decode()
    if response == CONFIRMATION:
        file_name = check_duplicates()
    else:
        print('\nThat file does not exist.\n')
        return  
    
    if file_name:
        try:
            # Inform server that client is ready to receive
            socket.send(CONFIRMATION.encode())

            # Open the file
            with open(file_name, 'w') as file:
                # Save incoming into file until 'ACK'
                while True:
                    incoming_message = socket.recv(1024).decode()
                    if incoming_message[-3:] == CONFIRMATION:
                        file.write(incoming_message[:-3])
                        break
                    else:
                        file.write(incoming_message)
            print('File transfer complete')
        except:
            print('\nError encountered receiving message from ftserver!\n')
            raise

######################################################
# Name: close_connection
# Preconditions: sockets to be closed
# Postconditions: Closes the data and control sockets
######################################################
def close_connection(control_socket, data_socket):
    control_socket.close()
    data_socket.close()

######################################################
# Name: make_request
# Preconditions: the control socket with the server
# and the command requested
# Postconditions: Sends command to server and receives
# data back. When done, closes connections to server.
######################################################
def make_request(control_socket, command_type):
    # If successfully sends commands to server
    if send_command(control_socket, command_type):
        # Data connection for receiving from server
        welcome_socket = init_data_conn(command_type)

        # Let server know client is ready for connection
        control_socket.send(CONFIRMATION.encode())
        data_socket, addr = welcome_socket.accept()

        # Get response for server based on command
        if command_type == 4:
            get_directory(data_socket)
        elif command_type == 5:
            get_file(data_socket)

    # Done - close sockets
    close_connection(control_socket, data_socket)

######################################################
# MAIN FUNCTION
# (1) Validates the command line arguments
# (2) Creates a control connection with the server
# (3) Sends command to the server
# (4) Creates a data connection with the server
# (5) Receives requested input from server
# (6) Entering Ctrl+C will terminate the program
# Source for setting up main function:
# https://www.guru99.com/learn-python-main-function-with-examples-understand-main.html
# Source for handling Ctrl+C:
# https://stackoverflow.com/questions/1187970/how-to-exit-from-python-without-traceback
# Source for error handling: https://stackoverflow.com/a/9015233
# Source for main function structure: My code from Project 1
######################################################
def main():
    try:
        # Validate all inputs from user
        command_type = validate_args()
        
        # Control connection with server
        control_socket = initiate_contact()

        # Send command and get data from servr
        make_request(control_socket, command_type)

    # User enters Ctrl+C
    except KeyboardInterrupt:
        print ('\n\nGoodbye!')
        close_connection(control_socket, data_socket)
    except Exception as e:
        print(e)
        close_connection(control_socket, data_socket)
        
    sys.exit(0)
    
if __name__=='__main__':
    main()
