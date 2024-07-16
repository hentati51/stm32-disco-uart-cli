/*
 * command_info.c
 *
 *  Created on: 22 févr. 2024
 *      Author: achraf
 */

#include "command_util.h"
#include "string.h"
#include <stdlib.h>


Command initCommand() {
    Command cmd;
    cmd.color = 9;
    cmd.func = 9;
    cmd.delay = 1000;
    return cmd;
}



Command parseReceivedCommand(char *str) {
    Command cmd = initCommand();

    // Tokenize the received string to extract color, function, and delay
    char *token = strtok(str, " ");
    if (token != NULL) {
        if (strcmp(token, "red") == 0) {
            cmd.color = RED;
        } else if (strcmp(token, "blue") == 0) {
            cmd.color = BLUE;
        } else if (strcmp(token, "green") == 0) {
            cmd.color = GREEN;
        } else if (strcmp(token, "orange") == 0) {
            cmd.color = ORANGE;
        }
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        if (strcmp(token, "on") == 0) {
            cmd.func = ON;
        } else if (strcmp(token, "off") == 0) {
            cmd.func = OFF;
        } else if (strcmp(token, "toggle") == 0) {
            cmd.func = TOGGLE;
        } else if (strcmp(token, "blink") == 0) {
            cmd.func = BLINK;
            token = strtok(NULL, " "); // Get the next token for delay
            if (token != NULL) {
                cmd.delay = atoi(token); // Convert delay string to integer
            }
        }
    }

    return cmd;
}

void resetCommand(Command *cmd) {
    *cmd = initCommand();
}

