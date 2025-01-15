SRC := ./src
TEST := ./test
OBJ := ./obj

ARGS= red.jpl
EXE := jplc
TESTEXE := test_jplc
CFLAGS := -Wall -g -O0
$(CC)= clang

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
LIBS :=

TESTSRCS := $(wildcard $(TEST)/*.c)
TESTOBJS := $(patsubst $(TEST)/%.c, $(OBJ)/%.o, $(TESTSRCS))
LIBS := -lcheck

.PHONY: all clean

# Compiler rules
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS)  $(OBJS)  $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJ) $(SRC): 
	mkdir $@

run: $(EXE)
	$(EXE) $(ARGS)

# Testing rules
test: $(TESTEXE)

$(TESTEXE): $(TESTOBJS) $(OBJS)
	$(CC) $(CFLAGS) $(TESTLIBS)  $(TESTOBJS)  $(OBJS) $^ -o $@

$(OBJ)/%.o: $(TEST)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# General rules
clean:
	rm -rf $(OBJ) $(EXE) $(TESTEXE)
