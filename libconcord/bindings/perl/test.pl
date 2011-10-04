#!/usr/bin/perl

#
# vi: formatoptions+=tc textwidth=80 tabstop=8 shiftwidth=8 noexpandtab:
#
# $Id: test.pl,v 1.3 2010/07/27 19:33:53 jaymzh Exp $
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# (C) Copyright Phil Dibowitz 2008
#

#
# WARNING WARNING WARNING! READ THIS!!
# NOTE: This is NOT meant for general use. It is a test script for
# 	the perl bindings and NOT for production use. It makes lots
#	of assumptions (a model 880, no bin mode, etc.), does almost
#	minimal error checking, and has sloppy output. This is for
#	TESTING ONLY!
#

use strict;
use warnings;

use Getopt::Long qw(:config bundling);
use concord;

use constant FW_FILE => '/tmp/perl_fw';
use constant CONFIG_FILE => '/tmp/perl_fw';
use constant IR_FILE => '/tmp/LearnIr.EZTut';

sub cb
{
	my ($count, $curr, $total, $data) = @_;

	print '*';
	#print "CALLBACK: count: $count, curr: $curr, total $total, data:"
	#	. " $data\n";
	return;
}

sub dump_config
{
	my ($blob, $size);
	
	my $err;
	print "Reading config from remote ";
	($err, $blob, $size) = concord::read_config_from_remote(\&cb, 1);
	if ($err != 0) {
		print "failed to read config from remote\n";
		exit(1);
	}
	print "done\n";
	print "Writing config to file ";
	$err = concord::write_config_to_file($blob, $size, CONFIG_FILE, 0);
	if ($err != 0) {
		print "failed to write config to file\n";
		exit(1);
	}
	print "done\n";
	concord::delete_blob($blob);
}

sub dump_firmware
{

	my ($blob, $size);

	my $err;
	print "Reading fw ";
	($err, $blob, $size) = concord::read_firmware_from_remote(\&cb, 1);
	if ($err != 0) {
		print "Failed to read fw from remote\n";
		exit(1);
	}
	print "done\n";
	print "Writing fw to file ";
	$err = concord::write_firmware_to_file($blob, $size, FW_FILE, 0);
	if ($err != 0) {
		print "failed to write fw to file\n";
		exit(1);
	}
	print "done\n";
	concord::delete_blob($blob);
}

sub upload_config
{
	my ($blob, $size, $binblob, $binsize);

	my $err;
	print "Reading config ";
	($err, $blob, $size) = concord::read_file(CONFIG_FILE);
	if ($err) {
		print "Failed to read config from file\n";
		exit(1);
	}
	print "done\n";
	print "Finding binary ";
	($err, $binblob, $binsize) = concord::find_config_binary($blob, $size);
	if ($err) {
		print "Failed to find bin\n";
		exit(1);
	}
	print "done\n";
	print "Preparing for config update ";
	concord::prep_config();
	if ($err) {
		print "Failed to prepare for config update";
		exit(1);
	}
	print "done\n";
	print "Invalidating flash ";
	concord::invalidate_flash();
	if ($err) {
		print "Failed to invalidate flash";
		exit(1);
	}
	print "done\n";
	print "Erasing flash ";
	concord::erase_config($binsize, \&cb, 1);
	if ($err) {
		print "Failed to erase flash";
		exit(1);
	}
	print "done\n";
	print "Writing config ";
	concord::write_config_to_remote($binblob, $binsize, \&cb, 1);
	if ($err) {
		print "Failed to write config to remote";
		exit(1);
	}
	print "done\n";
	print "Verifying config ";
	concord::verify_remote_config($binblob, $binsize, \&cb, 1);
	if ($err) {
		print "Failed to write config to remote";
		exit(1);
	}
	print "Finalizing config update ";
	concord::finish_config();
	if ($err) {
		print "Failed to finalize config update";
		exit(1);
	}
	print "done\n";

	concord::delete_blob($blob);
	print "done\n";

}

