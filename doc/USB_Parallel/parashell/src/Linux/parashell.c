/*
	* parashell  -- Brett Carroll @ 2002
	* Write data to the parallel port as recieved from the cmdline.
	* Usage: parashell [PORT] [DATA]
	* DATA can be decimal (0...255) or hexadecimal (0x00...0xff)
	* snippets taken from Tomi Engdahl's (tomi.engdahl@hut.fi)
	* Simple parallel port output control program for Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/io.h>

main(int argc, char **argv)
{                    
  int par_data;
  int PORT;

  if (argc!=3) 
      printf("USAGE: parashell [PORT] [DATA]\n ie., parashell 0x378 128\n"), exit(1); 
  
  sscanf(argv[1],"%i",&PORT); // Get PORT value (ie. 0x378)

  if (sscanf(argv[2],"%i",&par_data)!=1)
      fprintf(stderr, "ERROR: DATA entered is not a number.\n"), exit(1);

  if ((par_data<0) || (par_data>255))
      fprintf(stderr, "ERROR: DATA  must be between 0 and 255\nUSAGE: parashell [PORT] [DATA]\n"), exit(1); 	

  if (ioperm(PORT,1,1)) 
      fprintf(stderr, "ERROR: Can't gain access to port %x\n", PORT), exit(1);


  outb((unsigned char)par_data, PORT); 	// Output Data to the Parallel Port
//  inb(PORT);	// Input Parallel Port Data
}                                            
