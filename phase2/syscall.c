#include "headers/syscall.h"

extern void klog_print(char *s);
extern void klog_print_dec(unsigned int num);
extern void klog_print_hex(unsigned int num);

/* Variabili globali esterne */
extern int activeProc;
extern int blockedProc; 
extern int processId;
extern struct list_head queueLowProc;
extern struct list_head queueHighProc;
extern pcb_t *currentActiveProc;
extern int semIntervalTimer;
extern int semDiskDevice[8];
extern int semFlashDevice[8];
extern int semNetworkDevice[8];
extern int semPrinterDevice[8];
extern int semTerminalDeviceReading[8]; 
extern int semTerminalDeviceWriting[8];
extern cpu_t startTime;

/* Funzioni globali esterne */
extern void assegnaPID(pcb_PTR p);
extern void insert_ready_queue(int prio, pcb_PTR p);


/* FUNZIONI DI AIUTO */


void copyState(state_t *new, state_t *old) {
    old->cause = new->cause;
    old->entry_hi = new->entry_hi;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        old->gpr[i] = new->gpr[i];
    old->hi = new->hi;
    old->lo = new->lo;
    old->pc_epc = new->pc_epc;
    old->status = new->status;
}


/* funzione iterativa che elimina i figli e il processo stesso */
int term_proc_and_child(pcb_PTR parent) {
    int ret = FALSE;
    pcb_PTR p;
    while(!isPcbFree(parent->p_pid)) {
        p = parent;
        while(!emptyChild(p))
            p = container_of(p->p_child.next, pcb_t, p_sib);
        
        ret = __terminate_process(p); // termino p
    }
    return ret;
}


int __terminate_process(pcb_PTR p) {
    int ret = FALSE;
    // gestisco variabili globali e semaforo
    if (p->p_semAdd == NULL) {
        list_del(&p->p_list);   // lo tolgo da qualsiasi lista
        if (p == currentActiveProc) {
            //currentActiveProc = NULL;
            ret = TRUE;
        }
        --activeProc;
    } else {
        --blockedProc;
        outBlocked(p);
    }
    
    outChild(p); // tolgo p come figlio così va avanti
    freePcb(p);
    return ret;
}


