/**
IoT Summer Camp 2023
Networking Assignment
**/
#include <stddef.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

ssize_t get_message_size(int socket);
ssize_t write_message_size(size_t size, int socket);
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);
ssize_t server_read_all_header(int socket, char *buffer, size_t count);

/**
 * Reads the bytes of size to the socket
 *
 * Returns the number of bytes successfully read,
 * 0 if socket is disconnected, or -1 on failure
 */
// You may assume size won't be larger than a 4 byte integer
ssize_t get_message_size(int socket) {
    size_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, sizeof(size_t));
    if (read_bytes == 0 || (int)read_bytes == -1) {
        printf("Invalid Response\n");
        exit(1);
    }

    return (size_t)(size);
}

/**
 * Writes the bytes of size to the socket
 *
 * Returns the number of bytes successfully written,
 * 0 if socket is disconnected, or -1 on failure
 */
// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    //Good Luck!
    return 0;
}

/**
 * Attempts to read all count bytes from socket into buffer.
 * Assumes buffer is large enough.
 *
 * Returns the number of bytes read, 0 if socket is disconnected,
 * or -1 on failure.
 */
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    //Good Luck!
    return 0;
}


/**
 * Attempts to write all count bytes from buffer to socket.
 * Assumes buffer contains at least count bytes.
 *
 * Returns the number of bytes written, 0 if socket is disconnected,
 * or -1 on failure.
 */
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    size_t num_write = 0;
    while(num_write < count) {
        ssize_t ret = write(socket, buffer, count-num_write);
        if (ret == 0) return num_write;
        if (ret == -1 && errno == EINTR) {
            continue;
        }
        if (ret == -1) return -1;
        num_write += ret;
        buffer += ret;
    }
    return num_write;
}

ssize_t server_read_all_header(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t num_read = 0;
    while(num_read < count) {
        ssize_t ret = read(socket, buffer, 1);
        if (ret == 0) {
            return num_read;
        }
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) return -1;
        
        //check for '\n'
        if (buffer[0] == '\n') {
            // Do not store the newlines in your filenames & make it end with '\0'
            buffer[0] = '\0';
            return num_read;
        }

        //update value
        num_read += ret;
        buffer += ret;
    }
    
    return num_read;
}

