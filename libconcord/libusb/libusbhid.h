//
//  libusbhid.h
//  ConcordanceTest
//
//  Created by Doug Henderson on 10/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef ConcordanceTest_libusbhid_h
#define ConcordanceTest_libusbhid_h

#include "../lc_internal.h"
#include "../libconcord.h"

#include "../hid.h"
#ifdef WIN32
#include "win/usb.h"
#else
#include "libusb.h"
#endif
#include <errno.h>

#define LOGITECH_VID 0x046D
#define LOGITECH_MIN_PID 0xc110
#define LOGITECH_MAX_PID 0xc14f
#define NATIONAL_VID 0x0400
#define NATIONAL_PID 0xc359

static libusb_context *context = NULL;
static libusb_device_handle *h_hid = NULL;
static unsigned int irl;
static unsigned int orl;
static int ep_read = -1;
static int ep_write = -1;

int InitUSB();

void ShutdownUSB();

void check_ep(const libusb_endpoint_descriptor &ued);

bool is_harmony(struct libusb_device *h_dev);

/*
 * Find a HID device with VendorID == 0x046D ||
 *    (VendorID == 0x0400 && ProductID == 0xC359)
 */
int FindRemote(THIDINFO &hid_info);

int HID_WriteReport(const uint8_t *data);

int HID_ReadReport(uint8_t *data, unsigned int timeout);

#endif
