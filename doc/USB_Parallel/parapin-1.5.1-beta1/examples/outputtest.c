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
  int pin_sequence[] = { 1, 2, 3, 4, 5, 6, 7, 8, 17, 14, 16, 9, -1};
  int i;
  int prev = 0;
  char buf[240];

  if (pin_init_user(LPT1) < 0)
    exit(0);

  pin_output_mode(LP_DATA_PINS | LP_SWITCHABLE_PINS);

  i = -1;
  while (1) {
    if (pin_sequence[++i] == -1)
      i = 0;
    printf("setting pin %d\n", pin_sequence[i]);
    set_pin(LP_PIN[pin_sequence[i]] | prev);
    printf("Hit return...\n");
    fgets(buf, 5, stdin);
    printf("clearing pin %d\n", pin_sequence[i]);
    clear_pin(LP_PIN[pin_sequence[i]] | prev);
    /*    prev = LP_PIN[pin_sequence[i]]; */
  } while (0);

}
