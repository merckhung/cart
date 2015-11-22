CROSS_COMPILE       :=
AS                  :=   $(CROSS_COMPILE)as
AR                  :=   $(CROSS_COMPILE)ar
CC                  :=   $(CROSS_COMPILE)gcc
CPP                 :=   $(CROSS_COMPILE)g++
LD                  :=   $(CROSS_COMPILE)ld
NM                  :=   $(CROSS_COMPILE)nm
OBJCOPY             :=   $(CROSS_COMPILE)objcopy
OBJDUMP             :=   $(CROSS_COMPILE)objdump
RANLIB              :=   $(CROSS_COMPILE)ranlib
READELF             :=   $(CROSS_COMPILE)readelf
SIZE                :=   $(CROSS_COMPILE)size
STRINGS             :=   $(CROSS_COMPILE)strings
STRIP               :=   $(CROSS_COMPILE)strip
CPPLINT							:=	tools/cpplint.py

CPPFLAGS						:=	-Iinclude -I../../libnativehelper/include/nativehelper -Wall -g3 -DCART_DEBUG
LDFLAGS							:=
SRCS								:=	../utils.cc ../elf.cc ../oat.cc ../list.cc ../hash.cc ../heap.cc
SRCS								+=	../class.cc ../net.cc ../zip.cc ../debugger.cc
SRCS								+=	../java_vm_ext.cc ../jni_env_ext.cc ../entry_init_x86.cc

MODULES							:=	unittest oatdump artdump coatgen pool

# CPPLINT
CPPLINT_FILTER := --filter=-whitespace/line_length,-build/include,-readability/function,-readability/streams,-readability/todo,-runtime/references,-runtime/sizeof,-runtime/threadsafe_fn,-runtime/printf
CPPLINT_SRC := $(shell find ./ -name "*.h" -o -name "*.cc")

all: $(MODULES)

unittest:
	@$(CPP) -m32 $(CPPFLAGS) $(LDFLAGS) -o $@ unittest.cc $(SRCS) -lz -lpthread

oatdump: oatdump.o
	@$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

artdump: artdump.o
	@$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

coatgen: coatgen.o
	@$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $< -lz

pool:
	@$(CPP) $(CPPFLAGS) -std=gnu++11 $(LDFLAGS) -o $@ pool.cc decompressed_code.cc

test: clean all
	./oatdump ../samples/test.oat

test1: clean all
	./coatgen ../samples/core.oat

test2:
	./artdump ../samples/core.art

%.o: %.cc
	@$(CPP) $(CPPFLAGS) -c -o $@ $<

cpplint:
	$(CPPLINT) $(CPPLINT_FILTER) $(CPPLINT_SRC)

clean:
	@$(RM) -rf *.o *.coat *.dSYM $(MODULES)

