# Copyright 2019, University of Maryland and the MANGO development team.
#
# This file is part of MANGO.
#
# MANGO is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# MANGO is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with MANGO.  If not, see
# <https://www.gnu.org/licenses/>.

.PHONY: all clean examples test test_make unit_tests

all: lib/libmango.a examples examples/packages_available
#	cp obj/mango.mod* include
#	cp obj/mango.MOD* include

include makefile.system-dependent

ifeq ($(MANGO_PETSC_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
  EXTRA_LINK_FLAGS += $(PETSC_LIB)
  MANGO_AVAILABLE_PACKAGES += petsc
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
  MANGO_AVAILABLE_PACKAGES += hopspack
else ifneq ($(MANGO_HOPSPACK_AVAILABLE),F)
  $(error MANGO_HOPSPACK_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_NLOPT_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_NLOPT_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_NLOPT_AVAILABLE
  MANGO_AVAILABLE_PACKAGES += nlopt
else ifneq ($(MANGO_NLOPT_AVAILABLE),F)
  $(error MANGO_NLOPT_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_GSL_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_GSL_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_GSL_AVAILABLE
  MANGO_AVAILABLE_PACKAGES += gsl
else ifneq ($(MANGO_GSL_AVAILABLE),F)
  $(error MANGO_GSL_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_DAKOTA_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_DAKOTA_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_DAKOTA_AVAILABLE
  MANGO_AVAILABLE_PACKAGES += dakota
else ifneq ($(MANGO_DAKOTA_AVAILABLE),F)
  $(error MANGO_DAKOTA_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_EIGEN_AVAILABLE),T)
  EXTRA_C_COMPILE_FLAGS += -DMANGO_EIGEN_AVAILABLE
  EXTRA_F_COMPILE_FLAGS += -DMANGO_EIGEN_AVAILABLE
  MANGO_AVAILABLE_PACKAGES += mango
else ifneq ($(MANGO_EIGEN_AVAILABLE),F)
  $(error MANGO_EIGEN_AVAILABLE must be set to either T or F (case-sensitive))
endif

# Put .mod files in the ./obj/ directory:
EXTRA_C_COMPILE_FLAGS += -J obj -I obj -I include
EXTRA_F_COMPILE_FLAGS += -J obj -I obj

export

CXX = $(CC)

# Automatically detect all the source files
F_SRC_FILES = $(wildcard src/api/*.F90)
F_OBJ_FILES = $(patsubst src/api/%.F90, obj/%.f.o, $(F_SRC_FILES))
CPP_SRC_FILES = $(wildcard src/api/*.cpp)
CPP_OBJ_FILES = $(patsubst src/api/%.cpp, obj/%.cpp.o, $(CPP_SRC_FILES))
ALGORITHM_SRC_FILES = $(wildcard src/algorithms/*.cpp)
ALGORITHM_OBJ_FILES = $(patsubst src/algorithms/%.cpp, obj/%.cpp.o, $(ALGORITHM_SRC_FILES))
HOPSPACK_CPP_OBJ_FILES = $(patsubst external_packages/hopspack/src/%.cpp, obj/%.cpp.o, $(HOPSPACK_CPP_SRC_FILES))
HOPSPACK_C_OBJ_FILES   = $(patsubst external_packages/hopspack/src/%.c,   obj/%.c.o,   $(HOPSPACK_C_SRC_FILES))
HOPSPACK_HEADERS = $(wildcard external_packages/hopspack/src/*.h) $(wildcard external_packages/hopspack/src/*.hpp)
HEADER_FILES = $(wildcard src/api/*.hpp) $(wildcard src/algorithms/*.hpp)

TEST_SRC_FILES = $(wildcard src/api/tests/*.cpp)
TEST_OBJ_FILES = $(patsubst src/api/tests/%.cpp, obj/%.cpp.o, $(TEST_SRC_FILES))
ALGORITHM_TEST_SRC_FILES = $(wildcard src/algorithms/tests/*.cpp)
ALGORITHM_TEST_OBJ_FILES = $(patsubst src/algorithms/tests/%.cpp, obj/%.cpp.o, $(ALGORITHM_TEST_SRC_FILES))

include makefile.dependencies

# For info about the "Static pattern rules" below, see e.g.
# https://www.gnu.org/savannah-checkouts/gnu/make/manual/html_node/Static-Usage.html
# https://stackoverflow.com/questions/4320416/how-to-use-a-variable-list-as-a-target-in-a-makefile

# For now, I'll make all the .o files depend on _all_ the .hpp files. This causes some unnecessary recompiling when .hpp files change, but it eliminates the need for a detailed list of dependencies.
$(F_OBJ_FILES): obj/%.f.o: src/api/%.F90
	$(FC) $(EXTRA_F_COMPILE_FLAGS) -c $< -o $@

$(CPP_OBJ_FILES): obj/%.cpp.o: src/api/%.cpp $(HEADER_FILES)
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -I src/api -c $< -o $@

$(ALGORITHM_OBJ_FILES): obj/%.cpp.o: src/algorithms/%.cpp $(HEADER_FILES)
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -I src/api -c $< -o $@

$(TEST_OBJ_FILES): obj/%.cpp.o: src/api/tests/%.cpp $(HEADER_FILES)
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -I external_packages/catch2 -I src/api -c $< -o $@

$(ALGORITHM_TEST_OBJ_FILES): obj/%.cpp.o: src/algorithms/tests/%.cpp $(HEADER_FILES)
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -I external_packages/catch2 -I src/api -I src/algorithms -c $< -o $@

# Each hopspack file does not actually depend on _all_ the hopspack headers, but it is easier to impose a dependency on all the headers than the more precise dependencies.
# Similarly, only the modified hopspack source files depend on mango.hpp, but it is easier to make the dependency apply to all hopspack files here.
$(HOPSPACK_CPP_OBJ_FILES): obj/%.cpp.o: external_packages/hopspack/src/%.cpp $(HOPSPACK_HEADERS) $(HEADER_FILES)
	$(CXX) $(EXTRA_C_COMPILE_FLAGS) -I src/api -c $< -o $@

$(HOPSPACK_C_OBJ_FILES): obj/%.c.o: external_packages/hopspack/src/%.c $(HOPSPACK_HEADERS)
	$(CC) $(EXTRA_C_COMPILE_FLAGS) -c $< -o $@


lib/libmango.a: $(F_OBJ_FILES) $(CPP_OBJ_FILES) $(ALGORITHM_OBJ_FILES) $(HOPSPACK_CPP_OBJ_FILES) $(HOPSPACK_C_OBJ_FILES)
	ar rcs lib/libmango.a $(F_OBJ_FILES) $(CPP_OBJ_FILES) $(ALGORITHM_OBJ_FILES) $(HOPSPACK_CPP_OBJ_FILES) $(HOPSPACK_C_OBJ_FILES)
	cp obj/mango_mod.* include
#	rm include/mango.o
#	cp obj/mango.* include
# May need to copy any fortran module files from obj to include here?

examples: lib/libmango.a
	$(MAKE) -C examples	

examples/packages_available:
	@echo $(MANGO_AVAILABLE_PACKAGES) > examples/packages_available

clean:
	rm -f obj/* include/*.mod include/*.MOD include/*.Mod lib/* *~ src/*~ src/api/*~ examples/packages_available tests/unit_tests
	$(MAKE) -C examples clean

tests/unit_tests: $(TEST_OBJ_FILES) $(ALGORITHM_TEST_OBJ_FILES) lib/libmango.a
	$(CLINKER) -o $@ $^ $(EXTRA_C_LINK_FLAGS)
# Also we have a unit_tests target without "tests/" in front so you don't have to type tests/unit_tests all the time.
unit_tests: tests/unit_tests

test: $(TARGET) unit_tests
	cd tests && ./run_mpi_unit_tests
	@echo "Beginning functional tests." && cd examples && export MANGO_RETEST=no && ./run_examples

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
	@echo MANGO_NLOPT_AVAILABLE is $(MANGO_NLOPT_AVAILABLE)
	@echo MANGO_GSL_AVAILABLE is $(MANGO_GSL_AVAILABLE)
	@echo MANGO_DAKOTA_AVAILABLE is $(MANGO_DAKOTA_AVAILABLE)
	@echo MANGO_EIGEN_AVAILABLE is $(MANGO_EIGEN_AVAILABLE)
	@echo MANGO_AVAILABLE_PACKAGES is $(MANGO_AVAILABLE_PACKAGES)
	@echo EXTRA_F_COMPILE_FLAGS is $(EXTRA_F_COMPILE_FLAGS)
	@echo EXTRA_C_COMPILE_FLAGS is $(EXTRA_C_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
	@echo EXTRA_F_LINK_FLAGS is $(EXTRA_F_LINK_FLAGS)
	@echo EXTRA_C_LINK_FLAGS is $(EXTRA_C_LINK_FLAGS)
	@echo F_SRC_FILES is $(F_SRC_FILES)
	@echo F_OBJ_FILES is $(F_OBJ_FILES)
	@echo CPP_SRC_FILES is $(CPP_SRC_FILES)
	@echo CPP_OBJ_FILES is $(CPP_OBJ_FILES)
	@echo ALGORITHM_SRC_FILES is $(ALGORITHM_SRC_FILES)
	@echo ALGORITHM_OBJ_FILES is $(ALGORITHM_OBJ_FILES)
	@echo HEADER_FILES is $(HEADER_FILES)
	@echo TEST_SRC_FILES is $(TEST_SRC_FILES)
	@echo TEST_OBJ_FILES is $(TEST_OBJ_FILES)
	@echo ALGORITHM_TEST_SRC_FILES is $(ALGORITHM_TEST_SRC_FILES)
	@echo ALGORITHM_TEST_OBJ_FILES is $(ALGORITHM_TEST_OBJ_FILES)
	@echo HOPSPACK_CPP_SRC_FILES is $(HOPSPACK_CPP_SRC_FILES)
	@echo HOPSPACK_CPP_SRC_FILES is $(HOPSPACK_C_SRC_FILES)
	@echo HOPSPACK_C_OBJ_FILES is $(HOPSPACK_CPP_OBJ_FILES)
	@echo HOPSPACK_C_OBJ_FILES is $(HOPSPACK_C_OBJ_FILES)
	@echo HOPSPACK_HEADERS is $(HOPSPACK_HEADERS)
	@echo "********************* Below are the results of examples/make test_make **********************"
	$(MAKE) -C examples test_make

