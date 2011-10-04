/*
 * vi: formatoptions+=tc textwidth=80 tabstop=8 shiftwidth=8 noexpandtab:
 *
 * $Id: hid.h,v 1.7 2008/04/11 05:05:26 jaymzh Exp $
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
 */

#ifndef HID_H
#define HID_H

struct THIDINFO {
	string mfg;
	string prod;
	unsigned int vid;
	unsigned int pid;
	unsigned int ver;
	unsigned int irl;
	unsigned int orl;
	unsigned int frl;
};

int InitUSB();
void ShutdownUSB();

int FindRemote(THIDINFO &hid_info);

int HID_WriteReport(const uint8_t *data);
int HID_ReadReport(uint8_t *data, unsigned int timeout = 500);

#endif
