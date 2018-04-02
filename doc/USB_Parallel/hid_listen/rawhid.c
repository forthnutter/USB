/* Raw HID I/O Routines
 * Copyright 2008, PJRC.COM, LLC
 * paul@pjrc.com
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */


// This code will someday be turned into "librawhid", an easy-to-use
// and truly cross platform library for accessing HID reports.  But
// there are many complexities not properly handled by this simple
// code that would be expected from a high quality library.  In
// particular, how report IDs are handled is not uniform on the 3
// platforms.  The mac code uses a single buffer which assumes no
// other functions can cause the "run loop" to process HID callbacks.
// The linux version doesn't extract usage and usage page from the
// report descriptor and just hardcodes a signature for the Teensy
// USB debug example.  Lacking from all platforms are functions to
// manage multiple devices and robust detection of device removal
// and attachment.  There are probably lots of other issues... this
// code has really only been used in 2 projects.  If you use it,
// please report bugs to paul@pjrc.com


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rawhid.h"

#ifdef OPERATING_SYSTEM
#undef OPERATING_SYSTEM
#endif


/*************************************************************************/
/**                                                                     **/
/**                             Linux                                   **/
/**                                                                     **/
/*************************************************************************/

#if defined(LINUX) || defined(__LINUX__) || #system(linux)
#define OPERATING_SYSTEM linux
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>


struct rawhid_struct {
	int fd;
	int name;
	int isok;
};


rawhid_t * rawhid_open_only1(int vid, int pid, int usage_page, int usage)
{
	struct rawhid_struct *hid;
	struct stat devstat;
	struct hidraw_devinfo info;
	struct hidraw_report_descriptor *desc;
	char buf[512];
	// TODO; inputs are ignored and hardcoded into this signature....
	const unsigned char signature[]={0x06,0x31,0xFF,0x09,0x74};
	int r, i, fd=-1, len, found=0;

	//printf("Searching for device using hidraw....\n");
	for (i=0; i<HIDRAW_MAX_DEVICES; i++) {
		if (fd > 0) close(fd);
		snprintf(buf, sizeof(buf), "/dev/hidraw%d", i);
		r = stat(buf, &devstat);
		if (r < 0) continue;
		//printf("device: %s\n", buf);
		fd = open(buf, O_RDWR);
		if (fd < 0) continue;
		//printf("  opened\n");
		r = ioctl(fd, HIDIOCGRAWINFO, &info);
		if (r < 0) continue;
		//printf("  vid=%04X, pid=%04X\n", info.vendor & 0xFFFF, info.product & 0xFFFF);
		r = ioctl(fd, HIDIOCGRDESCSIZE, &len);
		if (r < 0 || len < 1) continue;
		//printf("  len=%u\n", len);
		desc = (struct hidraw_report_descriptor *)buf;
		if (len > sizeof(buf)-sizeof(int)) len = sizeof(buf)-sizeof(int);
		desc->size = len;
		r = ioctl(fd, HIDIOCGRDESC, desc);
		if (r < 0) continue;
		if (len >= sizeof(signature) &&
		   memcmp(desc->value, signature, sizeof(signature)) == 0) {
			//printf("  Match\n");
			found = 1;
			break;
		}
	}
	if (!found) {
		if (fd > 0) close(fd);
		return NULL;
	}
	hid = (struct rawhid_struct *)malloc(sizeof(struct rawhid_struct));
	if (!hid) {
		close(fd);
		return NULL;
	}
	hid->fd = fd;
	return hid;
}

int rawhid_status(rawhid_t *hid)
{
	// TODO: how to check if device is still online?
	return -1;
}

int rawhid_read(rawhid_t *h, void *buf, int bufsize, int timeout_ms)
{
	struct rawhid_struct *hid;
	int num;

	hid = (struct rawhid_struct *)h;
	if (!hid || hid->fd < 0) return -1;

	while (1) {
		num = read(hid->fd, buf, bufsize);
		if (num < 0) {
			if (errno == EINTR || errno == EAGAIN) continue;
			if (errno == EIO) {
				return -1;
				printf("I/O Error\n");
			}
			printf("read error, r=%d, errno=%d\n", num, errno);
			return -1;
		}
		//printf("read %d bytes\n", num);
		return num;
	}
}

