# Automatically detect all the examples:
CPP_SRC_FILES = $(wildcard src/*.cpp)
CPP_OBJ_FILES = $(patsubst src/%.cpp, obj/%.cpp.o, $(CPP_SRC_FILES))
C_SRC_FILES = $(wildcard src/*.c)
C_OBJ_FILES = $(patsubst src/%.c, obj/%.c.o, $(C_SRC_FILES))

CXX = mpicxx
CC = mpicc
CLINKER = mpicxx
EXTRA_C_COMPILE_FLAGS =
EXTRA_CPP_COMPILE_FLAGS =
EXTRA_LINK_FLAGS = 

.PHONY: all clean test_make

#.SECONDARY:
# The above line prevents .co and .fo files from being automatically deleted

.PRECIOUS: $(CPP_OBJ_FILES) $(C_OBJ_FILES)
# The above line prevents .o files from being automatically deleted

all: HOPSPACK_main_mpi

obj/%.cpp.o: src/%.cpp
	$(CXX) $(EXTRA_CPP_COMPILE_FLAGS) -I src -c $^ -o $@

obj/%.c.o: src/%.c
	$(CC) $(EXTRA_C_COMPILE_FLAGS) -I src -c $^ -o $@

HOPSPACK_main_mpi: $(C_OBJ_FILES) $(CPP_OBJ_FILES)
	$(CLINKER) -o $@ $^  $(EXTRA_LINK_FLAGS)

clean:
	rm -f obj/* bin/* src/*~ *~

test_make:
	@echo CPP_SRC_FILES is $(CPP_SRC_FILES)
	@echo CPP_OBJ_FILES is $(CPP_OBJ_FILES)
	@echo C_SRC_FILES is $(C_SRC_FILES)
	@echo C_OBJ_FILES is $(C_OBJ_FILES)
	@echo HOSTNAME is $(HOSTNAME)
	@echo CXX is $(CXX)
	@echo CC is $(CC)
	@echo CLINKER is $(CLINKER)
	@echo EXTRA_C_COMPILE_FLAGS is $(EXTRA_C_COMPILE_FLAGS)
	@echo EXTRA_CPP_COMPILE_FLAGS is $(EXTRA_CPP_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
