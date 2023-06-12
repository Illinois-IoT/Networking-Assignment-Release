/**
IoT Summer Camp 2023
Networking Assignment
**/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include "helper.c"

// an enum you may find helpful
typedef enum { GET, PUT, DELETE, V_UNKNOWN } verb;

//functions we are providing, you may find them helpful
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(char *host, char *port);
void write_to_server(char** cmds, verb operation, int socket_fd);
void read_from_server(char* local, verb operation, int socket_fd);
void print_client_help();


int main(int argc, char **argv) {

    //parse args: in the form of {host0, port1, method2, remote3, local4, NULL}
    char ** cmds = parse_args(argc, argv);  //need free!!!

    //check args
    verb operation = check_args(cmds);

    //connect to server
    int socket_fd = connect_to_server(cmds[0], cmds[1]);

    //write to server
    write_to_server(cmds, operation, socket_fd);

    //read from server
    read_from_server(cmds[4], operation, socket_fd);

    //close socket_fd
    close(socket_fd);
    
    //free
    free(cmds);

    return 0;
}

/**
 * This function build the connection with the server through the "three steps".
 * 
 * char* host: the host name
 * char* port: port number
 * 
 * Returns socket fd on success, else the program quit with a status 1
 */
int connect_to_server(char *host, char *port) {
    //Good Luck!
    return 0;
}


/**
 * This function read the response from the server, following certain format provided by the doc
 * 
 * char* local: local filename
 * verb operation: request type
 * int socket_fd: socket fd returned from "connect_to_server()"
 */
void read_from_server(char* local, verb operation, int socket_fd) {
    //1. get RESPONSE code
    char * response = malloc(6+1);    //need free
    memset(response, 0, 7);
    ssize_t read_response_ret = read_all_from_socket(socket_fd, response, 3);
    if (read_response_ret == 0) {
        printf("connection closed\n");
        exit(1);
    }
    if (read_response_ret == -1) {
        printf("connection closed\n");
        exit(1);
    }

    //2. Handle "OK"
    if (strcmp(response, "OK\n") == 0) {
        //PUT & DELETE - finish
        if (operation == PUT || operation == DELETE) {
            // ToDo: your code go here
        }

        //GET
        if (operation == GET) {
            //ToDo: a. read [size]: HINT: a function in helper.c


            //ToDo: b. read [binary data]
            FILE* file = fopen(local, "w");   //need close
            //HINT: mimic "write [binary data]" in write_to_server function




            //close file
            fclose(file);
        }

            
    } 
    //3. Handle "ERROR"
    else {
        read_all_from_socket(socket_fd, response+3, 3);
        if (strcmp(response, "ERROR\n") != 0) {
            printf("Invalid Response");
            exit(1);
        } 
    }

    //4. Clean up:

    //a. free?

    //b. shutdown? which end?
}



/**
 * This function write the request to the server, following certain format corresponding to certain request type.
 * 
 * char** cmds: {host0, port1, method2, remote3, local4, NULL}
 * verb operation: request type
 * int socket_fd: socket fd returned from "connect_to_server()"
 */
void write_to_server(char** cmds, verb operation, int socket_fd) {
    char* msg = NULL;       //need free

    //1. write header to server
    switch(operation){
        case GET: {
            msg = calloc(1, strlen(cmds[2]) + strlen(cmds[3]) + 3); 
            sprintf(msg, "%s %s\n", cmds[2], cmds[3]);
            break;
        }

        //Add more stuff here!

        default: {/*should not reach this line*/}
    }
    ssize_t write_ret = write_all_to_socket(socket_fd, msg, strlen(msg));
    if (write_ret == -1) exit(1);
    if (write_ret == 0) {
        printf("lost connection\n");
        exit(1);
    }

    //2. What else? Only PUT have [File size][Binary Data]
    if (operation == PUT) {
        FILE* file = fopen(cmds[4], "r");   //need close
        if (!file) {
            exit(1);
        }



        //ToDo: write [size]

        //a. get file size: HINT: stat

        //b. write size (HINT: use afunction in helper.c; you need to define size variable with name "count") & check return value of write




        //write [binary data]
        ssize_t num_write = 0;
        char buffer[4096];
        while(num_write < count) {
            ssize_t size = count-num_write;
            if (size > 4096) size = 4096;    //for large file
            fread(buffer, size, 1, file);
            ssize_t temp_size = write_all_to_socket(socket_fd, buffer, size);
            if (temp_size == -1) exit(1);
            if (temp_size == 0) {
               printf("connection closed\n");
                exit(1);
            }
            num_write += temp_size;
        }


        //ToDo: close file

    }


    //3. cleanup

    //a. free?

    // b. shutdown? which end?

}


/**
 * This is a function that we are providing
 *
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 *
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    return GET;
}



/**
 * This is a client help message regarding the client usage and the request types.
 */

void print_client_help() {
    printf("./client <host>:<port> <method> [remote] [local]\n");
    printf("Methods:\n \
        PUT <remote> <local>\tUploads <local> file to serve as filename <remote>.\n \
        GET <remote>\t\tDownloads file named <remote> from server.\n \
        DELETE <remote>\tDeletes file named <remote> on server.\n");
}
