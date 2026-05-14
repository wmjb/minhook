WINDRES:=$(CROSS_PREFIX)windres
DLLTOOL:=$(CROSS_PREFIX)dlltool
AR:=$(CROSS_PREFIX)ar
CC:=$(CROSS_PREFIX)gcc
CCLD:=$(CC)

# Detect architecture from CROSS_PREFIX or compiler
ARCH:=$(shell $(CC) -dumpmachine)

# Select correct HDE backend
ifeq ($(findstring x86_64,$(ARCH)),x86_64)
    HDE:=src/hde/hde64.c
else ifeq ($(findstring i686,$(ARCH)),i686)
    HDE:=src/hde/hde32.c
else ifeq ($(findstring arm,$(ARCH)),arm)
    HDE:=src/hde/hdearm32.c
else
    $(error Unsupported architecture: $(ARCH))
endif

SRCS:=src/buffer.c src/hook.c src/trampoline.c $(HDE)
OBJS:=$(SRCS:%.c=%.o)
DEPS:=$(SRCS:%.c=%.d)

INCS:=-Isrc -Iinclude

# Remove x86-only flag (-masm=intel)
CFLAGS:=-Wall -Werror -std=c11
LDFLAGS:=-Wl,-enable-stdcall-fixup -s -static-libgcc

all: MinHook.dll libMinHook.dll.a libMinHook.a

-include $(DEPS)

libMinHook.a: $(OBJS)
    $(AR) r $@ $^

libMinHook.dll.a: MinHook.dll dll_resources/MinHook.def
    $(DLLTOOL) --dllname MinHook.dll --input-def dll_resources/MinHook.def --output-lib $@

MinHook.dll: $(OBJS) dll_resources/MinHook.res dll_resources/MinHook.def
    $(CCLD) -o $@ -shared $(LDFLAGS) $^

.rc.res:
    $(WINDRES) -o $@ --input-format=rc --output-format=coff $<

.c.o:
    $(CC) -o $@ -c -MMD -MP $(INCS) $(CFLAGS) $<

clean:
    rm -f $(OBJS) $(DEPS) MinHook.dll libMinHook.dll.a libMinHook.a dll_resources/MinHook.res

.PHONY: clean
.SUFFIXES: .rc .res
