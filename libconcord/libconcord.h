/*
 * vi: formatoptions+=tc textwidth=80 tabstop=8 shiftwidth=8 noexpandtab:
 *
 * $Id: libconcord.h,v 1.22 2010/07/27 19:33:52 jaymzh Exp $
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
 * (C) Copyright Phil Dibowitz 2007
 * (C) Copyright Kevin Timmerman 2007
 */

#ifndef LIBCONCORD_H
#define LIBCONCORD_H

#include <stdio.h>

#ifdef WIN32
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#else

#include <stdint.h>

#endif /* end if win32/else */

#define LC_ERROR 1
#define LC_ERROR_INVALID_DATA_FROM_REMOTE 2
#define LC_ERROR_READ 3
#define LC_ERROR_WRITE 4
#define LC_ERROR_INVALIDATE 5
#define LC_ERROR_ERASE 6
#define LC_ERROR_VERIFY 7
#define LC_ERROR_POST 8
#define LC_ERROR_GET_TIME 9
#define LC_ERROR_SET_TIME 10
#define LC_ERROR_CONNECT 11
#define LC_ERROR_OS 12
#define LC_ERROR_OS_NET 13
#define LC_ERROR_OS_FILE 14
#define LC_ERROR_UNSUPP 15
#define LC_ERROR_INVALID_CONFIG 16
#define LC_ERROR_IR_OVERFLOW 17

/*
 * Filetypes, used by identity_file()
 */
#define LC_FILE_TYPE_CONNECTIVITY  0
#define LC_FILE_TYPE_CONFIGURATION 1
#define LC_FILE_TYPE_FIRMWARE      2
#define LC_FILE_TYPE_LEARN_IR      3

/*
 * Actual C clients are not fully supported yet, but that's the goal...
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * CALLBACK INFORMATION
 *
 * There is currently only one kind of callback, and it's for status
 * information. It should be a void function and takes the following
 * arguments:
 *   uint32_t count - the amount of times this cb has been called in a
 *                    given call of a given functioin
 *   uint32_t curr  - current status (usually bytes read/written)
 *   uint32_t total - total goal status (usually bytes expected to read/write)
 *   void *arg      - opaque object you can pass to functions to have them
 *                    pass back to your callback.
 */
typedef void (*lc_callback)(uint32_t, uint32_t, uint32_t, void*);

/*
 * REMOTE INFORMATION ACCESSORS
 *
 * These take nothing, and return what they say. Simple stuff.
 */
const char *get_mfg();
const char *get_model();
const char *get_codename();
int get_skin();
int get_fw_ver_maj();
int get_fw_ver_min();
int get_fw_type();
int get_hw_ver_maj();
int get_hw_ver_min();
int get_flash_size();
int get_flash_mfg();
int get_flash_id();
const char *get_flash_part_num();
int get_arch();
int get_proto();
const char *get_hid_mfg_str();
const char *get_hid_prod_str();
int get_hid_irl();
int get_hid_orl();
int get_hid_frl();
int get_usb_vid();
int get_usb_pid();
int get_usb_bcd();
char *get_serial(int p);
int get_config_bytes_used();
int get_config_bytes_total();

/*
 * TIME ACCESSORS
 *
 * These can ONLY be called *after* get_time() or set_time(). Each call will
 * initialize the internal time structures to the time used, and these
 * accessors can be used to access that data.
 */
int get_time_second();
int get_time_minute();
int get_time_hour();
int get_time_day();
int get_time_dow();
int get_time_month();
int get_time_year();
int get_time_utc_offset();
const char *get_time_timezone();

/*
 * HELPER FUNCTIONS
 */

/*
 * Translate a return value into an actual error message. Pass in the int
 * you received, get back a string.
 */
const char *lc_strerror(int err);
/*
 * Many functions require you to pass in a ptr which then gets pointed
 * to data that we allocate. You should then call this to clean that
 * data up when you are done with it.
 */
void delete_blob(uint8_t *ptr);

/*
 * Attempt to identify the action to perform to process the given file,
 * given its content.
 *
 * The function will indicate whether the file could be identified using
 * the return value. If the file can be identified, the type of the file
 * will be written to *mode.
 */
int identify_file(uint8_t *in, uint32_t size, int *type);

/*
 * GENERAL REMOTE INTERACTIONS
 */

/*
 * Initialize USB (and WinSock if necessary) and find the remote
 * if possible.
 */
int init_concord();
/*
 * Release the USB device, and tear down anything else necessary.
 */
int deinit_concord();
/*
 * This is another initialization function. Generally speaking you always
 * want to call this before you do anything. It will query the remote about
 * it's data and fill in internal data structures with that information. This
 * counts as a successful "connectivity test"
 */
int get_identity(lc_callback cb, void *cb_arg);
/*
 * Reboot the remote.
 */
int reset_remote();
/*
 * Get the time from the remote. Use the time accessors above to access
 * the data.
 */
int get_time();
/*
 * Set the time on the remote to the system time. To find out what time was
 * used, use the time accessors above.
 */
