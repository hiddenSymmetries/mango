ifdef NERSC_HOST
        HOSTNAME = $(NERSC_HOST)
else
        HOSTNAME?="laptop"
endif

ifeq ($(HOSTNAME),edison)
	FC = ftn
	## NERSC documentation recommends against specifying -O3
	## -mkl MUST APPEAR AT THE END!!
	EXTRA_COMPILE_FLAGS = -openmp -mkl
	EXTRA_LINK_FLAGS =  -openmp -mkl -Wl,-ydgemm_
	# Above, the link flag "-Wl,-ydgemm_" causes the linker to report which version of DGEMM (the BLAS3 matrix-matrix-multiplication subroutine) is used.
	# For batch systems, set the following variable to the command used to run jobs. This variable is used by 'make test'.
	REGCOIL_COMMAND_TO_SUBMIT_JOB = srun -n 1 -c 24

else ifeq ($(HOSTNAME),cori)
	FC = ftn
	## NERSC documentation recommends against specifying -O3
	## -mkl MUST APPEAR AT THE END!!
	EXTRA_COMPILE_FLAGS = -qopenmp -mkl
	EXTRA_LINK_FLAGS =  -qopenmp -mkl -Wl,-ydgemm_
	# Above, the link flag "-Wl,-ydgemm_" causes the linker to report which version of DGEMM (the BLAS3 matrix-matrix-multiplication subroutine) is used.
	# For batch systems, set the following variable to the command used to run jobs. This variable is used by 'make test'.
	REGCOIL_COMMAND_TO_SUBMIT_JOB = srun -n 1 -c 32

else ifeq ($(HOSTNAME),pppl)
	NETCDF_F = /usr/pppl/gcc/6.1-pkgs/netcdf-fortran-4.4.4
	NETCDF_C = /usr/pppl/gcc/6.1-pkgs/netcdf-c-4.4.1
	FC = mpifort
	EXTRA_COMPILE_FLAGS = -O2 -ffree-line-length-none -fexternal-blas -fopenmp -I$(NETCDF_F)/include -I$(NETCDF_C)/include
	EXTRA_LINK_FLAGS =  -fopenmp -L$(ACML_HOME)/lib -lacml -L$(NETCDF_C)/lib -lnetcdf -L$(NETCDF_F)/lib -lnetcdff
	REGCOIL_COMMAND_TO_SUBMIT_JOB = srun -n 1 -c 32
        LIBSTELL_DIR=/u/slazerso/bin/libstell_dir
        LIBSTELL_FOR_REGCOIL=/u/slazerso/bin/libstell.a
	REGCOIL_COMMAND_TO_SUBMIT_JOB = srun -N 1 -n 1 -c 8 -p dawson
else
	FC = mpif90

	MANGO_PETSC_AVAILABLE = T
	MANGO_HOPSPACK_AVAILABLE = F
	MANGO_DAKOTA_AVAILABLE = F
	MANGO_NLOPT_AVAILABLE = F

	#EXTRA_COMPILE_FLAGS = -fopenmp -I/opt/local/include -ffree-line-length-none -cpp
	EXTRA_COMPILE_FLAGS = -fopenmp -I/opt/local/include -ffree-line-length-none -O0 -g
	EXTRA_LINK_FLAGS =  -fopenmp -L/opt/local/lib -lnetcdff  -lnetcdf -framework Accelerate

	# For batch systems, set the following variable to the command used to run jobs. This variable is used by 'make test'.
	REGCOIL_COMMAND_TO_SUBMIT_JOB =
endif


# End of system-dependent variable assignments

ifeq ($(MANGO_PETSC_AVAILABLE),T)
  EXTRA_COMPILE_FLAGS += -DMANGO_PETSC_AVAILABLE
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

.PHONY: all clean examples test retest test_make

all: lib/libmango.a examples
#	cp obj/mango.mod* include
#	cp obj/mango.MOD* include

include makefile.depend

obj/%.o: src/%.f90
	$(FC) $(EXTRA_COMPILE_FLAGS) -c $^ -o $@
#	$(FC) $(EXTRA_COMPILE_FLAGS) -c $<

lib/libmango.a: $(OBJ_FILES)
	ar rcs lib/libmango.a $(OBJ_FILES)

examples: lib/libmango.a
	$(MAKE) -C examples	

clean:
	rm -f obj/* include/* lib/* *~ src/*~
	$(MAKE) -C examples clean

test: $(TARGET)
	@echo "Beginning functional tests." && cd examples && export REGCOIL_RETEST=no && ./runExamples.py

retest: $(TARGET)
	@echo "Testing existing output files for examples without re-running then." && cd examples && export REGCOIL_RETEST=yes && ./runExamples.py

test_make:
	@echo HOSTNAME is $(HOSTNAME)
	@echo FC is $(FC)
	@echo MANGO_PETSC_AVAILABLE is $(MANGO_PETSC_AVAILABLE)
	@echo MANGO_HOPSPACK_AVAILABLE is $(MANGO_HOPSPACK_AVAILABLE)
	@echo MANGO_DAKOTA_AVAILABLE is $(MANGO_DAKOTA_AVAILABLE)
	@echo MANGO_NLOPT_AVAILABLE is $(MANGO_NLOPT_AVAILABLE)
	@echo EXTRA_COMPILE_FLAGS is $(EXTRA_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
