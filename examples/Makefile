SRC := ./src
REF := ./ref
BIN := ./bin

SRCS := $(wildcard $(SRC)/*.jpl)
BINS := $(patsubst $(SRC)/%.jpl, $(BIN)/%, $(SRCS))

SUB := test/subtract

all: $(BINS)
$(BIN)/%: $(SRC)/%.jpl
	@mkdir -p bin
	./jplc $^ -o $@

run/%: $(BIN)/%
	@$^

test: all $(patsubst $(BIN)/%, test/%, $(BINS))
test/subtract: $(BIN)/subtract
	@OUTPUT="$$($(BIN)/subtract)"; \
	if [[ $$OUTPUT == "999999999.900000" ]]; then \
		printf "%-20s %s\n" "subtract" "PASSED"; \
	else \
		printf "%-20s %s\n" "subtract" "FAILED"; \
	fi

test/red: bin/red run/red cmp/red.png
test/circle: bin/circle run/circle cmp/circle.png
test/invert: bin/invert run/invert cmp/sample-inverted.png
test/sepia: bin/sepia run/sepia cmp/sample-sepia.png
test/blur: bin/blur run/blur cmp/sample-blurred.png

cmp/%: %
	@cmp -s $^ $(REF)/$^; \
	RETVAL=$$?
	@if [[ $$RETVAL -eq 0 ]]; then \
		printf "%-20s %s\n" "$^" "PASSED"; \
	else \
		printf "%-20s %s\n" "$^" "FAILED"; \
	fi

clean:
	rm -rf $(BIN)
	rm -f red.png circle.png sample-inverted.png sample-sepia.png sample-blurred.png