int set_time();
/*
 * POST to the members.harmonyremote.com website that the connection test was
 * successful. A Connectivity.EZHex file must be passed in so that we
 * can get the URL, cookie information, etc.
 */
int post_connect_test_success(uint8_t *data, uint32_t size);
/*
 * Prior to updating the config, if you want to interact with the website
 * you have to send it some initial data. This does that. The data passed
 * in here is a pointer to the config data config block (with XML - this
 * should NOT be the pointer result from find_binary_start().
 */
int post_preconfig(uint8_t *data, uint32_t size);
/*
 * After writing the config to the remote, this should be called to tell
 * the members.harmonyremote.com website that it was successful.
 */
int post_postconfig(uint8_t *data, uint32_t size);
/*
 * After writing a new firmware to the remote, this should be called to tell
 * the members.harmonyremote.com website that it was successful.
 */
int post_postfirmware(uint8_t *data, uint32_t size);
/*
 * This sends the remote a command to tell it we're about to start
 * writing to it's flash area and that it shouldn't read from it.
 * This must be used before writing a config, firmware, or anything
 * else that touches flash.
 *
 * If something goes wrong, or you change your mind after invalidating
 * flash, you should reboot the device.
 */
int invalidate_flash();

/*
 * CONFIGURATION INTERACTIONS
 */
 
/*
 * Read the config from the remote and store it into the unit8_t array
 * *out. The callback is for status information. See above for CB info.
 *
 * NOTE: The pointer *out should not point to anything useful. We will
 * allocate a char array and point your pointer at it. Use delete_blob to
 * reclaim this memory.
 */
int read_config_from_remote(uint8_t **out, uint32_t *size,
	lc_callback cb, void *cb_arg);
/*
 * Given a config block in the byte array *in that is size big, write
 * it to the remote. This should be *just* the binary blob (see
 * find_binary_start()). CB info above.
 */
int write_config_to_remote(uint8_t *in, uint32_t size,
	lc_callback cb, void *cb_arg);
/*
 * Read the in a file. If it's a standard XML file from the
 * membersharmonyremote.com website, the XML will be included. If it's just
 * the binary blob, that's fine too. size will be returned.
 *
 * NOTE: The pointer *out should not point to anything useful. We will
 * allocate a char array and point your pointer at it. Use delete_blob to
 * reclaim this memory.
 */
int read_file(char *file_name, uint8_t **out, uint32_t *size);
/*
 * Given a binary-only config blob *in, write the config to a file. Unless
 * binary is true, the XML will be constructed and written to the file
 * as well.
 */
int write_config_to_file(uint8_t *in, uint32_t size, char *file_name,
	int binary);
/*
 * After doing a write_config_to_remote(), this should be called to verify
 * that config. The data will be compared to what's in *in.
 */
int verify_remote_config(uint8_t *in, uint32_t size, lc_callback cb,
	void *cb_arg);
/*
 * Preps the remote for a config upgrade.
 *
 * Note that this and finish_config are NO-OPs for most remotes, and even on
 * remotes where it is implemented, testing implies that it's not necessary.
 * However, calling these functions is necessary to completely match the
 * original Windows software, and future remotes may require these functions
 * to be executed to operate correctly.
 */
int prep_config();
/*
 * Tells the remote the config upgrade was successful and that it should
 * use the new config upon next reboot.
 */
int finish_config();
/*
 * Flash can be changed to 0, but not back to 1, so you must erase the
 * flash (to 1) in order to write the flash.
 */
int erase_config(uint32_t size, lc_callback cb, void *cb_arg);
/*
 * Determine the location of binary data within an XML configuration file.
 * (aka skip past the XML portion).
 *
 * config and config size indicate the location and size of the configuration
 * data (for example, what one you might get from read_config_from_remote()).
 *
 * *binary_ptr and *binary_size will set to the location and size of the
 * binary portion of the configuration data.
 */
int find_config_binary(uint8_t *config, uint32_t config_size,
	uint8_t **binary_ptr, uint32_t *binary_size);

/*
 * SAFEMODE FIRMWARE INTERACTIONS
 */
/*
 * Make the safemode area of the flash all 1's so you can write
 * to it.
 */
int erase_safemode(lc_callback cb, void *cb_arg);
/*
 * Same as read_config_from_remote(), but reading the safemode firmware
 * instead.
 *
 * NOTE: The pointer *out should not point to anything useful. We will
 * allocate a char array and point your pointer at it. Use delete_blob to
 * reclaim this memory.
 */
int read_safemode_from_remote(uint8_t **out, uint32_t *size, lc_callback cb,
	void *cb_arg);
/*
 * NOTE: You CAN NOT WRITE SAFEMODE FIRMWARE OVER USB!
 */

/*
 * Write aforementioned safemode data to a file. Note that this is always
 * written as pure binary.
 */
int write_safemode_to_file(uint8_t *in, uint32_t size,char *file_name);

/*
 * FIRMWARE INTERACTIONS
 */