void rawhid_close(rawhid_t *h)
{
	struct rawhid_struct *hid;

	hid = (struct rawhid_struct *)h;
	if (!hid || hid->fd < 0) return;
	close(hid->fd);
	hid->fd = -1;
}

#if 0
struct rawhid_list_struct {
	int count;
	struct rawhid_list_entry {
		int name;
		int desc_length;
		const char *desc;
	} dev[HIDRAW_MAX_DEVICES];
};

rawhid_list_t * rawhid_list_open(int vid, int pid, int usage_page, int usage)
{
	struct rawhid_list_struct *list;
	struct stat devstat;
	struct hidraw_devinfo info;
	struct hidraw_report_descriptor *desc;
	char buf[512], *p;
	const unsigned char signature[]={0x06,0x31,0xFF,0x09,0x74};
	int r, i, fd=-1, len;

	list = (struct rawhid_list_struct *)malloc(sizeof(struct rawhid_list_struct));
	if (!list) return NULL;
	list->count = 0;

	printf("Searching for device using hidraw....\n");
	for (i=0; i<HIDRAW_MAX_DEVICES; i++) {
		if (fd > 0) close(fd);
		snprintf(buf, sizeof(buf), "/dev/hidraw%d", i);
		r = stat(buf, &devstat);
		if (r < 0) continue;
		printf("device: %s\n", buf);
		fd = open(buf, O_RDWR);
		if (fd < 0) continue;
		printf("  opened\n");
		r = ioctl(fd, HIDIOCGRAWINFO, &info);
		if (r < 0) continue;
		printf("  vid=%04X, pid=%04X\n", info.vendor & 0xFFFF, info.product & 0xFFFF);
		r = ioctl(fd, HIDIOCGRDESCSIZE, &len);
		if (r < 0 || len < 1) continue;
		printf("  len=%u\n", len);
		desc = (struct hidraw_report_descriptor *)buf;
		if (len > sizeof(buf)-sizeof(int)) len = sizeof(buf)-sizeof(int);
		desc->size = len;
		r = ioctl(fd, HIDIOCGRDESC, desc);
		if (r < 0) continue;
		if (len < sizeof(signature)) continue;
		// TODO: actual report parsing would be nice!
		if (memcmp(desc->value, signature, sizeof(signature)) != 0) continue;
		p = (char *)malloc(len);
		if (!p) continue;
		printf("  Match\n");
		memcpy(p, desc->value, len);
		list->dev[list->count].desc = p;
		list->dev[list->count].desc_length = len;
		list->dev[list->count].name = i;
		list->count++;
	}
	if (fd > 0) close(fd);
	return list;
}

int rawhid_list_count(rawhid_list_t *list)
{
	if (!list) return 0;
	return ((struct rawhid_list_struct *)list)->count;
}

void rawhid_list_close(rawhid_list_t *list)
{
	if (list) free(list);
}
#endif


#endif // linux


/*************************************************************************/
/**                                                                     **/
/**                             Mac OS X 10.5                           **/
/**                                                                     **/
/*************************************************************************/

#if (defined(DARWIN) || defined(__DARWIN__)) && !defined(OPERATING_SYSTEM)
#define OPERATING_SYSTEM darwin
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <unistd.h>

// http://developer.apple.com/technotes/tn2007/tn2187.html


struct rawhid_struct {
	IOHIDDeviceRef ref;
	int disconnected;
	uint8_t *buffer;
	int buffer_used;
	int buffer_report_id;
};


static void unplug_callback(void *hid, IOReturn ret, void *ref)
{
	// This callback can only be called when the "run loop" (managed by macos)
	// is run.  If the GUI is running it when idle, this will get called
	// automatically.  If not, the run loop needs to be run explicitly
	// before checking the result of this function.
	//printf("HID/macos: unplugged callback!\n");
	((struct rawhid_struct *)hid)->disconnected = 1;
}


static void input_callback(void *context, IOReturn result, void *sender, 
	IOHIDReportType type, uint32_t reportID, uint8_t *report,
	CFIndex reportLength)
{
	struct rawhid_struct *hid;

	//printf("input callback\n");
	if (!context) return;
	hid = (struct rawhid_struct *)context;
	hid->buffer_used = reportLength;
	hid->buffer_report_id = reportID;
	//printf("id = %d, reportLength = %d\n", reportID, (int)reportLength);
	//if (hid->ref == sender) printf("ref matches :-)\n");
	//if (report == hid->buffer) printf("buffer matches :)\n");
}


