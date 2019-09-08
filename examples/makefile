# Automatically detect all the examples:
SRC_FILES = $(wildcard src/*.F90)
OBJ_FILES = $(patsubst src/%.F90, obj/%.o, $(SRC_FILES))
BIN_FILES = $(patsubst src/%.F90, bin/%, $(SRC_FILES))

.PHONY: all clean test_make

.PRECIOUS: $(OBJ_FILES)
# The above line prevents .o files from being automatically deleted

all: $(BIN_FILES)

obj/%.o: src/%.F90
	$(FC) $(EXTRA_COMPILE_FLAGS) -I ../include -c $^ -o $@

bin/%: obj/%.o ../lib/libmango.a
	$(FLINKER) -o $@ $< ../lib/libmango.a $(EXTRA_LINK_FLAGS)

clean:
	rm -f obj/* bin/* src/*~ *~

test_make:
	@echo SRC_FILES is $(SRC_FILES)
	@echo OBJ_FILES is $(OBJ_FILES)
	@echo BIN_FILES is $(BIN_FILES)
	@echo HOSTNAME is $(HOSTNAME)
	@echo FC is $(FC)
	@echo FLINKER is $(FLINKER)
	@echo EXTRA_COMPILE_FLAGS is $(EXTRA_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
