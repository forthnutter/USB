/*
	* pout  -- By: Brett Carroll
	* Read data from the Parallel Port
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/io.h>

int main(int argc, char **argv)
{                    
	int in_data;
	int PORT;
	
	if (argc!=2) 
      printf("USAGE: pin [PORT]\n ie., pin 0x378\n"), exit(1); 
  
	sscanf(argv[1],"%i",&PORT); // Get PORT value (ie. 0x378, 0x379, 0x37a)

	if (ioperm(PORT,1,1))
		fprintf(stderr, "ERROR: Can't gain access to port %x\n", PORT), exit(1);

	in_data = inb(PORT);
	printf("%d\n",in_data);

return(in_data);
}                                            

