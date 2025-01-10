SRC := ./src
BIN := ./bin
REF := ./ref

SRCS := $(wildcard $(SRC)/*.jpl)
BINS := $(patsubst $(SRC)/%.jpl, $(BIN)/%, $(SRCS))

SUB := test/subtract

all: $(BINS)
$(BIN)/%: $(SRC)/%.jpl
	@mkdir -p $(BIN)
	./jplc $^ -o $@

run/%: $(BIN)/%
	$^


test: all $(patsubst $(BIN)/%, test/%, $(BINS))
test/%: $(BIN)/%
	$(eval IMG := $(patsubst test/%, %.png, $@))
	@if [ "$(IMG)" == "subtract.png" ]; then \
		OUTPUT="$$($(BIN)/subtract)"; \
		if [[ $$OUTPUT == "999999999.900000" ]]; then \
			echo "subtract: PASS"; \
		else \
			echo "subtract: FAIL"; \
		fi \
	else \
		$^ $(REF)/sample.png && \
		cmp -s $(IMG) $(REF)/$(IMG); \
		RETVAL=$$?; \
		if [ $$RETVAL -eq 0 ]; then \
			echo "$(IMG): PASS"; \
		else \
			echo "$(IMG): FAIL"; \
		fi \
	fi

clean:
	rm -rf $(BIN)
	rm -rf ./*.png