rawhid_t * rawhid_open_only1(int vid, int pid, int usage_page, int usage)
{
	IOHIDManagerRef hid_manager;
	CFMutableDictionaryRef dict;
	IOReturn ret;
	CFSetRef device_set;
	IOHIDDeviceRef device_list[256];
	uint8_t *buf;
	struct rawhid_struct *hid;
	int num_devices;

	// get access to the HID Manager
	hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
		printf("HID/macos: unable to access HID manager");
		return NULL;
	}
	// configure it to look for our type of device
	dict = IOServiceMatching(kIOHIDDeviceKey);
	if (dict == NULL) {
		printf("HID/macos: unable to create iokit dictionary");
		return NULL;
	}
	if (vid > 0) {
		CFDictionarySetValue(dict, CFSTR(kIOHIDVendorIDKey), 
			CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid));
	}
	if (pid > 0) {
		CFDictionarySetValue(dict, CFSTR(kIOHIDProductIDKey), 
			CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid));
	}
	if (usage_page > 0) {
		CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsagePageKey), 
			CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page));
	}
	if (usage > 0) {
		CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsageKey),
			CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage));
	}
	IOHIDManagerSetDeviceMatching(hid_manager, dict);

	// now open the HID manager
	ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
	if (ret != kIOReturnSuccess) {
		printf("HID/macos: Unable to open HID manager (IOHIDManagerOpen failed)");
		return NULL;
	}
	// get a list of devices that match our requirements
	device_set = IOHIDManagerCopyDevices(hid_manager);
	if (device_set == NULL) {
		//printf("HID/macos: no devices found\n");
		return NULL;
	}
	num_devices = (int)CFSetGetCount(device_set);
	//printf("number of devices found = %d\n", num_devices);
	if (num_devices < 1) {
		CFRelease(device_set);
		printf("HID/macos: no devices found, even though HID manager returned a set\n");
		return NULL;
	}
	if (num_devices > 256) {
		CFRelease(device_set);
		printf("HID/macos: too many devices, we get confused if more than 256!\n");
		return NULL;
	}
	CFSetGetValues(device_set, (const void **)&device_list);
	CFRelease(device_set);
	// open the first device in the list
	ret = IOHIDDeviceOpen(device_list[0], kIOHIDOptionsTypeNone);
	if (ret != kIOReturnSuccess) {
		printf("HID/macos: error opening device\n");
		return NULL;
	}
	// return this device
	hid = (struct rawhid_struct *)malloc(sizeof(struct rawhid_struct));
	buf = (uint8_t *)malloc(0x1000);
	if (hid == NULL || buf == NULL) {
		IOHIDDeviceRegisterRemovalCallback(device_list[0], NULL, NULL);
		IOHIDDeviceClose(device_list[0], kIOHIDOptionsTypeNone);
		printf("HID/macos: Unable to allocate memory\n");
		return NULL;
	}
	hid->ref = device_list[0];
	hid->disconnected = 0;
	hid->buffer = buf;
	hid->buffer_used = 0;

	// register a callback to receive input
	IOHIDDeviceRegisterInputReportCallback(hid->ref, hid->buffer, 0x1000,
		input_callback, hid);


	// register a callback to find out when it's unplugged
	IOHIDDeviceScheduleWithRunLoop(hid->ref, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDDeviceRegisterRemovalCallback(hid->ref, unplug_callback, hid);
	return hid;
}

int rawhid_status(rawhid_t *hid)
{
	if (!hid) return -1;
	// if a GUI is causing the run loop run, this will likely mess it up.  Just
	// comment it out and if the callback still gets called without this, then
	// there's no need to run the run loop here!
	while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;
	if (((struct rawhid_struct *)hid)->disconnected) {
		//printf("HID/macos: status: disconnected\n");
		return -1;
	}
	//printf("HID/macos: status: ok\n");
	return 0;
}

void rawhid_close(rawhid_t *hid)
{
	IOHIDDeviceRef ref;

	if (!hid) return;
	ref = ((struct rawhid_struct *)hid)->ref;
	IOHIDDeviceRegisterRemovalCallback(ref, NULL, NULL);
	IOHIDDeviceClose(ref, kIOHIDOptionsTypeNone);
	free(hid);
}