/*
 * We don't yet support firmware upgrades on all remotes. This function
 * will tell you if it's supported on the remote we found. You must pass
 * in whether you intend to do direct or not (direct isn't always supported
 * either). This will return 0 for yes and LC_ERROR_UNSUPP otherwise.
 */
int is_fw_update_supported(int direct);
/*
 * This function tells you if the config will be wiped out by a live
 * firmware upgrade (some remotes use the config area in memory as a
 * staging area for the firmware). This will return 0 for yes and LC_ERROR
 * for no.
 */
int is_config_safe_after_fw();
/*
 * Preps the remote for a firmware upgrade
 */
int prep_firmware();
/*
 * Tells the remote the firmware upgrade was successful and that it should
 * copy the firmware from the "staging" area to the live area on next reboot.
 * Don't forget to reboot.
 */
int finish_firmware();
/*
 * Make the firmware area of the flash all 1's so you can write
 * to it.
 */
int erase_firmware(int direct, lc_callback cb, void *cb_arg);
/*
 * Same as read_config_from_remote(), but reading the firmware instead.
 *
 * NOTE: The pointer *out should not point to anything useful. We will
 * allocate a char array and point your pointer at it. Use delete_blob to
 * reclaim this memory.
 */
int read_firmware_from_remote(uint8_t **out, uint32_t *size, lc_callback cb,
	void *cb_arg);
/*
 * Same as write_config_to_remote(), but with the firmware instead.
 */
int write_firmware_to_remote(uint8_t *in, uint32_t size, int direct,
	lc_callback cb,	void *cb_arg);
/*
 * Same as write_config_to_file(), but with firmware instead. Note
 * that unless binary is specified, the firmware is broken into chunks
 * and written in ASCII-encoded HEX in XML <DATA> blocks, the way
 * the members.harmonyremote.com website delivers it.
 */
int write_firmware_to_file(uint8_t *in, uint32_t size, char *file_name,
	int binary);
/*
 * Extract the binary firmware from a file read in with
 * read_file(). Obviously this function isn't necessary in
 * binary mode.
 */
int extract_firmware_binary(uint8_t *xml, uint32_t xml_size, uint8_t **out,
	uint32_t *size);

/*
 * IR-stuff
 * ===========================
 * Data structure information:
 *
 * carrier_clock    : in Hz, usually ~36000..40000
 * ir_signal        : IR mark/space durations (alternating) in microsconds
 * ir_signal_length : total number of mark/space durations in ir_signal
 *      ir_signal will start with a mark and end with a space duration,
 *      hence ir_signal_length will always be an even number.
 * 
 * They are usually filled in by calling learn_from_remote(...),
 * to learn IR signals from an existing other remote, but may also
 * be set by the application, e.g. be derived from Pilips Pronto Hex
 * codes or RC5/NEC/... command codes (separate conversion library required).
 *
 * encoded posting format : IR code data converted to Logitech 
 *     posting string format, returned by encode_for_posting.
 *     Having the encoding separate from the posting keeps the
 *     parameter list of post_new_code() tidy and allows the
 *     application to display the encoded signal when desired.
 */

/*
 * Scan the contents of the received LearnIR.EZTut file
 * (read into *data[size]) for the key names to be learned.
 *
 * Fills key_names with the found names and key_names_length
 * with the number of found key names.
 * Returns 0 for success, or an error code in case of failure.
 *
 * Memory allocated for the strings must be freed by the caller
 * via delete_key_names() when not needed any longer.
 */
int get_key_names(uint8_t *data, uint32_t size,
	char ***key_names, uint32_t *key_names_length);

void delete_key_names(char **key_names, uint32_t key_names_length);

/*
 * Fill ir_data with IR code learned from other remote
 * via Harmony IR receiver.
 *
 * Returns 0 for success, error code for failure.
 *
 * Memory allocated for ir_signal must be freed by the caller
 * via delete_ir_signal() when not needed any longer.
 */
int learn_from_remote(uint32_t *carrier_clock,
	uint32_t **ir_signal, uint32_t *ir_signal_length,
	lc_callback cb, void *cb_arg);

void delete_ir_signal(uint32_t *ir_signal);

/*
 * Fill encoded_signal with IR code encoded to Logitech
 * posting string format.
 *
 * Returns 0 for success, error code in case of failure.
 *
 * Memory allocated for the string must be freed by the caller
 * via delete_post_string() when not needed any longer.
 */
int encode_for_posting(uint32_t carrier_clock,
	uint32_t *ir_signal, uint32_t ir_signal_length,
	char **encoded_signal);

void delete_encoded_signal(char *encoded_signal);

/*
 * Post encoded IR-code with key_name and additional 
 * information from XML data[size] to Logitech.
 *
 * Logitech will only accept keynames already present in the
 * database or user-defined via 'Learn new Key' web page
 * for the current device.
 *
 * Returns 0 for success, error code for failure.
 */
int post_new_code(uint8_t *data, uint32_t size, 
	char *key_name, char *encoded_signal);

#ifdef __cplusplus
}
#endif

#endif /* LIBCONCORD_H */
