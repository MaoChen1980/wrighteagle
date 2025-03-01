# WrightEagle (Soccer Simulation League 2D)
# BASE SOURCE CODE RELEASE 2016
# Copyright (C) 1998-2016 WrightEagle 2D Soccer Simulation Team,
# Multi-Agent Systems Lab.,
# School of Computer Science and Technology,
# University of Science and Technology of China, China.

# Detect architecture
ARCH := $(shell uname -m)

# Set compiler flags based on architecture
ifeq ($(ARCH),x86_64)
    ARCH_FLAGS := -m64
else ifeq ($(ARCH),i386)
    ARCH_FLAGS := -m32
else ifeq ($(findstring arm,$(ARCH)),arm)
    ARCH_FLAGS := -march=native
else
    ARCH_FLAGS := -march=native
endif

# Export architecture flags for sub-makefiles
export ARCH_FLAGS

# Use environment variables or default values
DEBUG ?= Debug
RELEASE ?= Release

# Detect number of CPU cores for parallel compilation
NUM_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# Common compiler flags
CXX ?= g++
export CXX

# Default target
first: d

# Build targets
all: d r

d:
	@echo "Building debug version ($(ARCH))..."
	@cd ${DEBUG} && $(MAKE) -j$(NUM_JOBS) all || true

r:
	@echo "Building release version ($(ARCH))..."
	@cd ${RELEASE} && $(MAKE) -j$(NUM_JOBS) all || true

# Clean targets
clean:
	@echo "Cleaning build files..."
	@cd ${DEBUG} && $(MAKE) clean || true
	@cd ${RELEASE} && $(MAKE) clean || true
	@rm -f $(shell find . -type f -name 'core*') || true

.PHONY: first all d r clean
