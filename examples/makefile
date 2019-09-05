# Automatically detect all the examples:
SRCFILES = $(wildcard src/*.f90)
OBJFILES = $(patsubst src/%.f90, obj/%.o, $(SRCFILES))
BINFILES = $(patsubst src/%.f90, bin/%, $(SRCFILES))

.PHONY: all clean test_make

.PRECIOUS: $(OBJFILES)
# The above line prevents .o files from being automatically deleted

all: $(BINFILES)

obj/%.o: src/%.f90
	$(FC) $(EXTRA_COMPILE_FLAGS) -I ../obj -c $^ -o $@

bin/%: obj/%.o ../lib/libmango.a
	$(FC) -o $@ $< ../lib/libmango.a $(EXTRA_LINK_FLAGS)

clean:
	rm -f obj/* bin/* src/*~ *~

test_make:
	@echo SRCFILES is $(SRCFILES)
	@echo OBJFILES is $(OBJFILES)
	@echo BINFILES is $(BINFILES)
	@echo HOSTNAME is $(HOSTNAME)
	@echo FC is $(FC)
	@echo EXTRA_COMPILE_FLAGS is $(EXTRA_COMPILE_FLAGS)
	@echo EXTRA_LINK_FLAGS is $(EXTRA_LINK_FLAGS)
