#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define DATA_FILE_PATH "/var/tmp/aesdsocketdata"

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        // Log the signal and exit
        openlog("aesdsocket", LOG_PID, LOG_USER);
        syslog(LOG_INFO, "Caught signal, exiting");
        closelog();

        // Delete the data file
        remove(DATA_FILE_PATH);

        // Exit the program
        exit(EXIT_SUCCESS);
    }
}

// Function to run the program as a daemon
void run_as_daemon() {
    pid_t pid, sid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    // If we got a good PID, then we can exit the parent process.
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Open any logs here
    openlog("aesdsocket", LOG_PID, LOG_USER);

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Could not create a new session ID");
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Could not change working directory to /");
        exit(EXIT_FAILURE);
    }

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Run the main program
}

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int daemon_mode = 0;

    // Check for daemon mode option
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (daemon_mode) {
        // Run as daemon
        run_as_daemon();
    }

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to port 9000
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return -1;
    }

    // Accept connections in a loop
    while (1) {
        // Accept connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Log accepted connection to syslog
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        openlog("aesdsocket", LOG_PID, LOG_USER);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        closelog();

        // Receive data and append to file
        char buffer[BUFFER_SIZE];
        FILE *data_file = fopen(DATA_FILE_PATH, "a");
        ssize_t bytes_received;
        while ((bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_received, data_file);
            // Check for newline to indicate end of packet
            if (memchr(buffer, '\n', bytes_received) != NULL) {
                break;
            }
        }
        fclose(data_file);

        // Send back the content of the data file to the client
        data_file = fopen(DATA_FILE_PATH, "r");
        while ((bytes_received = fread(buffer, 1, BUFFER_SIZE, data_file)) > 0) {
            send(new_socket, buffer, bytes_received, 0);
        }
        fclose(data_file);

        close(new_socket);

        // Log closed connection to syslog
        openlog("aesdsocket", LOG_PID, LOG_USER);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
        closelog();
    }

    close(server_fd);
    return 0;
}