int rawhid_read(rawhid_t *h, void *buf, int bufsize, int timeout_ms)
{
	struct rawhid_struct *hid;
	int r, len;

	//printf("begin read\n");
	hid = (struct rawhid_struct *)h;
	if (!hid || hid->disconnected) return -1;
	while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) {
		if (hid->buffer_used) {
			len = hid->buffer_used;
			if (len > bufsize) len = bufsize;
			memcpy(buf, hid->buffer, len);
			hid->buffer_used = 0;
			return len;
		}
		if (hid->disconnected) {
			return -1;
		}
	}
	r = CFRunLoopRunInMode(kCFRunLoopDefaultMode, (double)timeout_ms / 1000.0, true);
	if (r == kCFRunLoopRunTimedOut) {
		//printf("read timeout\n");
		return 0;
	}
	if (hid->buffer_used) {
		len = hid->buffer_used;
		if (len > bufsize) len = bufsize;
		memcpy(buf, hid->buffer, len);
		hid->buffer_used = 0;
		return len;
	}
	if (hid->disconnected) return -1;
	return 0;
	//num = bufsize;
	//ret = IOHIDDeviceGetReport(ref, kIOHIDReportTypeInput, 0, buf, &num);
	//if (!ret) return -1;
	//return num;
}

int rawhid_write(rawhid_t *hid, const void *buf, int len, int timeout_ms)
{
	IOReturn ret;

	if (((struct rawhid_struct *)hid)->disconnected) return -1;
	ret = IOHIDDeviceSetReport(((struct rawhid_struct *)hid)->ref,
		kIOHIDReportTypeOutput, 0, buf, len);
	if (ret != kIOReturnSuccess) return -1;
	return 0;
}


#endif // Darwin - Mac OS X


/*************************************************************************/
/**                                                                     **/
/**                     Windows 2000/XP/Vista                           **/
/**                                                                     **/
/*************************************************************************/

#if (defined(WIN32) || defined(WINDOWS) || defined(__WINDOWS__)) && !defined(OPERATING_SYSTEM)
#define OPERATING_SYSTEM windows
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>

// http://msdn.microsoft.com/en-us/library/ms790932.aspx

struct rawhid_struct {
	HANDLE handle;
};


rawhid_t * rawhid_open_only1(int vid, int pid, int usage_page, int usage)
{
	GUID guid;
	HDEVINFO info;
	DWORD index=0, required_size;
	SP_DEVICE_INTERFACE_DATA iface;
	SP_DEVICE_INTERFACE_DETAIL_DATA *details;
	HIDD_ATTRIBUTES attrib;
	PHIDP_PREPARSED_DATA hid_data;
	HIDP_CAPS capabilities;
	struct rawhid_struct *hid;
	HANDLE h;
	BOOL ret;


	HidD_GetHidGuid(&guid);
	info = SetupDiGetClassDevs(&guid, NULL, NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (info == INVALID_HANDLE_VALUE) {
		printf("HID/win32: SetupDiGetClassDevs failed");
		return NULL;
	}
	for (index=0; ;index++) {
		iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
		if (!ret) {
			// end of list
			SetupDiDestroyDeviceInfoList(info);
			return NULL;
		}
		SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &required_size, NULL);
		details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(required_size);
		if (details == NULL) continue;
		memset(details, 0, required_size);
		details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details,
			required_size, NULL, NULL);
		if (!ret) {
			free(details);
			continue;
		}
		h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED, NULL);
		free(details);
		if (h == INVALID_HANDLE_VALUE) continue;
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		ret = HidD_GetAttributes(h, &attrib);
		if (!ret) {
			CloseHandle(h);
			continue;
		}
		//printf("HID/win32:   USB Device:\n");
		//printf("HID/win32:     vid =        0x%04X\n", (int)(attrib.VendorID));
		//printf("HID/win32:     pid =        0x%04X\n", (int)(attrib.ProductID));
		if (vid > 0 && vid != (int)(attrib.VendorID)) {
			CloseHandle(h);
			continue;
		}
		if (pid > 0 && pid != (int)(attrib.ProductID)) {
			CloseHandle(h);
			continue;
		}
		if (!HidD_GetPreparsedData(h, &hid_data)) {
			printf("HID/win32: HidD_GetPreparsedData failed\n");
			CloseHandle(h);
			continue;
		}
		if (!HidP_GetCaps(hid_data, &capabilities)) {
			printf("HID/win32: HidP_GetCaps failed\n");
			HidD_FreePreparsedData(hid_data);
			CloseHandle(h);
			continue;
		}
		//printf("HID/win32:     usage_page = 0x%04X\n", (int)(capabilities.UsagePage));
		//printf("HID/win32:     usage      = 0x%04X\n", (int)(capabilities.Usage));
		if (usage_page > 0 && usage_page != (int)(capabilities.UsagePage)) {
			HidD_FreePreparsedData(hid_data);
			CloseHandle(h);
			continue;
		}
		if (usage > 0 && usage != (int)(capabilities.Usage)) {
			HidD_FreePreparsedData(hid_data);
			CloseHandle(h);
			continue;
		}
		HidD_FreePreparsedData(hid_data);
		hid = (struct rawhid_struct *)malloc(sizeof(struct rawhid_struct));
		if (!hid) {
			CloseHandle(h);
			printf("HID/win32: Unable to get %d bytes", sizeof(struct rawhid_struct));
			continue;
		}
		hid->handle = h;
		return hid;
	}
}