pcb_PTR findPcb(int pid) {
    
    struct list_head *pos;
    pcb_PTR p;
    if (pid == currentActiveProc->p_pid)
        return currentActiveProc;

    list_for_each(pos, &queueHighProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    
    list_for_each(pos, &queueLowProc) {
        if((p = container_of(pos, pcb_t, p_list))->p_pid == pid)
            return p;
    }
    p = isPcbBlocked(pid);
    return p;
}


void updateCurrProcTime() {
    cpu_t now;
    STCK(now);
    currentActiveProc->p_time += now - startTime;
    STCK(startTime);
}


/* SYSCALL */


int createProcess(state_t *a1, int a2, support_t *a3) {
    pcb_PTR p = allocPcb();
    
    if(p == NULL)
        return NOPROC;
    else {
        insertChild(currentActiveProc, p);
        copyState(a1, &p->p_s);
        insert_ready_queue(a2, p);
        if(a3 != NULL || a3 != 0)
            p->p_supportStruct = a3;
        else
            p->p_supportStruct = NULL;
        return p->p_pid;
    }
}


/* Ricerca il processo da terminare ed invoca la funzione che lo termina */
int terminateProcess(int pid) {
    int blocking_callerProc = FALSE;
    pcb_PTR p;
    if (pid == 0) {
        // se il pid e' 0, allora termino il processo corrente
        blocking_callerProc = TRUE;
        term_proc_and_child(currentActiveProc);
    } else {
        p = findPcb(pid);
        blocking_callerProc = term_proc_and_child(p);
    }
    return blocking_callerProc;
}


/* Porta il processo attualmente attivo in stato "Blocked" */
int passeren(int *semaddr) {
    if (*semaddr > 0)
        --(*semaddr);
    else {
        insertBlocked(semaddr, currentActiveProc);
        --activeProc;
        ++blockedProc;
        //klog_print("\n\nP: Passeren eseguita su processo: ");
        //klog_print_dec(currentActiveProc->p_pid);
        return TRUE;
    }
    return FALSE;
}


/* Porta il primo processo disponibile di un semaforo dallo stato "Blocked" in "Ready" */
void verhogen(int *semaddr) {
    pcb_PTR pid = removeBlocked(semaddr);

    if (pid == NULL){
        ++(*semaddr);
        //klog_print("\n\nV: non ho trovato nulla da mettere in Ready");
    }
    else {
        //Proc rimosso dal semaforo, lo inserisco nella lista dei proc ready
        --blockedProc;
        insert_ready_queue(pid->p_prio, pid);
        //klog_print("\n\nV: ho messo in ready state il processo numero ");
        //klog_print_dec(pid->p_pid);
    }
    //klog_print("\n\nVerhogen eseguita con successo..."); 
}


void doIOdevice(int *cmdAddr, int cmdValue) {
    /*
    int deviceNumber;
    int is_terminal = 0; //Se 1 è terminal Writing, se 2 è terminal Reading, se 0 other devices.
    devreg_t callingDevice;
    */
    int *devSemaphore; //Indirizzo del semaforo 
    int interruptLine;
    devregarea_t *deviceRegs = (devregarea_t*) RAMBASEADDR;

    for (int i = 0; i < 8; i++){
        if (& (deviceRegs->devreg[4][i].term.transm_command) == (memaddr*) cmdAddr) { //Terminal Devices Writing
            devSemaphore = &semTerminalDeviceWriting[i];
            //returnStatus = deviceRegs->devreg[4][i].term.transm_status;
            interruptLine = 7;
            klog_print("\n\ndoio: terminale di scrittura numero -> ");
            klog_print_dec(i);
            break;
        }
        else if (& (deviceRegs->devreg[4][i].term.recv_command) == (memaddr*) cmdAddr) { //Terminal Devices Reading
            devSemaphore = &semTerminalDeviceReading[i];
            //returnStatus = deviceRegs->devreg[4][i].term.recv_status;
            interruptLine = 7;
            klog_print("\n\ndoio: terminale di lettura numero -> ");
            klog_print_dec(i);
            break;
        }
        for(int j = 0; j < 4; j++){
            if (& (deviceRegs->devreg[j][i].dtp.command) == (memaddr*) cmdAddr ){
                //returnStatus = deviceRegs->devreg[j][i].dtp.status;
                if (j == 0)      { devSemaphore = &semDiskDevice[i];      interruptLine = j; }
                else if (j == 1) { devSemaphore = &semFlashDevice[i];     interruptLine = j; }  
                else if (j == 2) { devSemaphore = &semNetworkDevice[i];   interruptLine = j; }
                else             { devSemaphore = &semPrinterDevice[i];   interruptLine = j; }
            }
            break;
        }
    }



    //Eseguo la P del processo attualmente in esecuzione.
    passeren(devSemaphore);

    klog_print("\n\ndoio: semaforo syscall -> ");
    klog_print_hex((memaddr *)devSemaphore);

    //setSTATUS(IEPON);
    currentActiveProc->p_s.status |= STATUS_IM(interruptLine); 


    // Eseguo il comando richiesto.
    *cmdAddr = cmdValue;

    klog_print("\n\ndoio: eseguita.");
    //Ritorno lo stato del dispositivo che ha eseguit I/O
    //return returnStatus;
}


void getCpuTime(state_t *excState) {
    updateCurrProcTime();
    excState->reg_v0 = currentActiveProc->p_time;
}


void waitForClock() {
    passeren(&semIntervalTimer);
    insertBlocked(&semIntervalTimer, currentActiveProc);
    --activeProc;
    ++blockedProc;
}


support_t* getSupportData() {
    return (currentActiveProc->p_supportStruct != NULL) ? currentActiveProc->p_supportStruct : NULL;
}


void getIDprocess(state_t *callerProcess, int parent) {
    if(parent==0)
        (*callerProcess).reg_v0 = currentActiveProc->p_pid;
    else
        (*callerProcess).reg_v0 = currentActiveProc->p_parent->p_pid;
}


// inserisce il processo chiamante al termine della coda della rispettiva coda dei processi
void yield() {
    list_del(&currentActiveProc->p_list);

    if(currentActiveProc->p_prio == PROCESS_PRIO_LOW)
        insert_ready_queue(PROCESS_PRIO_LOW, currentActiveProc); 
    else                          
        insert_ready_queue(PROCESS_PRIO_HIGH,currentActiveProc);
}