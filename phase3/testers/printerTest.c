/* Does nothing but outputs to the printer and terminates */

#include "/usr/local/include/umps3/umps/libumps.h"

#include "h/tconst.h"
#include "h/print.h"

void main() {
	print(WRITETERMINAL, "printTest is ok\n");
	
	print(WRITEPRINTER, "printTest is ok\n");
	
	SYSCALL(TERMINATE, 0, 0, 0);
}
