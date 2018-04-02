/*  This file is part of the package "parapin".

    The parapin package is free software; you can redistribute it
    and/or modify it under the terms of the GNU Library General Public
    License (LGPL) as published by the Free Software Foundation.

    The parapin package is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with parapindriver; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    For futher information on the parapin package, please refer to the
    project information hosted on Sourceforge --
    
    http://sourceforge.net/projects/parapin/

*/

/* parapindriver.c -- source file for the parapindriver kernel module.
   This simple driver is intended to provide a basic userspace
   interface to the execellent parapin kernel module initially created
   by Jeremy Elson.  While parapin can be built as either a userspace
   library or as a module for kernelspace, the kernelspace flavor
   provides advantages in access control, but it can not be referenced
   directly from userspace.  It requires another driver to wrap around
   it, register to a device, and handle some system calls that
   eventually get translated in the driver to parapin calls.  This is
   the purpose of parapindriver.

   Note that this driver does not support or expose the interrupt handling
   capability of kparapin.
*/
 
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>

/* Some distros define _IOW by default, and others only define it if
 * sys/ioctl.h is brougt in explicitly.
 */
#ifndef _IOW
#include <sys/ioctl.h>
#endif

#include "parapin.h"
#include "parapindriver.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* Module documentation for those who find it */
/* Documentation for those who find it */
MODULE_LICENSE("LGPL");
MODULE_AUTHOR("Al Hooton <al@hootons.org>");
MODULE_DESCRIPTION("Wrapper driver for kparapin module (http://parapin.sourceforge.net/)");
#endif /* LINUX_VERSION_CODE */

/* Prototypes for our private functions.  We need these to appear before
   the following declaration of fops. */
static int ppdrv_open(struct inode *, struct file *);
static int ppdrv_release(struct inode *, struct file *);
static int ppdrv_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

/* System calls we're handling */
struct file_operations fops = {
  .open = ppdrv_open,
  .release = ppdrv_release,
  .ioctl = ppdrv_ioctl
};

/* Static vars for this file only */
static int major;

/* One input parameter, supposedly the name of our device in /dev, passed
   in to us at boot time by the script that loads us.  It gets the majornum
   we are assigned from /proc, and then creates the /dev entry.
   If it doesn't come in we use a default.  We don't actually
   need this, the only reason  we want it is to allow us to register under
   the same name, so it is potentially easier for humans to keep track of
   what's happening.  The loading script will also use this string to
   find our majornum in /proc/devices.  */
static char devname_default[] = "parapindriver";
static char *devname = devname_default;

/* If MODULE_PARAM is not defined, we're building with the "new" kernel
 * module parameter passing model, so we need to use module_param()
 */
#ifdef MODULE_PARM
MODULE_PARM(devname, "s");
#else
module_param(devname, charp, 0);
#endif
MODULE_PARM_DESC(devname, "Name of the device in /dev");


/* init_module - called when module is loaded in to the kernel */
int insert_module(void)
{
  /* Attempt to get dynamically-assigned major number for our device */
  major = register_chrdev(0, devname, &fops);
  
  if (major < 0) {
    /* We didn't get a major, we're outta here */
    printk(KERN_CRIT "parapindriver: register_chrdev failed!, error: %d\n",
	   major);
    return(major);
  }

  printk(KERN_INFO "parapindriver: assigned major: %d\n", major);

  return(0);
}
module_init(insert_module);



/* cleanup_module - called when module is unloaded from kernel */
void remove_module(void)
{
  int ret;

  /* First, attempt to unregister our device */
  ret = unregister_chrdev(major, devname);

  if (ret < 0) {
    /* The unregister failed, whine */
    printk(KERN_CRIT "parapindriver: unregister_chrdev failed!, error: %d\n", ret);
  }
}
module_exit(remove_module);



/* ppdrv_open - handle open calls */
int ppdrv_open(struct inode *inodePtr, struct file *filePtr)
{
  int ret;


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
  /* Make sure we're not already open */
  if (MOD_IN_USE > 0) {
    return(-EBUSY);
  }
#endif

  /* Initialize kparapin */
  ret = pin_init_kernel(0, (void *)NULL);

  if (ret !=0) {
    /* kparapin didn't initialize, return the reason why */
    printk(KERN_ERR "parapindriver: failed to initialize kparapin, error: %d\n", ret);

    return(ret);
  }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
  /* Increment the use count for this module */
  MOD_INC_USE_COUNT;
#endif

  return(0);
}



/* ppdrv_release - handle release calls */
int ppdrv_release(struct inode *inodePtr, struct file *filePtr)
{

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
  /* Make sure we're open */
  if (MOD_IN_USE == 0) {
    return(-EPERM);
  }
#endif

  /* Shut down kparapin */
  pin_release();

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
  /* Decrement the use count for this module */
  MOD_DEC_USE_COUNT;
#endif

  return(0);
}



/* ppdrv_ioctl - handle ioctl calls */
int ppdrv_ioctl(struct inode *inodePtr, struct file *filePtr, unsigned int cmd, unsigned long val)
{
  int retval = 0;

  /* Handle our ioctly commands */
  switch (cmd) {

  case PPDRV_IOC_PINMODE_OUT:
    /* Declare one or more pins to be output pins */
    pin_output_mode((int)val);
    break;

  case PPDRV_IOC_PINMODE_IN:
    /* Declare one or more pins to be input pins */
    pin_input_mode((int)val);
    break;

  case PPDRV_IOC_PINSET:
    /* Set one or more pins high (1) */
    set_pin((int)val);
    break;

  case PPDRV_IOC_PINCLEAR:
    /* Set one or more pins low (0) */
    clear_pin((int)val);
    break;

  case PPDRV_IOC_PINGET:
    /* Get the values of one or more input pins, return through val */
    retval = pin_is_set((int)val);
    break;

  default:
    /* Invalid ioctl cmd for us, indicate so */
    retval = -ENOTTY;
    break;
  }

  return(retval);
}


