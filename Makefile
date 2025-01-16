SRC := ./src
BIN := ./
OBJ := ./obj
TEST= ./examples/red.jpl
EXE := $(BIN)compiler
CFLAGS := -Wall -Wpedantic -O3 -g
$(CC)= clang
SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
LIBS :=
.PHONY: all clean
compile: $(EXE)
$(EXE): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $(LIBS)  $^ -o $@
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ) $(SRC) $(BIN): 
	mkdir $@
run: $(EXE)
	$(EXE) $(TEST)
clean:
	rm -rf $(OBJ) $(EXE)
