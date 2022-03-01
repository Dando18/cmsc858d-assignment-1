CC = clang++-12
OPT = -O3
WARNINGS = -Wall -Werror -pedantic
STD = -std=c++20
DEBUGFLAGS = -DNDEBUG
BOUNDS_CHECKING =
FLAGS = $(OPT) $(WARNINGS) $(STD) $(DEBUGFLAGS) $(BOUNDS_CHECKING)

ifeq ($(DEBUG),1)
DEBUGFLAGS := $(filter-out -DNDEBUG, $(DEBUGFLAGS))
OPT = -Og
DEBUGFLAGS = -g
NO_BOUNDS_CHECKING = 0
endif

ifeq ($(NO_BOUNDS_CHECKING),1)
BOUNDS_CHECKING = -DNO_BOUNDS_CHECKING
endif

TESTFLAGS = $(filter-out -DNDEBUG,$(FLAGS))

BINDIR = bin
TARGETS = $(BINDIR)/experiment $(BINDIR)/tests

all: $(TARGETS)

$(BINDIR)/experiment: experiment.cc bitvector.h sparsearray.h
	$(CC) $(FLAGS) -o $@ $<

$(BINDIR)/tests: tests.cc bitvector.h sparsearray.h
	$(CC) $(TESTFLAGS) -o $@ $< 

clean:
	rm -f $(TARGETS)