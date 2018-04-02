/*  This file is part of the package "parapin".

    The parapin package is free software; you can redistribute it
    and/or modify it under the terms of the GNU Library General Public
    License (LGPL) as published by the Free Software Foundation.

    The parapin package is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with parapin; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA


    For futher information on the parapin package, please refer to the
    project information hosted on Sourceforge --

    http://sourceforge.net/projects/parapin/

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parapin.h"

int main(int argc, char *argv[])
{
  int pin;
  if(argc < 2)
    {
      printf("USAGE: setlp0pin pin#\n");
      exit(1);
    }
  pin = atoi(argv[1]);
  if (pin_init_user(LPT1) < 0)
    exit(0);
  pin_output_mode(LP_DATA_PINS | LP_SWITCHABLE_PINS);

  if(pin < 0)  // clear requested
    {
      pin = pin * -1;
      clear_pin(LP_PIN[pin]);
      printf("pin# %d cleared\n", pin);
    }
  else
    {

      set_pin(LP_PIN[pin]);
      printf("pin# %d set.  Run \"setlp0pin -%d\" to clear pin# %d\n", pin, pin, pin);
    }
  return 0;
}
