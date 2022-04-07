#ifndef EXCHANDL_H_INCLUDED
#define EXCHANDL_H_INCLUDED

#include <umps/libumps.h>
#include <umps/arch.h>
#include "umps/cp0.h"
#include "handlerFunction.h"

void exception_handler();

/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/
void interrupt_handler(state_t *excState);

void tlb_handler(state_t *callerProc);
void trap_handler(state_t *callerProc);

#endif