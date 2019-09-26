.PHONY: all clean examples test retest test_make

all: lib/libmango.a examples
#	cp obj/mango.mod* include
#	cp obj/mango.MOD* include

include makefile.system-dependent

ifeq ($(MANGO_PETSC_AVAILABLE),T)
  EXTRA_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
  EXTRA_LINK_FLAGS += $(PETSC_LIB)
else ifneq ($(MANGO_PETSC_AVAILABLE),F)
  $(error MANGO_PETSC_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_HOPSPACK_AVAILABLE),T)
  EXTRA_COMPILE_FLAGS += -DMANGO_HOPSPACK_AVAILABLE
else ifneq ($(MANGO_HOPSPACK_AVAILABLE),F)
  $(error MANGO_HOPSPACK_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_DAKOTA_AVAILABLE),T)
  EXTRA_COMPILE_FLAGS += -DMANGO_DAKOTA_AVAILABLE
else ifneq ($(MANGO_DAKOTA_AVAILABLE),F)
  $(error MANGO_DAKOTA_AVAILABLE must be set to either T or F (case-sensitive))
endif

ifeq ($(MANGO_NLOPT_AVAILABLE),T)
  EXTRA_COMPILE_FLAGS += -DMANGO_NLOPT_AVAILABLE
else ifneq ($(MANGO_NLOPT_AVAILABLE),F)
  $(error MANGO_NLOPT_AVAILABLE must be set to either T or F (case-sensitive))
endif

# Put .mod files in the ./obj/ directory:
EXTRA_COMPILE_FLAGS += -J obj -I obj

export

# Automatically detect all the examples:
SRC_FILES = $(wildcard src/*.F90)
OBJ_FILES = $(patsubst src/%.F90, obj/%.o, $(SRC_FILES))

include makefile.dependencies

obj/%.o: src/%.F90
	$(FC) $(EXTRA_COMPILE_FLAGS) -c $< -o $@
#	$(FC) $(EXTRA_COMPILE_FLAGS) -c $<

lib/libmango.a: $(OBJ_FILES)
	ar rcs lib/libmango.a $(OBJ_FILES)
	cp obj/mango.* include
	rm include/mango.o

examples: lib/libmango.a
	$(MAKE) -C examples	

clean::
	rm -f obj/* include/* lib/* *~ src/*~
	$(MAKE) -C examples clean

test: $(TARGET)
	@echo "Beginning functional tests." && cd examples/bin && export MANGO_RETEST=no && ./run_examples

retest: $(TARGET)
	@echo "Testing existing output files for examples without re-running then." && cd examples/bin && export MANGO_RETEST=yes && ./run_examples

test_make:
	@echo OBJ_FILES is $(OBJ_FILES)
	@echo HOSTNAME is $(HOSTNAME)
	@echo FC is $(FC)
	@echo FLINKER is $(FLINKER)
	@echo MANGO_PETSC_AVAILABLE is $(MANGO_PETSC_AVAILABLE)
	@echo MANGO_HOPSPACK_AVAILABLE is $(MANGO_HOPSPACK_AVAILABLE)
	@echo MANGO_DAKOTA_AVAILABLE is $(MANGO_DAKOTA_AVAILABLE)
	@echo MANGO_NLOPT_AVAILABLE is $(MANGO_NLOPT_AVAILABLE)
	@echo EXTRA_COMPILE_FLAGS is $(EXTRA_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
