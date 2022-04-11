#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include "../../phase1/headers/pcb.h"

/* Global Variables */
static int processId;           // Variabile globale utilizzata per assegnare un id unico ai processi creati
int activeProc;                 // Processi iniziati e non ancora terminati: attivi || Process Count
int blockedProc;                // Processi 'blocked': in attesa di I/O oppure timer || Soft-Block Count
struct list_head queueLowProc;  // Coda dei processi a bassa priorità
struct list_head queueHighProc; // Coda dei processi a alta priorità
pcb_PTR currentActiveProc;      // Puntatore processo in stato "running" (attivo) || Current Process
cpu_t startTime;                // Intero utilizzato per aggiornare il tempo di uso di un processo della cpu
int yieldHighProc;              // Variabile booleana usata per fare yield su processi ad alta priorita'

// Vettore di interi per i semafori dei device|| Device Semaphores
/* 
    * Consideriamo ogni coppia di semafori dei terminali come segue: 
    * Primo semaforo dedicato alle operazioni di scrittura (transm)
    * Secondo semaforo dedicato alle operazioni di lettura (recv)
*/  
int semIntervalTimer;
int semDiskDevice[8];
int semFlashDevice[8];
int semNetworkDevice[8];
int semPrinterDevice[8];
int semTerminalDeviceReading[8]; 
int semTerminalDeviceWriting[8];


#endif