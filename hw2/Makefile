CC := gcc
LEX := flex
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include
STUBD := stubs

MAIN  := $(BLDD)/main.o

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN) $(AUX), $(ALL_OBJF))
ALL_STUBF := $(TSTD)/curses_stub.c

TEST_ALL_SRCF := $(shell find $(TSTD) -type f -name *.c)
TEST_SRCF := $(filter-out $(ALL_STUBF), $(TEST_ALL_SRCF))

INC := -I $(INCD)

ifdef NO_MAXLINE
NO_MAXLINE_FLAG := -DNO_MAXLINE
endif

CFLAGS := -Wall -Werror -Wno-unused-variable -Wno-unused-function $(NO_MAXLINE_FLAG) -MMD
COLORF := -DCOLOR
DFLAGS := -g -DDEBUG -DCOLOR
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=c99 -D_DEFAULT_SOURCE
TEST_LIB := -lcriterion
LIBS := 
CURSES_LIBS := -lcurses -ltermcap

CFLAGS += $(STD)

EXEC := browse
TEST_EXEC := $(EXEC)_tests
STUBBED_EXEC := $(EXEC)_stubbed

.PHONY: clean all setup debug

all: setup $(BIND)/$(EXEC) $(BIND)/$(TEST_EXEC)

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS) $(COLORF)
debug: all

setup: $(BIND) $(BLDD)
$(BIND):
	mkdir -p $(BIND)
$(BLDD):
	mkdir -p $(BLDD)

$(BIND)/$(EXEC): $(ALL_OBJF)
	$(CC) $^ -o $@ $(CURSES_LIBS) $(LIBS)

$(BIND)/$(STUBBED_EXEC): $(ALL_OBJF) $(ALL_STUBF)
	$(CC) $^ -o $@ $(LIBS)

$(BIND)/$(TEST_EXEC): $(ALL_FUNCF) $(TEST_SRCF) $(TEST_REF_OBJF) $(BIND)/$(STUBBED_EXEC)
	$(CC) $(CFLAGS) $(INC) $(ALL_FUNCF) $(TEST_SRCF) $(TEST_REF_OBJF) \
	$(TEST_LIB) $(CURSES_LIBS) $(LIBS) -o $@

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(STUBD)/%.o: $(STUBD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -rf $(BLDD) $(BIND)

.PRECIOUS: $(BLDD)/*.d
-include $(BLDD)/*.d
