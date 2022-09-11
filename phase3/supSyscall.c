#include "headers/supSyscall.h"

extern int semPrinterDevice[8];
extern int semTerminalDeviceWriting[8];
extern int semTerminalDeviceReading[8];

void get_tod(support_t *s)
{
    cpu_t tod;
    STCK(tod);
    s->sup_exceptState[GENERALEXCEPT].reg_v0 = tod;
}

void terminate(support_t *s)
{
    //SYSCALL(VERHOGEN, (int)&swap_pool_sem, 0, 0); // release swap pool semaphore, serve? da controllare
    for (int i = 0; i < POOLSIZE; ++i)
        if (swap_pool_table[i].sw_asid == s->sup_asid)
            swap_pool_table[i].sw_asid = NOPROC; // unmark swap pool table
    free_sd(s);                                  // free support descriptor
    SYSCALL(VERHOGEN, (int)&master_sem, 0, 0);   // realise master sem
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void write_to_printer(support_t *s)
{
    syscall_write(s, IL_PRINTER);
}

void write_to_terminal(support_t *s)
{
    syscall_write(s, IL_TERMINAL);
}

int *get_dev_sem(int i, int IL_X)
{
    int *sems;
    switch (IL_X)
    {
    case IL_PRINTER:
        sems = semPrinterDevice;
        break;

    case IL_TERMINAL:
        sems = semTerminalDeviceWriting;
        break;

    default:
        PANIC();
        break;
    }

    return &sems[i];
}

// da controllare
void syscall_write(support_t *s, int IL_X)
{
    unsigned int status;
    char *msg = (char *)s->sup_exceptState[GENERALEXCEPT].reg_a1;
    int len = s->sup_exceptState[GENERALEXCEPT].reg_a2;
    int arg1, arg2, terminal = IL_X == IL_TERMINAL, index = s->sup_asid - 1;

    int *sem = get_dev_sem(index, IL_X);
    void *device = (void *)DEV_REG_ADDR(IL_X, index);

    if (len < 0 || len > 100 || (memaddr)msg < KUSEG)
        SYSCALL(TERMINATE, 0, 0, 0);

    SYSCALL(PASSEREN, (int)&sem, 0, 0);

    for (int i = 0; i < len; i++)
    {
        setSTATUS(getSTATUS() & (!IECON)); // disable interrupts

        arg1 = (int)&((dtpreg_t *)device)->command;
        arg2 = PRINTCHR;

        if (terminal)
        {
            arg1 = (int)&((termreg_t *)device)->transm_command;
            arg2 |= ((unsigned int)msg[i] << 8);
        }
        else
        {
            ((dtpreg_t *)device)->data0 = msg[i];
            arg2 |= 0;
        }

        setSTATUS(getSTATUS() | IECON); // enable interrupts

        status = SYSCALL(DOIO, arg1, (int)arg2, 0);

        if ((status & (terminal ? TERMSTATMASK : -1)) != (terminal ? RECVD : DEV_STATUS_READY))
        {
            len = -(status & (terminal ? TERMSTATMASK : -1));
            break;
        }
    }

    SYSCALL(VERHOGEN, (int)&sem, 0, 0);

    s->sup_exceptState[GENERALEXCEPT].reg_v0 = len;
}

// guardato dal progetto fede 
void read_from_terminal(char *virtualAddress)
{ 
    int ret = 0;

    support_t* sup = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0); 

    int terminalASID = sup->sup_asid - 1; // legge da 1 a 8 (ASID), ma i devices vanno da 0 a 7
    int terminalSemaphoreIndex = (TERMINT - 3) * 8 + 2*terminalASID;  
    
    if((unsigned int)virtualAddress < KUSEG){    /* indirizzo out memoria virtuale / o lunghezza richiesta 0 */
    }

    devreg_t *terminalDEVREG = (devreg_t*)(START_DEVREG + ((TERMINT - 3) * 0x80) + (terminalASID * 0x10));

    char recv_char = 0;                                                           
    while(recv_char != '\n'){   /* lettura dell'input */      
      int status = SYSCALL(DOIO, (unsigned int)&(terminalDEVREG->term.recv_command), RECEIVECHAR, 0);  
      if((status & 0xFF) == CHARRECV){ // NO errore
        *virtualAddress = status >> BYTELENGTH;
        recv_char = status >> BYTELENGTH;
        virtualAddress++;
        ret++;
      } else {
        ret = -status; 
        break;
      }
    }
    // VERHOGEN DA FARE corretto?
     SYSCALL(VERHOGEN, (int)&semTerminalDeviceReading[terminalSemaphoreIndex], 0, 0);  
  sup->sup_exceptState[GENERALEXCEPT].reg_v0 = ret;
}
