TEST= ./examples/red.jpl
FLAGS = -p
CFLAGS := -lm -Wall -O1 -g
INCLUDES := -Ihdr
$(CC)= clang
SRC := $(shell find ./ -wholename "./src/*.c")
OBJ := $(patsubst ./src/%, ./obj/%, $(patsubst %.c, %.o, $(SRC)))
DIR := obj obj/parser obj/lexer obj/helper obj/typechecker obj/ir
LIBS :=

compile: compiler
compiler: $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

./obj/%.o: ./src/%.c | $(DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

$(DIR):
	mkdir $(DIR)

run: compiler
	./compiler $(FLAGS) $(TEST)

clean:
	rm -rf compiler $(DIR) vgcore.*
