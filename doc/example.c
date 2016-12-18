#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <string.h>

void processMessage(const uint8_t*);


/*----------------------------------------------------------------------*/
int main(int argc, char*argv[])
{
  int res                      = 0;  /* return codes from libusb functions */
  libusb_device_handle* handle = 0;  /* handle for USB device */
  int kernelDriverDetached     = 0;  /* Set to 1 if kernel driver detached */
  int numBytes                 = 0;  /* Actual bytes transferred. */
  uint8_t buffer[64];                /* 64 byte transfer buffer */

  /* Initialise libusb. */
  res = libusb_init(0);
  if (res != 0)
  {
    fprintf(stderr, "Error initialising libusb.\n");
    return 1;
  }

  /* Get the first device with the matching Vendor ID and Product ID. If
   * intending to allow multiple demo boards to be connected at once, you
   * will need to use libusb_get_device_list() instead. Refer to the libusb
   * documentation for details. */
  handle = libusb_open_device_with_vid_pid(0, 0x04d8, 0x0070);
  if (!handle)
  {
    fprintf(stderr, "Unable to open device.\n");
    return 1;
  }

  /* Check whether a kernel driver is attached to interface #0. If so, we'll 
   * need to detach it.
   */
  if (libusb_kernel_driver_active(handle, 0))
  {
    res = libusb_detach_kernel_driver(handle, 0);
    if (res == 0)
    {
      kernelDriverDetached = 1;
    }
    else
    {
      fprintf(stderr, "Error detaching kernel driver.\n");
      return 1;
    }
  }

  /* Claim interface #0. */
  res = libusb_claim_interface(handle, 0);
  if (res != 0)
  {
    fprintf(stderr, "Error claiming interface.\n");
    return 1;
  }


  /* We can now send and receive messages. For example, set normal mode. */
  memset(buffer, 0, 64);
  buffer[58] = 0;      /* CANCTRL = Normal mode. */
  buffer[60] = 0x02;   /* SPI command = Write.   */
  buffer[61] = 0x0f;   /* Register = CANCTRL     */
  buffer[62] = 0;      /* Data = 0 (normal mode) */
  
  /* Send the message to endpoint 1 with a 100ms timeout. */
  res = libusb_interrupt_transfer(handle, 1, buffer, 64, &numBytes, 100);
  if (res == 0)
  {
    printf("%d bytes transmitted successfully.\n", numBytes);
  }
  else
  {
    fprintf(stderr, "Error sending message to device.\n");
  }


  /* Listen for a message. Note that for a normal application you'll need 
   * to use asynchronous mode because we can't predict when messages will be
   * available. This involves setting up a callback function to handle incoming
   * messages - refer to libusb documentation. */

  /* Wait up to 5 seconds for a message to arrive on endpoint 0x81. */
  res = libusb_interrupt_transfer(handle, 0x81, buffer, 64, &numBytes, 5000);
  if (0 == res)
  {
    if (numBytes == 64)
    {
      processMessage(buffer);
    }
    else
    {
      printf("Received %d bytes, expected 64.\n", numBytes);
    }
  }
  else
  {
    fprintf(stderr, "Error receiving message.\n");
  }

  /* Release interface #0. */
  res = libusb_release_interface(handle, 0);
  if (0 != res)
  {
    fprintf(stderr, "Error releasing interface.\n");
  }

  /* If we detached a kernel driver from interface #0 earlier, we'll now 
   * need to attach it again.  */
  if (kernelDriverDetached)
  {
    libusb_attach_kernel_driver(handle, 0);
  }

  /* Shutdown libusb. */
  libusb_exit(0);

  return 0;
}


/*----------------------------------------------------------------------*/
void processMessage(const uint8_t* buffer)
{
  unsigned index = 0;

  /* Most significant bit set indicates a CAN message is present. */
  while(buffer[index] & 0x80)
  {
    unsigned extendedID = buffer[index] & 0x20;
    unsigned rtr        = buffer[index] & 0x10;
    unsigned dataLength = buffer[index] & 0x0f;
    unsigned canID      = 0;

    ++index;

    if (extendedID)  /* 29 bit identifier */
    {
      canID = buffer[index] << 21;
      ++index;
      canID |= (((buffer[index] & 0xe0 >> 5) | 
            (buffer[index] & 0x03)) << 16);
      ++index;
      canID |= (buffer[index] << 8);
      ++index;
      canID |= (buffer[index]);
      ++index;
    }
    else /* standard 11 bit identifier */
    {
      canID      = buffer[index] << 3;
      ++index;
      canID |= ((buffer[index] >> 1) & 7);
      ++index;
    }

    printf("CAN ID: 0x%x [%s] ", canID,
        extendedID ? "extended" : "standard");

    if (rtr)
    {
      printf("RTR\n");
    }
    else
    {
      unsigned i = 0;
      for (i = 0; i < dataLength; ++i)
      {
        printf("0x%02x ", buffer[index]);
        ++index;
      }
      printf("\n");
    }
  }

  printf("CAN Status:      0x%02x\n", buffer[57]);
  printf("Transmit Errors: %u\n", buffer[55]);
  printf("Receive Errors:  %u\n", buffer[56]);

  /* If the command was read, we have received the result. */
  if (buffer[60] == 0x03)
  {
    printf("Read from register 0x%02x returned 0x%02x\n",
        buffer[61], buffer[62]);
  }
}

