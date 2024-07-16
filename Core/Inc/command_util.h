#ifndef COMMAND_UTIL_H
#define COMMAND_UTIL_H


//green PD12
//orange PD13
//red PD14
//bleu PD15

    typedef enum {
    GREEN=12,
    ORANGE,
    RED,
    BLUE
} LEDColor;

typedef enum {
    ON,
    OFF,
    TOGGLE,
    BLINK,

} Func;

typedef struct {
    Func func;
    LEDColor color;
    int delay;
} Command;


Command initCommand() ;
void resetCommand(Command *cmd) ;
Command parseReceivedCommand(char *str) ;




#endif /* LED_UTIL_H */
