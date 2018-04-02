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
    project information hosted on Sourceforge:
    
    http://sourceforge.net/projects/parapin/


    $Id: parapin-linux.h,v 1.17 2007/12/16 17:38:17 ahooton Exp $

*/
 

/*
 * System-specific include files for Linux
 */


/* Enable the use of KERNEL_VERSION(). */
#include <linux/version.h>

/* Warn about 2.4 support being deprecated and vanishing in the next release */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#warning Parapin support for kernels older than 2.6.0 is deprecated and will be removed in the near future.  If you must use older kernels please plan to stay on old versions of parapin.  Seriously consider migrating to 2.6.x kernels.
#endif

/* ioport.h is needed for both userspace and kernel/module personalities,
   but only for kernels before 2.6 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/ioport.h>
#endif

/* Stuff needed only when building for kernel contexts. */
#ifdef __KERNEL__

/* The Makefile determined if config.h exists or not, act accordingly */
#ifdef CONFIG_H
//#include <linux/config.h>
#endif

#include <linux/kernel.h>

/* module.h is needed if this will be a kernel modules as opposed to
   being linked-in to the kernel. */
#ifdef MODULE
#include <linux/module.h>
#endif /* MODULE */

#include <linux/stddef.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/delay.h>

#include <linux/parport.h>
#include <linux/parport_pc.h>
#include <linux/interrupt.h>

#define printf printk


/* parallel port we found and device we registered */
struct parport *parapin_port = NULL;
struct pardevice *parapin_dev = NULL;

/* was the system configured to use the interrupt before we got here? */
static int old_irq_mode = 0;
static int have_irq_handler = 0;

#else /* __KERNEL__ */

/* Includes needed when building as the userspace personality. */
#include <stdio.h>
#include <sys/io.h>

#endif /* __KERNEL__ */


/***************** Linux-Specific inb() and outb() *************************/

/* base address of parallel port I/O */
static int lp_base;

#define INB(register_number) inb(lp_base + register_number)
#define OUTB(value, register_number) outb(value, lp_base + register_number)

/**************** Linux-specific Initialization Functions *******************/


#ifdef __KERNEL__

/* kernel-mode parallel port initialization.  the 'lpt' argument is
 * the LP number of the port that you want.  this code will
 * EXCLUSIVELY claim the parallel port; i.e., so that other devices
 * connected to the same parallel port will not be able to use it
 * until you call the corresponding pin_release().  */

int pin_init_kernel(int lpt, void (*irq_func)(int, void *, struct pt_regs *))
{
  /* Get the enumeration number for this port */
  parapin_port = parport_find_number(lpt);


  if (parapin_port == NULL) {
    printk("parapin: init failed, parallel port lp%d not found\n", lpt);
    return -ENODEV;
  }


  /* NOTE: For a brief time sometime in the middle of the 2.6.x kernel
     series, the definition of parport_register_device() was changed
     briefly such that the 5th argument (the irq handler) did not have
     the "struct pt_regs() *" argument explicitly declared, which was
     an error.  It appears that at least one Debian distro went out to
     the world with this change in place.  In this case gcc will emit a
     warning about incompatible arguments being passed, but everything will
     work correctly.  If you get this warning on the 5th argument in the
     following call, and it bothers you, take a look at your
     include/linux/parport.h kernel header file to see if it has the
     incorrect function prototype for parport_register_device().   */

  /* Register the device on the port for exclusive access */
  parapin_dev = parport_register_device(parapin_port,
					"parapin", /* name for debugging */
					NULL, /* preemption callback */
					NULL, /* wakeup callback */
					irq_func, /* interrupt callback */
					PARPORT_DEV_EXCL, /* flags */
					(void *)NULL); /* user data */

  if (parapin_dev == NULL)
    return -ENODEV;

  /* ok - all systems go.  claim the parallel port. */
  parport_claim(parapin_dev);

  /* remember the LP base of our parallel port */
  lp_base = parapin_port->base;

  /* put us into bidir mode if we have an ECR */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
  if (parapin_port->modes & PARPORT_MODE_PCECR)
    parport_pc_write_econtrol(parapin_port, 0x20);
#else
  {
    struct parport_pc_private *priv = parapin_port->private_data;
    if (priv->ecr)
      outb(0x20, ECONTROL(parapin_port));
  }
#endif

  /* initialize the state of the registers */
  pin_init_internal();

  /* remember the current state of the interrupt enable flag */
  old_irq_mode = pin_is_set(LP_IRQ_MODE);
  have_irq_handler = (irq_func != NULL);

  /* disable interrupts */
  pin_disable_irq();

  /* tell the user what's happening */
  printk("parapin: claiming %s at 0x%lx, irq %d\n", parapin_port->name,
         parapin_port->base, parapin_port->irq);

  return 0;
}


/* this must be called by kernel programs when you're done using the
   parallel port.  it releases the port to be used by other apps. */
void pin_release()
{
  /* restore interrupts to their former state */
  change_pin(LP_IRQ_MODE, old_irq_mode ? LP_SET : LP_CLEAR);

  /* release and unregister the parallel port */
  parport_release(parapin_dev);
  parport_unregister_device(parapin_dev);


  parport_put_port(parapin_port);



}


/* are interrupts available? */
int pin_have_irq()
{
  return (parapin_port && have_irq_handler && (parapin_port->irq >= 0));
}

/* turn interrupts on */
void pin_enable_irq()
{
  if (pin_have_irq()) {
    set_pin(LP_IRQ_MODE);
    udelay(10); /* wait for the spurious interrupt (if any) to pass */
    enable_irq(parapin_port->irq);
  } else {
    printk("parapin: trying to enable interrupts without proper prereqs");
  }
}

/* turn interrupts off */
void pin_disable_irq()
{
  if (parapin_port && parapin_port->irq >= 0)
    disable_irq(parapin_port->irq);
  clear_pin(LP_IRQ_MODE);
}


#else  /* user-space-only functions */

/* user-land initialization */
int pin_init_user(int base)
{
  lp_base = base;

  /* get write permission to the I/O port */
  if (ioperm(lp_base, 3, 1) < 0) {
    perror("can't get IO permissions!");
    return -1;
  }

  pin_init_internal();
  return 0;
}

#endif /* __KERNEL__ */


/* If this is 2.6 or later, and we're building kparapin, we need to
   explicitly export symbols that are to be used by other modules. */
#ifdef __KERNEL__
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
EXPORT_SYMBOL(set_pin);
EXPORT_SYMBOL(clear_pin);
EXPORT_SYMBOL(change_pin);
EXPORT_SYMBOL(pin_is_set);
EXPORT_SYMBOL(pin_input_mode);
EXPORT_SYMBOL(pin_output_mode);
EXPORT_SYMBOL(pin_mode);
EXPORT_SYMBOL(pin_release);
EXPORT_SYMBOL(pin_init_kernel);
EXPORT_SYMBOL(pin_have_irq);
EXPORT_SYMBOL(pin_enable_irq);
EXPORT_SYMBOL(pin_disable_irq);
#endif /* LINUX_VERSION_CODE */
#endif /* __KERNEL__ */



