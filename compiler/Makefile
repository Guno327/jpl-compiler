SRC := ./src
BIN := ./
OBJ := ./obj
ARGS= red.jpl
EXE := $(BIN)jplc
CFLAGS := -Wall -g -O0
$(CC)= clang
SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
LIBS :=
.PHONY: all clean
all: $(EXE)
$(EXE): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $(LIBS)  $^ -o $@
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ) $(SRC) $(BIN): 
	mkdir $@
run: $(EXE)
	$(EXE) $(ARGS)
clean:
	rm -rf $(OBJ) $(EXE)
