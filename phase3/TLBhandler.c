

void general_execption_hendler(){

}


void sup_syscall_handler() {

    switch (1) //da cambiare
    {
    case GET_TOD:
        get_tod();
        break;
    case TERMINATE:
        terminate();
        break;
    case WRITEPRINTER:
        write_printer();
        break;
    case WRITETERMINAL:
        write_terminal();
        break;
    case READTERMINAL:
        read_terminal();
        break;
    
    default:
        break;
    }
}