int rawhid_status(rawhid_t *hid)
{
	PHIDP_PREPARSED_DATA hid_data;

	if (!hid) return -1;
	if (!HidD_GetPreparsedData(((struct rawhid_struct *)hid)->handle, &hid_data)) {
		printf("HID/win32: HidD_GetPreparsedData failed, device assumed disconnected\n");
		return -1;
	}
	printf("HID/win32: HidD_GetPreparsedData ok, device still online :-)\n");
	HidD_FreePreparsedData(hid_data);
	return 0;
}

void rawhid_close(rawhid_t *hid)
{
	if (!hid) return;
	CloseHandle(((struct rawhid_struct *)hid)->handle);
	free(hid);
}

int rawhid_read(rawhid_t *h, void *buf, int bufsize, int timeout_ms)
{
	DWORD num=0, result;
	BOOL ret;
	OVERLAPPED ov;
	struct rawhid_struct *hid;
	int r;

	hid = (struct rawhid_struct *)h;
	if (!hid) return -1;

	memset(&ov, 0, sizeof(OVERLAPPED));
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) return -1;

	ret = ReadFile(hid->handle, buf, bufsize, &num, &ov);
	if (ret) {
		//printf("HID/win32:   read success (immediate)\n");
		r = num;
	} else {
		if (GetLastError() == ERROR_IO_PENDING) {
			result = WaitForSingleObject(ov.hEvent, timeout_ms);
			if (result == WAIT_OBJECT_0) {
				if (GetOverlappedResult(hid->handle, &ov, &num, FALSE)) {
					//printf("HID/win32:   read success (delayed)\n");
					r = num;
				} else {
					//printf("HID/win32:   read failure (delayed)\n");
					r = -1;
				}
			} else {
				//printf("HID/win32:   read timeout, %lx\n", result);
				CancelIo(hid->handle);
				r = 0;
			}
		} else {
			//printf("HID/win32:   read error (immediate)\n");
			r = -1;
		}
	}
	CloseHandle(ov.hEvent);
	return r;
}


int rawhid_write(rawhid_t *h, const void *buf, int len, int timeout_ms)
{
	DWORD num=0;
	BOOL ret;
	OVERLAPPED ov;
	struct rawhid_struct *hid;
	int r;

	hid = (struct rawhid_struct *)h;
	if (!hid) return -1;

	memset(&ov, 0, sizeof(OVERLAPPED));
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) return -1;

	// first byte is report ID, must be zero if report IDs not used
	ret = WriteFile(hid->handle, buf, len, &num, &ov);
	if (ret) {
		if (num == len) {
			//printf("HID/win32:   write success (immediate)\n");
			r = 0;
		} else {
			//printf("HID/win32:   partial write (immediate)\n");
			r = -1;
		}
	} else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (GetOverlappedResult(hid->handle, &ov, &num, TRUE)) {
				if (num == len) {
					//printf("HID/win32:   write success (delayed)\n");
					r = 0;
				} else {
					//printf("HID/win32:   partial write (delayed)\n");
					r = -1;
				}
			} else {
				//printf("HID/win32:   write error (delayed)\n");
				r = -1;
			}
		} else {
			//printf("HID/win32:   write error (immediate)\n");
			r = -1;
		}
	}
	CloseHandle(ov.hEvent);
	return r;
}


#endif // windows



#ifndef OPERATING_SYSTEM
#error Unknown operating system
#endif








