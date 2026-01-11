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

# ANSI color codes for output
COLOR_RESET := \033[0m
COLOR_BOLD := \033[1m
COLOR_GREEN := \033[32m
COLOR_YELLOW := \033[33m
COLOR_BLUE := \033[34m
COLOR_RED := \033[31m

# Default target
first: d

# Build targets
all: d r

help:
	@echo "$(COLOR_BOLD)WrightEagle Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_GREEN)Available targets:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)make first$(COLOR_RESET)     - Build debug version (default)"
	@echo "  $(COLOR_BLUE)make all$(COLOR_RESET)       - Build both debug and release"
	@echo "  $(COLOR_BLUE)make d$(COLOR_RESET)         - Build debug version"
	@echo "  $(COLOR_BLUE)make r$(COLOR_RESET)         - Build release version"
	@echo "  $(COLOR_BLUE)make clean$(COLOR_RESET)     - Clean all build artifacts"
	@echo "  $(COLOR_BLUE)make distclean$(COLOR_RESET)  - Clean everything including generated files"
	@echo "  $(COLOR_BLUE)make help$(COLOR_RESET)      - Show this help message"
	@echo ""
	@echo "$(COLOR_GREEN)Variables:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)NUM_JOBS=$(COLOR_RESET)$(NUM_JOBS)     - Number of parallel jobs"
	@echo "  $(COLOR_BLUE)CXX=$(COLOR_RESET)$(CXX)               - C++ compiler"
	@echo ""

d:
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)==> Building debug version ($(ARCH))...$(COLOR_RESET)"
	@if [ -d "$(DEBUG)" ]; then \
		cd $(DEBUG) && $(MAKE) -j$(NUM_JOBS) all; \
	else \
		echo "$(COLOR_RED)Error: Debug directory '$(DEBUG)' not found$(COLOR_RESET)"; \
		exit 1; \
	fi
	@echo "$(COLOR_GREEN)==> Debug build complete!$(COLOR_RESET)"

r:
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)==> Building release version ($(ARCH))...$(COLOR_RESET)"
	@if [ -d "$(RELEASE)" ]; then \
		cd $(RELEASE) && $(MAKE) -j$(NUM_JOBS) all; \
	else \
		echo "$(COLOR_RED)Error: Release directory '$(RELEASE)' not found$(COLOR_RESET)"; \
		exit 1; \
	fi
	@echo "$(COLOR_GREEN)==> Release build complete!$(COLOR_RESET)"

# Clean targets
clean:
	@echo "$(COLOR_YELLOW)==> Cleaning build files...$(COLOR_RESET)"
	@if [ -d "$(DEBUG)" ]; then \
		cd $(DEBUG) && $(MAKE) clean; \
	fi
	@if [ -d "$(RELEASE)" ]; then \
		cd $(RELEASE) && $(MAKE) clean; \
	fi
	@find . -type f -name 'core*' -delete 2>/dev/null || true
	@echo "$(COLOR_GREEN)==> Clean complete!$(COLOR_RESET)"

distclean: clean
	@echo "$(COLOR_YELLOW)==> Cleaning generated files...$(COLOR_RESET)"
	@find . -type f \( -name '*.d' -o -name '*.o' \) -delete 2>/dev/null || true
	@rm -f Debug/WEBase Release/WEBase 2>/dev/null || true
	@echo "$(COLOR_GREEN)==> Distclean complete!$(COLOR_RESET)"

.PHONY: first all d r clean distclean help