sub upload_firmware
{
	my ($err, $blob, $size, $binblob, $binsize);

	$err = concord::is_fw_update_supported(0);
	if ($err) {
		print "Sorry, firmware is not supported on your device\n";
		exit(1);
	}
	print "Reading fw\n";
	($err, $blob, $size) = concord::read_file(FW_FILE);
	if ($err) {
		print "Failed to read config from file\n";
		exit(1);
	}
	print "extracting fw ";
	print "done\n";
	($err, $binblob, $binsize) = concord::extract_firmware_binary($blob, $size);
	if ($err) {
		print "Failed extract fw\n";
		exit(1);
	}
	print "done\n";
	print "preping fw ";
	$err = concord::prep_firmware();
	if ($err) {
		print "Failed prep fw\n";
		exit(1);
	}
	print "done\n";
	print "invalidating flash ";
	$err = concord::invalidate_flash();
	if ($err) {
		print "Failed invalidate flash\n";
		exit(1);
	}
	print "done\n";
	print "erasing flash ";
	$err = concord::erase_firmware(0, \&cb, 0);
	if ($err) {
		print "Failed invalidate flash\n";
		exit(1);
	}
	print "done\n";
	print "erasing fw ";
	$err = concord::write_firmware_to_remote($binblob, $binsize, 0, \&cb, 0);
	if ($err) {
		print "Failed invalidate flash\n";
		exit(1);
	}
	print "done\n";
	print "finishing fw ";
	$err = concord::finish_firmware();
	if ($err) {
		print "Failed finish fw\n";
		exit(1);
	}
	print "done\n";
	concord::delete_blob($blob);
	concord::delete_blob($binblob);
}

sub learn_ir_commands
{
	my ($err, $blob, $size, $binblob, $binsize);

	print "Reading IR file ";
	($err, $blob, $size) = concord::read_file(IR_FILE);
	print "done\n";

	print "Getting key names ";
	my $key_names;
	($err, $key_names) =
		concord::get_key_names($blob, $size);
	print "done\n";

	my ($carrier_clock, $ir_signal, $ir_length); 

	for (my $i = 0; $i < scalar(@$key_names); $i++) {
		print "KEY: $key_names->[$i]\n";
		print "Press the right key within 5 seconds... ";
		($err, $carrier_clock, $ir_signal, $ir_length) =
			concord::learn_from_remote(\&cb, 0);
		if ($err) {
			print "Failed to learn\n";
			exit(1);
		}

		my $str;
		($err, $str) = concord::encode_for_posting($carrier_clock,
			$ir_signal, $ir_length);
		
		if ($err) {
			print "Failed to encode\n";
			exit(1);
		}

		concord::delete_ir_signal($ir_signal);

		$err = concord::post_new_code($blob, $size, $key_names->[$i],
				$str);

		if ($err) {
			print "Failed to post\n";
			exit(1);
		}
		print "done\n";
	}
}
	

#
# main
#


select STDOUT;
$| = 1;

my $bar = 'val';

my $ret = concord::init_concord();
if ($ret != 0) {
	print "Failed to init concord\n";
	exit;
}
print "Get identity ";
concord::get_identity(\&cb, $bar);
print " done\n";

print 'mfg: ' . concord::get_mfg() . "\n";
print 'mfg: ' . concord::get_model() . "\n";

my $opts = {};
GetOptions($opts,
	'dump-config|c',
	'upload-config|C',
	'dump-firmware|f',
	'upload-firmware|F',
	'learn-ir|l',
	) || die();

if (keys(%$opts) != 1) {
	print "Only one mode is allowed\n";
	exit(1);
}

if (exists($opts->{'dump-config'})) {
	dump_config();
} elsif (exists($opts->{'upload-config'})) {
	upload_config();
	print "resetting...\n";
	concord::reset_remote();
} elsif (exists($opts->{'dump-firmware'})) {
	dump_firmware();
} elsif (exists($opts->{'upload-firmware'})) {
	upload_firmware();
	print "resetting...\n";
	concord::reset_remote();
} elsif (exists($opts->{'learn-ir'})) {
	learn_ir_commands();
}

