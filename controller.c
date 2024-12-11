/**********************************************
*  Filename: controller.c
*  Description: front end controller for the thermostat
*  Author: Andrew Keenan, Christian Gulak
*  Date: 12/10/2024
*  Note: gcc -o controller controller.c
***********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define BUFFER_SIZE 256
#define PIPE_TO_BACKEND "/tmp/pipe_to_backend"
#define PIPE_FROM_BACKEND "/tmp/pipe_from_backend"

// Global buffer to store received data
char buffer[BUFFER_SIZE];
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to handle receiving data from the backend
void *receive_data(void *arg) {
    int read_pipe = *(int *)arg;

    while (1) {
        char recv_buffer[BUFFER_SIZE];
        ssize_t bytes_received = read(read_pipe, recv_buffer, sizeof(recv_buffer) - 1);
        if (bytes_received > 0) {
            recv_buffer[bytes_received] = '\0';

            pthread_mutex_lock(&buffer_mutex);
            strncpy(buffer, recv_buffer, BUFFER_SIZE);
            pthread_mutex_unlock(&buffer_mutex);

            // Print the received message
            printf("Temperature is: %s\n", buffer);
        }
    }

    return NULL;
}

// signal handler for cleanup
void handle_signal(int sig) {
    pthread_cancel(receiver_thread);
    pthread_join(receiver_thread, NULL);
    close(to_backend_fd);
    close(from_backend_fd);
    unlink(PIPE_TO_BACKEND);
    unlink(PIPE_FROM_BACKEND);
    return 0;
}

int main() {
    // register the signal handler because it will never exit unless cntl + c
    signal(SIGINT, handle_signal);
    int to_backend_fd, from_backend_fd;
    pthread_t receiver_thread;

    // Create named pipes if they don't exist
    if (mkfifo(PIPE_TO_BACKEND, 0666) == -1 && access(PIPE_TO_BACKEND, F_OK) == -1) {
        perror("Failed to create pipe to backend");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(PIPE_FROM_BACKEND, 0666) == -1 && access(PIPE_FROM_BACKEND, F_OK) == -1) {
        perror("Failed to create pipe from backend");
        exit(EXIT_FAILURE);
    }

    // Open the pipes
    if ((to_backend_fd = open(PIPE_TO_BACKEND, O_WRONLY)) == -1) {
        perror("Failed to open pipe to backend");
        exit(EXIT_FAILURE);
    }

    if ((from_backend_fd = open(PIPE_FROM_BACKEND, O_RDONLY)) == -1) {
        perror("Failed to open pipe from backend");
        close(to_backend_fd);
        exit(EXIT_FAILURE);
    }

    // Start the receiver thread
    if (pthread_create(&receiver_thread, NULL, receive_data, &from_backend_fd) != 0) {
        perror("Failed to create receiver thread");
        close(to_backend_fd);
        close(from_backend_fd);
        exit(EXIT_FAILURE);
    }

    // Main loop to send user input
    while (1) {
        char user_input[BUFFER_SIZE];
        printf("Enter desired temperature: ");
        fgets(user_input, sizeof(user_input), stdin);

        // Convert input to integer
        int desired_temperature = atoi(user_input);
        if (desired_temperature == 0 && user_input[0] != '0') {
            printf("Invalid input. Please enter a valid integer.\n");
            continue;
        }

        // Send the desired temperature to the backend
        snprintf(user_input, sizeof(user_input), "%d", desired_temperature);
        if (write(to_backend_fd, user_input, strlen(user_input)) == -1) {
            perror("Failed to send data to backend");
        } else {
            printf("Sent desired temperature: %d\n", desired_temperature);
        }
    }

    // Clean up when sigkill is run
    return 0;
}
