# Automatically detect all the examples:
#F_SRC_FILES = $(wildcard src/*.F90)
F_SRC_FILES= src/nondifferentiable_f.F90
F_OBJ_FILES = $(patsubst src/%.F90, obj/%.f.o, $(F_SRC_FILES))
F_BIN_FILES = $(patsubst src/%.F90, bin/%, $(F_SRC_FILES))
#C_SRC_FILES = $(wildcard src/*.cpp)
C_SRC_FILES = src/nondifferentiable_c.cpp
C_OBJ_FILES = $(patsubst src/%.cpp, obj/%.c.o, $(C_SRC_FILES))
C_BIN_FILES = $(patsubst src/%.cpp, bin/%, $(C_SRC_FILES))

.PHONY: all clean test_make

#.SECONDARY:
# The above line prevents .co and .fo files from being automatically deleted

.PRECIOUS: $(F_OBJ_FILES) $(C_OBJ_FILES)
# The above line prevents .o files from being automatically deleted

all: $(F_BIN_FILES) $(C_BIN_FILES)

obj/%.f.o: src/%.F90
	$(FC) $(EXTRA_F_COMPILE_FLAGS) -I ../include -c $^ -o $@

obj/%.c.o: src/%.cpp
	$(CC) $(EXTRA_C_COMPILE_FLAGS) -I ../include -c $^ -o $@

bin/%: obj/%.c.o ../lib/libmango.a
	$(CLINKER) -o $@ $< ../lib/libmango.a $(EXTRA_C_LINK_FLAGS)

bin/%: obj/%.f.o ../lib/libmango.a
	$(FLINKER) -o $@ $< ../lib/libmango.a $(EXTRA_F_LINK_FLAGS)

clean:
	rm -f obj/* bin/* src/*~ *~

test_make:
	@echo F_SRC_FILES is $(F_SRC_FILES)
	@echo F_OBJ_FILES is $(F_OBJ_FILES)
	@echo F_BIN_FILES is $(F_BIN_FILES)
	@echo C_SRC_FILES is $(C_SRC_FILES)
	@echo C_OBJ_FILES is $(C_OBJ_FILES)
	@echo C_BIN_FILES is $(C_BIN_FILES)
	@echo HOSTNAME is $(HOSTNAME)
	@echo FC is $(FC)
	@echo FLINKER is $(FLINKER)
	@echo CC is $(CC)
	@echo CLINKER is $(CLINKER)
	@echo EXTRA_C_COMPILE_FLAGS is $(EXTRA_C_COMPILE_FLAGS)
	@echo EXTRA_F_COMPILE_FLAGS is $(EXTRA_F_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_C_LINK_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_F_LINK_FLAGS)
