# WrightEagle (Soccer Simulation League 2D)
# BASE SOURCE CODE RELEASE 2016
# Copyright (C) 1998-2016 WrightEagle 2D Soccer Simulation Team,
# Multi-Agent Systems Lab.,
# School of Computer Science and Technology,
# University of Science and Technology of China, China.

# Detect OS and architecture
OS_NAME := $(shell uname -s | tr A-Z a-z)
ARCH := $(shell uname -m)

# Set compiler flags based on architecture
ifeq ($(ARCH),x86_64)
    ARCH_FLAGS := -m64
else ifeq ($(ARCH),i386)
    ARCH_FLAGS := -m32
else ifeq ($(findstring arm,$(ARCH)),arm)
    ARCH_FLAGS := -march=native
else ifeq ($(findstring aarch64,$(ARCH)),aarch64)
    ARCH_FLAGS := -march=native
else
    ARCH_FLAGS := -march=native
endif

# Export architecture flags for sub-makefiles
export ARCH_FLAGS

# Use environment variables or default values
DEBUG ?= Debug
RELEASE ?= Release
PROFILE ?= Profile

# Detect number of CPU cores for parallel compilation
NUM_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)

# Common compiler flags
CXX ?= g++
CXX_VERSION := $(shell $(CXX) -dumpversion)
export CXX

# ANSI color codes for output
COLOR_RESET := \033[0m
COLOR_BOLD := \033[1m
COLOR_GREEN := \033[32m
COLOR_YELLOW := \033[33m
COLOR_BLUE := \033[34m
COLOR_RED := \033[31m
COLOR_CYAN := \033[36m

# Default target
first: d

# Build targets
all: d r p

help:
	@echo "$(COLOR_BOLD)WrightEagle Build System$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_GREEN)Build targets:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)make first$(COLOR_RESET)     - Build debug version (default)"
	@echo "  $(COLOR_BLUE)make all$(COLOR_RESET)       - Build debug, release, and profile versions"
	@echo "  $(COLOR_BLUE)make d$(COLOR_RESET)         - Build debug version"
	@echo "  $(COLOR_BLUE)make r$(COLOR_RESET)         - Build release version"
	@echo "  $(COLOR_BLUE)make p$(COLOR_RESET)         - Build profile version"
	@echo ""
	@echo "$(COLOR_GREEN)Utility targets:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)make clean$(COLOR_RESET)     - Clean all build artifacts"
	@echo "  $(COLOR_BLUE)make distclean$(COLOR_RESET)  - Clean everything including generated files"
	@echo "  $(COLOR_BLUE)make help$(COLOR_RESET)      - Show this help message"
	@echo ""
	@echo "$(COLOR_GREEN)Development targets:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)make format$(COLOR_RESET)        - Format source code with clang-format"
	@echo "  $(COLOR_BLUE)make check-syntax$(COLOR_RESET)  - Check syntax without building"
	@echo "  $(COLOR_BLUE)make static-analysis$(COLOR_RESET) - Run static analysis with cppcheck"
	@echo ""
	@echo "$(COLOR_GREEN)Variables:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)NUM_JOBS=$(COLOR_RESET)$(NUM_JOBS)     - Number of parallel jobs"
	@echo "  $(COLOR_BLUE)CXX=$(COLOR_RESET)$(CXX)               - C++ compiler (currently $(CXX_VERSION))"
	@echo "  $(COLOR_BLUE)DEBUG=$(COLOR_RESET)$(DEBUG)             - Debug build directory"
	@echo "  $(COLOR_BLUE)RELEASE=$(COLOR_RESET)$(RELEASE)         - Release build directory"
	@echo "  $(COLOR_BLUE)PROFILE=$(COLOR_RESET)$(PROFILE)         - Profile build directory"
	@echo ""
	@echo "$(COLOR_GREEN)Detected:$(COLOR_RESET)"
	@echo "  $(COLOR_BLUE)Architecture:$(COLOR_RESET) $(ARCH)"
	@echo "  $(COLOR_BLUE)OS:$(COLOR_RESET) $(OS_NAME)"
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

p:
	@echo "$(COLOR_BOLD)$(COLOR_CYAN)==> Building profile version ($(ARCH))...$(COLOR_RESET)"
	@if [ -d "$(PROFILE)" ]; then \
		cd $(PROFILE) && $(MAKE) -j$(NUM_JOBS) all; \
	else \
		echo "$(COLOR_RED)Error: Profile directory '$(PROFILE)' not found$(COLOR_RESET)"; \
		exit 1; \
	fi
	@echo "$(COLOR_GREEN)==> Profile build complete!$(COLOR_RESET)"

# Clean targets
clean:
	@echo "$(COLOR_YELLOW)==> Cleaning build files...$(COLOR_RESET)"
	@if [ -d "$(DEBUG)" ]; then \
		cd $(DEBUG) && $(MAKE) clean; \
	fi
	@if [ -d "$(RELEASE)" ]; then \
		cd $(RELEASE) && $(MAKE) clean; \
	fi
	@if [ -d "$(PROFILE)" ]; then \
		cd $(PROFILE) && $(MAKE) clean; \
	fi
	@find . -type f -name 'core*' -delete 2>/dev/null || true
	@echo "$(COLOR_GREEN)==> Clean complete!$(COLOR_RESET)"

distclean: clean
	@echo "$(COLOR_YELLOW)==> Cleaning generated files...$(COLOR_RESET)"
	@find . -type f \( -name '*.d' -o -name '*.o' \) -delete 2>/dev/null || true
	@rm -f Debug/WEBase Release/WEBase Profile/WEBase 2>/dev/null || true
	@echo "$(COLOR_GREEN)==> Distclean complete!$(COLOR_RESET)"

# Development support targets
format:
	@echo "$(COLOR_CYAN)Formatting code with clang-format...$(COLOR_RESET)"
	@if command -v clang-format >/dev/null 2>&1; then \
		find src/ -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cc' | xargs clang-format -i -style=file 2>/dev/null || \
		clang-format -i -style=file src/*/*; \
		echo "$(COLOR_GREEN)Code formatting completed$(COLOR_RESET)"; \
	else \
		echo "$(COLOR_YELLOW)clang-format not found. Please install it to use format target$(COLOR_RESET)"; \
	fi

check-syntax:
	@echo "$(COLOR_CYAN)Checking syntax without building...$(COLOR_RESET)"
	@$(CXX) ${ARCH_FLAGS} -fsyntax-only -Wall -Wextra -std=c++11 -Isrc $(shell find src -name '*.cpp') 2>&1
	@echo "$(COLOR_GREEN)Syntax check completed$(COLOR_RESET)"

static-analysis:
	@echo "$(COLOR_CYAN)Running static analysis with cppcheck...$(COLOR_RESET)"
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c++11 --verbose --force --check-library -I src/ src/ 2>&1; \
		echo "$(COLOR_GREEN)Static analysis completed$(COLOR_RESET)"; \
	else \
		echo "$(COLOR_YELLOW)cppcheck not found. Please install it to use static-analysis target$(COLOR_RESET)"; \
	fi

.PHONY: first all d r p clean distclean help format check-syntax static-analysis
