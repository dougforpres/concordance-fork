/*
 * vi: formatoptions+=tc textwidth=80 tabstop=8 shiftwidth=8 noexpandtab:
 *
 * $Id: libusbhid.cpp,v 1.19 2010/07/17 22:17:37 jaymzh Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * (C) Copyright Kevin Timmerman 2007
 * (C) Copyright Phil Dibowitz 2007
 */

#include "libusbhid.h"

int InitUSB()
{
	libusb_init(&context);
    libusb_set_debug(context, 3);
    
	return 0;
}

void ShutdownUSB()
{
	if (h_hid) {
		libusb_release_interface(h_hid,0);
	}
    
    libusb_exit(context);
}

void check_ep(const libusb_endpoint_descriptor &ued)
{
	debug("address %02X attrib %02X max_length %i",
		ued.bEndpointAddress, ued.bmAttributes,
		ued.wMaxPacketSize);

	if ((ued.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
	    LIBUSB_TRANSFER_TYPE_INTERRUPT) {
		if (ued.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) {
			if (ep_read == -1) {
				ep_read = ued.bEndpointAddress;
				// hack! todo: get from HID report descriptor
				irl = ued.wMaxPacketSize;
			}
		} else {
			if (ep_write == -1) {
				ep_write = ued.bEndpointAddress;
				// hack! todo: get from HID report descriptor
				orl = ued.wMaxPacketSize;
			}
		}
	}
}

bool is_harmony(struct libusb_device *h_dev)
{
	/* IF vendor == logitech AND product is in range of harmony
	 *   OR vendor == National Semiconductor and product is harmony
	 */
    libusb_device_descriptor desc;
    
    libusb_get_device_descriptor(h_dev, &desc);
    // TODO - Deal with errors!
    
	if ((desc.idVendor == LOGITECH_VID
	      && (desc.idProduct >= LOGITECH_MIN_PID
	          && desc.idProduct <= LOGITECH_MAX_PID))
	    || (desc.idVendor == NATIONAL_VID
	          && desc.idProduct == NATIONAL_PID)) {
		return true;
	}
	return false;
}

/*
 * Find a HID device with VendorID == 0x046D ||
 *    (VendorID == 0x0400 && ProductID == 0xC359)
 */
int FindRemote(THIDINFO &hid_info)
{
    libusb_device **devices;
    
    ssize_t num_devices = libusb_get_device_list(context, &devices);

	libusb_device *h_dev;
	bool found = false;
    
	for (int i = 0; i < num_devices && !found; i++) {
        h_dev = (libusb_device *)(devices[i]);
        
        if (is_harmony(h_dev)) {
			found = true;
			break;
		}
	}

	if (h_dev) {
		libusb_open(h_dev, &h_hid);
	}
    
	if (!h_hid) {
		debug("Failed to establish communication with remote: %s",
			usb_strerror());
		return LC_ERROR_CONNECT;
	}

#ifdef linux
	/*
	 * Before we attempt to claim the interface, lets go ahead and get
	 * the kernel off of it, in case it claimed it already.
	 *
	 * This is ONLY available when on Linux. We don't check for an error
	 * because it will error if no kernel driver is attached to it.
	 */
	usb_detach_kernel_driver_np(h_hid, 0);
#endif

	int err;
	if ((err = libusb_set_configuration(h_hid, 1))) {
		debug("Failed to set device configuration: %d (%s)", err,
			usb_strerror());
		return err;
	}

	if ((err = libusb_claim_interface(h_hid, 0))) {
		debug("Failed to claim interface: %d (%s)", err,
			usb_strerror());
		return err;
	}

    libusb_device_descriptor desc;
    libusb_config_descriptor *uc = NULL;
    
    libusb_get_device_descriptor(h_dev, &desc);
    // TODO - Deal with errors!

	unsigned char maxconf = desc.bNumConfigurations;
    
	for (unsigned char j = 0; j < maxconf; ++j) {
		libusb_get_config_descriptor(h_dev, j, &uc);
        
		unsigned char maxint = uc->bNumInterfaces;
        
		for (unsigned char k = 0; k < maxint; ++k) {
			const libusb_interface &ui = uc->interface[k];
            
			unsigned char maxalt = ui.num_altsetting;
            
			for (unsigned char l = 0; l < maxalt; ++l) {
				const libusb_interface_descriptor &uid = ui.altsetting[l];

				debug("bNumEndpoints %i", uid.bNumEndpoints);
                
				unsigned char maxep = uid.bNumEndpoints;
                
				for (unsigned char n = 0; n < maxep; ++n) {
					check_ep(uid.endpoint[n]);
				}
			}
		}
	}

	if (ep_read == -1 || ep_write == -1) return 1;

	// Fill in hid_info

	unsigned char s[128];
	libusb_get_string_descriptor_ascii(h_hid, desc.iManufacturer, s, sizeof(s));
	hid_info.mfg = (char *)s;
	libusb_get_string_descriptor_ascii(h_hid, desc.iProduct, s, sizeof(s));
	hid_info.prod = (char *)s;

	hid_info.vid = desc.idVendor;
	hid_info.pid = desc.idProduct;
	hid_info.ver = desc.bcdDevice;

	hid_info.irl = irl;
	hid_info.orl = orl;
	hid_info.frl = 0;/// ???

	return 0;
}

int HID_WriteReport(const uint8_t *data)
{
	/*
	 * In Windows you send an preceeding 0x00 byte with
	 * every command, and the codebase used to do that, and we'd
	 * skip the first byte here. Now, we do not assume this, we send
	 * wholesale here, and add the 0 in the windows code.
	 */
    int written = 0;
    
	const int err = libusb_bulk_transfer(h_hid, ep_write, (unsigned char *)data, orl, &written, 500);

	if (err < 0) {
		debug("Failed to write to device: %d (%s)", err,
			usb_strerror());
		return err;
	}

	return 0;
}

int HID_ReadReport(uint8_t *data, unsigned int timeout)
{
	/* Note default timeout is set to 500 in hid.h */
    int read = 0;
    
	const int err = libusb_bulk_transfer(h_hid, ep_read, (unsigned char *)data, irl, &read, timeout);

	if (err == -ETIMEDOUT) {
		debug("Timeout on interrupt read from device");
		return err;
	}

	if (err < 0) {
		debug("Failed to read from device: %d (%s)", err,
			usb_strerror());
		return err;
	}

	return 0;
}

