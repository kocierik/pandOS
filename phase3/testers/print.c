/* Function to call print parameterized output to a terminal device */

#include "/usr/local/include/umps3/umps/libumps.h"

#include "h/tconst.h"


void print(int device, char *str) {

	char *s = "Bad device write status\n";
	int leng, status;

	for (leng = 0; str[leng] != '\0'; leng++);
	
	status = SYSCALL (device, (int)str, leng, 0);
	
	if (status < 0) {
		status = SYSCALL (device, (int)s, 26, 0);
		SYSCALL (TERMINATE, 0, 0, 0);
	}
}
