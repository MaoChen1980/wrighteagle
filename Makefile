# WrightEagle (Soccer Simulation League 2D)
# BASE SOURCE CODE RELEASE 2016
# Copyright (C) 1998-2016 WrightEagle 2D Soccer Simulation Team,
# Multi-Agent Systems Lab.,
# School of Computer Science and Technology,
# University of Science and Technology of China, China.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# Use environment variables or default values
DEBUG ?= Debug
RELEASE ?= Release

# Dynamically determine the number of parallel jobs based on the system
NUM_JOBS ?= $(shell nproc)

first: d

all: d r

d:
	@cd ${DEBUG} && $(MAKE) -j${NUM_JOBS} all || true

r:
	@cd ${RELEASE} && $(MAKE) -j${NUM_JOBS} all || true

clean:
	@cd ${DEBUG} && $(MAKE) clean || true
	@cd ${RELEASE} && $(MAKE) clean || true
	@rm -f $(shell find . -type f -name 'core*') || true
