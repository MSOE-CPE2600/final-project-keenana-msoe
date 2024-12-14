# Thermostat Final Project
## Brief introduction
This final project is a simulation of a thermostat with a controller that has an attached AC and Heater. A controller (controller.c) controls the therrmostat where the back end of the project holds the state machine. The two processes communicate through shared memory although it was intended to be 2 named pipes.
## Design process / Code Breakdown
### Struct.h
this is a header file which holds the temps structure that the rest of the back end code uses.
### Thermostat.c
This file contains the state machine function and logic for the thermostat. The single function is a void * function so that a thread can be used. The function also passes in the temps struct.
### Backend.c
The backend function runs a timer with a continuous loop where every 5 seconds the shared memory from the controller is sampled and the input temperature is taken into account. After the reading in of user input the program updates a temps struct and then sends the data to the thermostat to update the state machine. Finally the program sends a formatted output message to the front end before waiting for the timer again.
### Controller.c
Handles the user input part of the thermostat and takes in the user input in a continuous loop which printing the output from the back end also in a continuous loop. The function reads in the user input and passes it to the back end via a piece of shared memory and then waits for the back end to return a message which is printed to the user.
### New techniques
Mainly the new topic is the use of timers and sending the SIGALRM signal. The project alos includes multiple uses of threads and was supposed to include named pipes but was implemented incorrectly. Signal handling is also used through the timer and also the exit process to make sure that cleanup is done properly.
## Use of thermostat
To use the thermostat open up 2 terminals and run controller and backend. From the controller simply enter in input integers for the thermostat and watch the magic happen.