/**********************************************
*  Filename: struct.h
*  Description: h file for struct used for thread
*  Author: Andrew Keenan, Christian Gulak
*  Date: 12/10/2024
*  Note: 
***********************************************/
#define OFF 0
#define COOLING 1
#define HEATING 2
typedef struct {
    int new_temp;
    int input_temp;
    int current_temp;
    int current_state;
    char prev_input[256];
} temps;