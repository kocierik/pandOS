#include "headers/syscall.h"

void get_tod(state_t *excState){
    unsigned int tod;
    STCK(tod);
    (*excState)->reg_v0 = tod; // TODO fatta a caso, da controllare
}

void terminate(state_t *excState){
    SYSCALL(TERMPROCESS,0,0,0);
}

void write_to_printer(state_t *excState){

}

void write_to_terminal(state_t *excState){

}

void read_from_terminal(state_t *excState){

}