#ifndef HANDFUNC_H_INCLUDED
#define HANDFUNC_H_INCLUDED

#include <umps/libumps.h>
#include <umps/arch.h>
#include "syscall.h"
//#include "../klog.c"

/*
* La funzione chiama l'opportuno interrupt in base al primo device che trova in funzione.
* Per vedere se un device è in funzione utilizziamo la macro CAUSE_IP_GET che legge gli opportuni bit di CAUSE e
* restituisce 1 quando un dispositivo è attivo. 
* //N.B. La funzione CAUSE_GET_IP è ben commentata dov'è definita.
*/
int interruptHandler();

void passOrDie();
int TLBHandler();
void trapHandler();
void syscall_handler(int SYS);

#endif