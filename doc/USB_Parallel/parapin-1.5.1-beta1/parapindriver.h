/* parapindriver.h -- header file for the parapindriver module.
*/

/*  Copyright (c) 2003, Al Hooton  <al@hootons.org>

    This file is part of the program "parapindriver".

    Parapindriver is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License (LGPL) as published by the Free Software Foundation.

    Parapindriver is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with parapindriver; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For futher information on the parapin package, please refer to the
    project information hosted on Sourceforge --
    
    http://parapin.sourceforge.net/

*/

#ifndef _PARAPINDRIVER_H_
#define _PARAPINDRIVER_H_


/* Bring in parapin.h so users of this driver get the same LP_PINxx 
   definitions that kparapin is expecting.  We just run those
   values through parapindriver with no change.  */
#include "parapin.h"


/* Define our ioctl commands.  We choose the magic number 0x96 */
#define PPDRV_IOC_MAGIC (0x96)

#define PPDRV_IOC_PINMODE_OUT       _IOW(PPDRV_IOC_MAGIC, 0, int)
#define PPDRV_IOC_PINMODE_IN        _IOW(PPDRV_IOC_MAGIC, 1, int)
#define PPDRV_IOC_PINSET            _IOW(PPDRV_IOC_MAGIC, 2, int)
#define PPDRV_IOC_PINCLEAR          _IOW(PPDRV_IOC_MAGIC, 3, int)
#define PPDRV_IOC_PINGET            _IOWR(PPDRV_IOC_MAGIC, 4, int)


#endif /* _PARAPINDRIVER_H_ */
