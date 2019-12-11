.PHONY: all clean examples test retest test_make

all: lib/libmango.a examples
#	cp obj/mango.mod* include
#	cp obj/mango.MOD* include

include makefile.system-dependent

ifeq ($(MANGO_PETSC_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
  EXTRA_LINK_FLAGS += $(PETSC_LIB)
else ifneq ($(MANGO_PETSC_AVAILABLE),F)
  $(error MANGO_PETSC_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_HOPSPACK_AVAILABLE),T)
  # See if hopspack files are present in the expected location
  ifeq (,$(wildcard external_packages/hopspack/src/HOPSPACK_MangoEvaluator.cpp))
    $(error HOPSPACK modified source files are not present in the expected location external_packages/hopspack/src. You probably need to run "cd external_packages; ./install_hopspack.sh")
  endif
  EXTRA_C_COMPILE_FLAGS += -DMANGO_HOPSPACK_AVAILABLE -I external_packages/hopspack/src
  EXTRA_F_COMPILE_FLAGS += -DMANGO_HOPSPACK_AVAILABLE
  HOPSPACK_CPP_SRC_FILES = $(wildcard external_packages/hopspack/src/*.cpp)
  HOPSPACK_C_SRC_FILES   = $(wildcard external_packages/hopspack/src/*.c)
else ifneq ($(MANGO_HOPSPACK_AVAILABLE),F)
  $(error MANGO_HOPSPACK_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_DAKOTA_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_DAKOTA_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_DAKOTA_AVAILABLE
else ifneq ($(MANGO_DAKOTA_AVAILABLE),F)
  $(error MANGO_DAKOTA_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_NLOPT_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_NLOPT_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_NLOPT_AVAILABLE
else ifneq ($(MANGO_NLOPT_AVAILABLE),F)
  $(error MANGO_NLOPT_AVAILABLE must be set to either T or F (case-sensitive))
endif

# Put .mod files in the ./obj/ directory:
EXTRA_C_COMPILE_FLAGS += -J obj -I obj -I include
EXTRA_F_COMPILE_FLAGS += -J obj -I obj

export

CXX = $(CC)

# Automatically detect all the examples:
F_SRC_FILES = $(wildcard src/*.F90)
F_OBJ_FILES = $(patsubst src/%.F90, obj/%.f.o, $(F_SRC_FILES))
CPP_SRC_FILES = $(wildcard src/*.cpp)
CPP_OBJ_FILES = $(patsubst src/%.cpp, obj/%.cpp.o, $(CPP_SRC_FILES))
HOPSPACK_CPP_OBJ_FILES = $(patsubst external_packages/hopspack/src/%.cpp, obj/%.cpp.o, $(HOPSPACK_CPP_SRC_FILES))
HOPSPACK_C_OBJ_FILES   = $(patsubst external_packages/hopspack/src/%.c,   obj/%.c.o,   $(HOPSPACK_C_SRC_FILES))
HOPSPACK_HEADERS = $(wildcard external_packages/hopspack/src/*.h) $(wildcard external_packages/hopspack/src/*.hpp)

include makefile.dependencies

# For info about the "Static pattern rules" below, see e.g.
# https://www.gnu.org/savannah-checkouts/gnu/make/manual/html_node/Static-Usage.html
# https://stackoverflow.com/questions/4320416/how-to-use-a-variable-list-as-a-target-in-a-makefile

$(F_OBJ_FILES): obj/%.f.o: src/%.F90
	$(FC) $(EXTRA_F_COMPILE_FLAGS) -c $< -o $@

$(CPP_OBJ_FILES): obj/%.cpp.o: src/%.cpp include/mango.hpp
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -c $< -o $@

# Each hopspack file does not actually depend on _all_ the hopspack headers, but it is easier to impose a dependency on all the headers than the more precise dependencies.
# Similarly, only the modified hopspack source files depend on mango.hpp, but it is easier to make the dependency apply to all hopspack files here.
$(HOPSPACK_CPP_OBJ_FILES): obj/%.cpp.o: external_packages/hopspack/src/%.cpp $(HOPSPACK_HEADERS) src/mango.hpp
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -c $< -o $@

$(HOPSPACK_C_OBJ_FILES): obj/%.c.o: external_packages/hopspack/src/%.c $(HOPSPACK_HEADERS)
	$(CC) $(EXTRA_C_COMPILE_FLAGS) -c $< -o $@


lib/libmango.a: $(F_OBJ_FILES) $(CPP_OBJ_FILES) $(HOPSPACK_CPP_OBJ_FILES) $(HOPSPACK_C_OBJ_FILES)
	ar rcs lib/libmango.a $(F_OBJ_FILES) $(CPP_OBJ_FILES) $(HOPSPACK_CPP_OBJ_FILES) $(HOPSPACK_C_OBJ_FILES)
	cp obj/mango.* include
#	rm include/mango.o
#	cp obj/mango.* include
# May need to copy any fortran module files from obj to include here?

examples: lib/libmango.a
	$(MAKE) -C examples	

clean:
	rm -f obj/* include/*.mod include/*.MOD include/*.Mod lib/* *~ src/*~
	$(MAKE) -C examples clean

test: $(TARGET)
	@echo "Beginning functional tests." && cd examples && export MANGO_RETEST=no && ./run_examples

retest: $(TARGET)
	@echo "Testing existing output files for examples without re-running then." && cd examples && export MANGO_RETEST=yes && ./run_examples

# This next target is used by examples/run_examples to get MANGO_COMMAND_TO_SUBMIT_JOB when run_examples is run standalone.
print_command_to_submit_job:
	@echo $(MANGO_COMMAND_TO_SUBMIT_JOB)

test_make:
	@echo MANGO_HOST is $(MANGO_HOST)
	@echo HOSTNAME is $(HOSTNAME)
	@echo CC is $(CC)
	@echo CXX is $(CXX)
	@echo FC is $(FC)
	@echo CLINKER is $(CLINKER)
	@echo FLINKER is $(FLINKER)
	@echo MANGO_PETSC_AVAILABLE is $(MANGO_PETSC_AVAILABLE)
	@echo MANGO_HOPSPACK_AVAILABLE is $(MANGO_HOPSPACK_AVAILABLE)
	@echo MANGO_DAKOTA_AVAILABLE is $(MANGO_DAKOTA_AVAILABLE)
	@echo MANGO_NLOPT_AVAILABLE is $(MANGO_NLOPT_AVAILABLE)
	@echo EXTRA_F_COMPILE_FLAGS is $(EXTRA_F_COMPILE_FLAGS)
	@echo EXTRA_C_COMPILE_FLAGS is $(EXTRA_C_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
	@echo EXTRA_F_LINK_FLAGS is $(EXTRA_F_LINK_FLAGS)
	@echo EXTRA_C_LINK_FLAGS is $(EXTRA_C_LINK_FLAGS)
	@echo F_OBJ_FILES is $(F_OBJ_FILES)
	@echo C_OBJ_FILES is $(C_OBJ_FILES)
	@echo HOPSPACK_CPP_SRC_FILES is $(HOPSPACK_CPP_SRC_FILES)
	@echo HOPSPACK_CPP_SRC_FILES is $(HOPSPACK_C_SRC_FILES)
	@echo HOPSPACK_C_OBJ_FILES is $(HOPSPACK_CPP_OBJ_FILES)
	@echo HOPSPACK_C_OBJ_FILES is $(HOPSPACK_C_OBJ_FILES)
	@echo HOPSPACK_HEADERS is $(HOPSPACK_HEADERS)
