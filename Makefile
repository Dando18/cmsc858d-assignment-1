CC = clang++-12
OPT = -O3
WARNINGS = -Wall -Werror -Wextra -pedantic -Wshadow
STD = -std=c++20
DEBUGFLAGS = -DNDEBUG
BOUNDS_CHECKING =
FLAGS = $(OPT) $(WARNINGS) $(STD) $(DEBUGFLAGS) $(BOUNDS_CHECKING) -I$(INCDIR)

ifeq ($(DEBUG),1)
DEBUGFLAGS := $(filter-out -DNDEBUG, $(DEBUGFLAGS))
OPT = -Og
DEBUGFLAGS = -g
NO_BOUNDS_CHECKING = 0
endif

ifeq ($(NO_BOUNDS_CHECKING),1)
BOUNDS_CHECKING = -DNO_BOUNDS_CHECKING
endif

TESTFLAGS = $(filter-out -DNDEBUG -DNO_BOUNDS_CHECKING,$(FLAGS))

BINDIR = bin
SRCDIR = src
INCDIR = include
TARGETS = $(BINDIR)/experiment $(BINDIR)/tests

all: $(TARGETS)

$(BINDIR)/experiment: $(SRCDIR)/experiment.cc $(INCDIR)/bitvector.h $(INCDIR)/sparsearray.h $(INCDIR)/utilities.h $(BINDIR)
	$(CC) $(FLAGS) -o $@ $<

$(BINDIR)/tests: $(SRCDIR)/tests.cc $(INCDIR)/bitvector.h $(INCDIR)/sparsearray.h $(INCDIR)/utilities.h $(BINDIR)
	$(CC) $(TESTFLAGS) -o $@ $< 

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(TARGETS)