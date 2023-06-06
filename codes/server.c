/**
IoT Summer Camp 2023
Networking Assignment
**/
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include "helper.c"

#define HEADER_SIZE 100
#define MAX_CLIENTS 1

static int socket_fd;
static int client_fd;

static char* directory_name;

typedef enum { GET, PUT, DELETE, V_UNKNOWN } verb;

//for client
static verb request;
static char* filename;  //need free
static size_t file_size;
static FILE* file;
static char* header;

void close_server();
int run_server(char *port);
void handle_client(int client_fd);
void write_error(int client_fd);
void read_header(int client_fd);
void delete_file_from_dir(int client_fd);
void read_data(int client_fd);
void read_size(int client_fd);
void write_ok_preprocess(int client_fd);
void write_ok(int client_fd);
void finish_serving(int client_fd);
void clean_up_client(int client_fd);
void write_size(int client_fd);
void write_data(int client_fd);
void write_error_preprocess(int client_fd);

int main(int argc, char **argv) {
    //check argv & get port
    if (argc != 2) {
        //print_server_usage
        fprintf(stderr, "./server <port>\n");
        exit(1);
    }

    //mktempdir for the client
    directory_name = strdup("temp_dir");
    mkdir(directory_name, 0777);
    
    // build server: argv[1]:port
    socket_fd = run_server(argv[1]);

    // handle client
    handle_client(client_fd);

    //close server
    close_server();
    
    return 0;
}

/**
 * This is a signal handler for SIGPIPE.
 *
 * By default, the process will be killed once receiving SIGPIPE. We want to do nothing instead. i.e. "Don't kill the process!"
 */
void SIGPIPE_handler(int s) { /* do nothing */ }

/**
 * This is a signal handler for SIGINT.
 *
 * By default, the process will be killed once receiving SIGINT. We want to close the server properly before exiting!
 * Your server should exit on receiving SIGINT.
 */
void SIGINT_handler(int s) {
    close_server();
    _exit(1);
}

/**
 * This function build a server through "four steps" we talked about in the class
 * 
 * char* port: port number
 * 
 * Returns socket fd on success, else the program quit with the status code 1
 */

int run_server(char *port) {
    struct addrinfo hints;
    struct addrinfo * result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1) {
        perror(NULL);
        exit(1);
    }
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror(NULL);
        exit(1);
    }
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) < 0) {
        perror(NULL);
        exit(1);
    }
    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror(NULL);
        exit(1);
    }
    if(listen(socket_fd, MAX_CLIENTS) != 0) {
        perror(NULL);
        exit(1);
    }
    freeaddrinfo(result);
    client_fd = accept(socket_fd, NULL, NULL);
    return socket_fd;
}


/**
 * This function aimed to close the server properly by shutdown and close the client_fd & socket_fd
 */
void close_server() {
    //shutdown fd
    if (shutdown(client_fd, SHUT_RDWR)){
        perror(NULL);
        exit(1);
    }
    //close client_fd
    close(client_fd);

    //shutdown - socket_fd
    if (shutdown(socket_fd, SHUT_RDWR)){
        perror(NULL);
        exit(1);
    }
    //close socket_fd
    close(socket_fd);
}


/**
 * This function starts to handle clients. The purpose is to do a preprocess for the global variable
 * It will call read_header as the next step
 */
void handle_client(int client_fd) {
    header = calloc(HEADER_SIZE,1); 
    read_header(client_fd);
}

void read_header(int client_fd) {
    int flag = 1;
    ssize_t read_header_ret = server_read_all_header(client_fd, header, HEADER_SIZE);
    if (read_header_ret == -1) {
        flag = -1;
    }
    // parse the header
    // do things based on the REQUEST
    char* request_type = strtok(header, " ");
    filename = strtok(NULL, " ");
    if (!strcmp(request_type, "GET")) {
        // go here!
    } else if (!strcmp(request_type, "PUT")) {
        // go here!
    } else if (!strcmp(request_type, "DELETE")) {
        // go here!
    }
}


void write_ok_preprocess(int client_fd) {
    switch(request) {
        case GET: {
            char * pathname = calloc(1, strlen(directory_name)+strlen(filename)+2); //'/' & '\0'   //need free!!!
            sprintf(pathname, "%s/%s", directory_name, filename);
            file = fopen(pathname, "r");
            free(pathname);
            if(!file) {
                write_error_preprocess(client_fd);
                return;
            }
            //get filesize
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            rewind(file);
            
            break;
        }
        case PUT: {
            break;
        }
        case DELETE: {
            //check valid file
            char * pathname = calloc(1, strlen(directory_name)+strlen(filename)+2);
            sprintf(pathname, "%s/%s", directory_name, filename);
            FILE* temp = fopen(pathname, "r");
            if(!temp) {
                write_error_preprocess(client_fd);
                return;
            }
            fclose(temp);
            free(pathname);

            //now find it - delete
            delete_file_from_dir(client_fd); 
            break;
        }
        default: {/*should not reach here*/}
    }

    //write ok
    write_ok(client_fd);
}


void read_size(int client_fd) {
    //for PUT only!
    file_size = get_message_size(client_fd);
    //open file for write
    char * pathname = malloc(strlen(directory_name)+strlen(filename)+2); //'/' & '\0'   //need free!!!
    sprintf(pathname, "%s/%s", directory_name, filename);
    file = fopen(pathname, "w");
    free(pathname);
    read_data(client_fd);
}

void read_data(int client_fd) {
    // for PUT only!
    // ToDo: implement this function!

}


void write_error_preprocess(int client_fd){
    //reset header
    memset(header, 0, HEADER_SIZE);
    // go to write_error
    write_error(client_fd);
}


void write_error(int client_fd){
    //use header to store error msg (recall that from the doc)

    //write error msg: write_all_to_socket

    //check return value: what should we do when the value is -1?
}

void delete_file_from_dir(int client_fd){
    //ToDo: implement this function!
}


void write_ok(int client_fd) {
    //reset header & header_off

    //use header for Ok msg

    //write header: hint: write_all_to_socket

    //check return value of write_all_to_socket: what should we handle?

    //PUT & DELETE: are we done?

    //for GET only: what's next? Hint: project document!

}

void write_size(int client_fd){
    // for GET only
    // ToDo: Implement this function based on read_size

}


void write_data(int client_fd){
    //GET only
    if (request == GET) {
        ssize_t num_write = 0;
        char buffer[4096+1];
        ssize_t count = file_size;
        while(num_write < count) {
            memset(buffer, 0, 4096+1);
            ssize_t size = count-num_write;
            if (count-num_write > 4096) size = 4096;    //for large file
            fread(buffer, 1, size, file);
            ssize_t temp_size  = write_all_to_socket(client_fd, buffer, size);
            if (temp_size == -1) {
                fclose(file);
                clean_up_client(client_fd);
                return;
            }
            num_write += temp_size;  
        }

        //now means finish reading for the required size
        //close file
        fclose(file);
        finish_serving(client_fd);
    }
}

/**
* This function calls clean_up_client inside
*/
void finish_serving(int client_fd) {
    clean_up_client(client_fd);
}


/**
* This function aimed to free the heap-allocated memory we created for client 
*/
void clean_up_client(int client_fd) {
    if(filename) {
        free(filename);
        filename = NULL;
    }
    if(header) {
        free(header);
        header = NULL;
    }
}