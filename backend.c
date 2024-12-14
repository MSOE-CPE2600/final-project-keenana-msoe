/**********************************************
*  Filename: backend.c
*  Description: back end display for the thermostat
*  Author: Andrew Keenan, Christian Gulak
*  Date: 12/10/2024
*  Note: gcc -o backend backend.o thermostat.o
***********************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include "struct.h"
#include "thermostat.h"
#define PIPE_TO_BACKEND "/tmp/pipe_to_backend"
#define PIPE_FROM_BACKEND "/tmp/pipe_from_backend"
#define BUFFER_SIZE 256

int run = 1;
int sample = 0;

void cleanup(int signal) {
    run = 0;
}

// timer function
void timer(int signal) {
    sample = 1;
}

int main () {
    // register the signal handler because it will never exit unless cntl + c
    signal(SIGINT, cleanup);
    signal(SIGALRM, timer);
    // initialize
    struct itimerval itimer;
    temps temp_data;
    char buffer[BUFFER_SIZE] = {0};
    char prev_input[BUFFER_SIZE] = {0};
    pthread_t thread;
    int from_backend_fd, to_backend_fd;

    // Initialize temps struct
    temp_data.current_temp = 70;
    temp_data.new_temp = 70;
    temp_data.input_temp = 70;
    temp_data.current_state = OFF;

    to_backend_fd = open(PIPE_TO_BACKEND, O_CREAT | O_RDONLY | O_NONBLOCK, 0666);
    from_backend_fd = open(PIPE_FROM_BACKEND, O_CREAT | O_WRONLY, 0666);
    if (to_backend_fd == -1 || from_backend_fd == -1) {
        perror("Pipe error");
        exit(EXIT_FAILURE);
    }

    // Configure and start the timer
    itimer.it_value.tv_sec = 5;
    itimer.it_value.tv_usec = 0;
    itimer.it_interval.tv_sec = 5;
    itimer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &itimer, NULL);

    while (run) {
        if (sample) {
            // read in user data from controller
            int bytes_read = read(to_backend_fd, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                // are bytes read from the buffer?
                buffer[bytes_read] = '\0';
                temp_data.input_temp = atoi(buffer);
                strcpy(prev_input, buffer);
            } else if (strlen(prev_input) > 0) {
                // was there a last input?
                temp_data.input_temp = atoi(prev_input);
            } else {
                // no last input ex. start of run just pass current
                temp_data.input_temp = temp_data.current_temp;
            }

            // kick off thread for state machine
            pthread_create(&thread, NULL, state_machine, (void*)&temp_data);
            // wait for state machine to finish
            pthread_join(thread, NULL);

            // write to frontend
            char output[BUFFER_SIZE];
            snprintf(output, sizeof(output), "The desired temp is %d, the current temp is %d.\n",
                temp_data.input_temp, temp_data.new_temp);
            write(from_backend_fd, output, strlen(output));

            // update the current temp
            temp_data.current_temp = temp_data.new_temp;

            // status message
            printf("itemp: %d\nctemp: %d\ncstate: %d\n\n", 
                temp_data.input_temp, temp_data.current_temp, temp_data.current_state);
        }
        pause();
    }

    // Cleanup
    close(to_backend_fd);
    close(from_backend_fd);
    unlink(PIPE_TO_BACKEND);
    unlink(PIPE_FROM_BACKEND);
    pthread_cancel(thread);
    return 0;
}