/**********************************************
*  Filename: thermostat.c
*  Description: c file for the thermostat 
*  Author: Andrew Keenan, Christian Gulak
*  Date: 12/10/2024
*  Note: 
***********************************************/
#include "struct.h"
void *state_machine(void *args) {
    temps *data = (temps*)args;
    int current_temp = data->current_temp;
    int input_temp = data->input_temp;
    int state = data->current_state;
    switch (state) {
        case OFF:
            if (input_temp > current_temp) {
                // switch to heating
                state = HEATING;
            } else if (input_temp < current_temp) {
                // switch to cooling
                state = COOLING;
            }
            data->new_temp = current_temp;
            break;
        case COOLING:
            if (input_temp == current_temp) {
                // turn off
                state = OFF;
                data->new_temp = current_temp;
            } else if (input_temp > current_temp) {
                // go to heating
                state = HEATING;
                data->new_temp = current_temp;
            } else {
                data->new_temp = current_temp - 1;
            }
            break;
        default:
            // heating state
            if (input_temp == current_temp) {
                // turn off
                state = OFF;
                data->new_temp = current_temp;
            } else if (input_temp < current_temp) {
                // go to cooling
                state = COOLING;
                data->new_temp = current_temp;
            } else {
                data->new_temp = current_temp + 1;
            }
            break;
    }
    data->current_state = state;
    return 0;
}