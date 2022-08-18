#include "headers/supSyscall.h"

cpu_t get_tod(state_t *excState){
    cpu_t tod;
    STCK(tod);
    return tod;
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