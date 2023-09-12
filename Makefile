### OpenTDM Makefile ###

-include .config

ifndef CPU
    CPU := $(shell uname -m | sed -e s/i.86/i386/ -e s/amd64/x86_64/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)
endif

ifndef REV
    REV := $(shell git rev-list HEAD | wc -l | tr -d " ")
endif

ifndef VER
    VER := r$(REV)~$(shell git rev-parse --short HEAD)
endif

CC ?= gcc
WINDRES ?= windres
STRIP ?= strip
RM ?= rm -f

CFLAGS ?= -O2 -fno-strict-aliasing -g -Wall -Wno-attribute-warning -Wno-unused-but-set-variable -MMD $(INCLUDES)
LDFLAGS ?= -shared
LIBS ?=

ifdef CONFIG_WINDOWS
	CC = i686-w64-mingw32-gcc
	WINDRES = i686-w64-mingw32-windres
	CFLAGS += -Duint32_t=int -Duint8_t="unsigned char"
    LDFLAGS += -mconsole
    LDFLAGS += -Wl,--nxcompat,--dynamicbase
    CURL_CFLAGS = -static -static-libgcc -DCURL_STATIC
    CFLAGS += -DHAVE_CURL $(CURL_CFLAGS) -DCURL_STATICLIB \
              -Ideps/win32/include
    LIBS += deps/win32/lib/libcurl.a \
            deps/win32/lib/libcrypto.a \
            deps/win32/lib/libcrypt32.a \
            deps/win32/lib/libz.a \
            deps/win32/lib/libssl.a \
            deps/win32/lib/libssh2.a \
            deps/win32/lib/libidn.a \
            deps/win32/lib/libiconv.a \
            deps/win32/lib/libintl.a \
            deps/win32/lib/libwldap32.a \
            deps/win32/lib/libgdi32.a \
            deps/win32/lib/libcrypto.a \
            deps/win32/lib/libcrypt32.a \
            deps/win32/lib/libssp.a \
            deps/win32/lib/libidn2.a \
            deps/win32/lib/libpthread.a \
            -static -static-libgcc
else
    CFLAGS += -fPIC -fvisibility=hidden
    CURL_CFLAGS ?= -Ideps/$(CPU)/curl/include
    CFLAGS += -DHAVE_CURL $(CURL_CFLAGS) -DCURL_STATICLIB \
              -Ideps/$(CPU)/openssl/include \
              -Ideps/$(CPU)/zlib/include
    LIBS +=   deps/$(CPU)/curl/lib/libcurl.a \
              deps/$(CPU)/openssl/lib/libssl.a \
              deps/$(CPU)/openssl/lib/libcrypto.a \
              deps/$(CPU)/zlib/lib/libz.a
endif

CFLAGS += -DC_ONLY
CFLAGS += -DOPENTDM_VERSION='"$(VER)"' -DOPENTDM_REVISION=$(REV)
RCFLAGS += -DOPENTDM_VERSION='\"$(VER)\"' -DOPENTDM_REVISION=$(REV)


OBJS := g_chase.o g_cmds.o g_combat.o g_func.o g_items.o g_main.o g_misc.o \
g_phys.o g_save.o g_spawn.o g_svcmds.o g_target.o g_tdm_client.o g_tdm_cmds.o \
g_tdm_core.o g_tdm_curl.o g_tdm_macros.o g_tdm_stats.o g_tdm_vote.o g_trigger.o \
g_utils.o g_weapon.o mt19937.o p_client.o p_hud.o p_menu.o p_view.o p_weapon.o \
g_tdm_votemenu.o q_shared.o

ifdef CONFIG_WINDOWS
    CPU := x86
    OBJS += sys_win32.o
    OBJS += opentdm.o
    LIBS += -lws2_32
    TARGET ?= game$(CPU)-opentdm-$(VER).dll
    
else
    OBJS += sys_linux.o
    LIBS += -lm
    TARGET ?= game$(CPU)-opentdm-$(VER).so
endif

ifdef CONFIG_32BIT
	CFLAGS += -m32
	LDFLAGS += -m32
	CPU = x86
	TARGET ?= game$(CPU)-opentdm-$(VER).so
endif

all: $(TARGET)

default: all

.PHONY: all default clean strip

# Define V=1 to show command line.
ifdef V
    Q :=
    E := @true
else
    Q := @
    E := @echo
endif

-include $(OBJS:.o=.d)

%.o: %.c
	$(E) [CC] $@
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.rc
	$(E) [RC] $@
	$(Q)$(WINDRES) $(RCFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(E) [LD] $@
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(E) [CLEAN]
	$(Q)$(RM) *.o *.d $(TARGET)

strip: $(TARGET)
	$(E) [STRIP]
	$(Q)$(STRIP) $(TARGET)

