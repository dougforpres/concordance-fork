#!/usr/bin/env python

#
# vi: formatoptions+=tc textwidth=80 tabstop=8 shiftwidth=8 noexpandtab:
#
# $Id: setup.py,v 1.4 2010/08/28 16:26:27 jaymzh Exp $
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

from distutils.core import setup

setup(
    name='libconcord',
    version='0.23',
    py_modules=['libconcord'],
)

