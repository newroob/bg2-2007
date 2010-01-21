#
# SDK Makefile for x86 Linux
#
#

#############################################################################
# Developer configurable items
#############################################################################

# the name of the mod binary (_i486.so is appended to the end)
NAME=server

# the location of the vcproj that builds the mod
MOD_PROJ=../../game/server/server_hl2mp-2005.vcproj
# the name of the mod configuration (typically <proj name>_<build type><build target>)
MOD_CONFIG="Server(HL2MP)_ReleaseWin32"

# the directory the base binaries (tier0_i486.so, etc) are located
GAME_DIR=$(SOURCE_DIR)/lib/linux/

# the path to your mods directory
# set this so that 'make install' or 'make installrelease' will copy your binary over automatically.
MOD_DIR=$(GAME_DIR)/bg2-svn

# compiler options 3.4.6 or 4.2.4 will work nicely.
CC=gcc-4.2
CPLUS=g++-4.2
CLINK=gcc-4.2
CPP_LIB="/usr/lib/gcc/i486-linux-gnu/4.2.4/libstdc++.a /usr/lib/gcc/i486-linux-gnu/4.2.4/libgcc_eh.a"

# GCC 4.2.4 optimization flags, if you're using anything below, don't use these!
OPTFLAGS= -O2

# put any compiler flags you want passed here
USER_CFLAGS=

# Link Libraries
LIBFILES= \
  $(SOURCE_DIR)/lib/linux/tier1_486.a \
  $(SOURCE_DIR)/lib/linux/choreoobjects_486.a \
  $(SOURCE_DIR)/lib/linux/particles_486.a \
  $(SOURCE_DIR)/lib/linux/dmxloader_486.a \
  tier0_i486.so \
  vstdlib_i486.so \
  steam_api_i486.so \
  $(SOURCE_DIR)/lib/linux/tier3_486.a \
  $(SOURCE_DIR)/lib/linux/tier2_486.a \
  $(SOURCE_DIR)/lib/linux/tier1_486.a \
  $(SOURCE_DIR)/lib/linux/mathlib_486.a \


# link flags for your mod, make sure to include any special libraries here
#NOTE: YES we want to include the lib files 2 times. We've run into problems with the 1-pass linker not bringing in symbols it should.
LDFLAGS="-lm -ldl $(LIBFILES) $(LIBFILES) -s"

# XERCES 2.6.0 or above ( http://xml.apache.org/xerces-c/ ) is used by the vcproj to makefile converter
# it must be installed before being able to run this makefile
# if you have xerces installed already you should be able to use the two lines below
XERCES_INC_DIR=/usr/include
XERCES_LIB_DIR=/usr/lib


#############################################################################
# Things below here shouldn't need to be altered
#############################################################################
MAKE=make

# the dir we want to put binaries we build into
BUILD_DIR=.
# the place to put object files
BUILD_OBJ_DIR=$(BUILD_DIR)/obj

# the location of the source code
SOURCE_DIR=../..

# the CPU target for the build, must be i486 for now
ARCH=i486
ARCH_CFLAGS=-mtune=pentium3 -march=pentium3 -pipe

# -fpermissive is so gcc 3.4.x doesn't complain about some template stuff
BASE_CFLAGS=-DVPROF_LEVEL=1 -DSWDS -D_LINUX -DLINUX -DNDEBUG -fpermissive -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared -Wl,-Map,$@_map.txt -Wl

#flags passed to the compiler
CFLAGS="$(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(OPTFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -Ufopen=dont_use_fopen -UPROTECTED_THINGS_ENABLE"

# define list passed to make for the sub makefile
BASE_DEFINES=CC=$(CC) CPLUS=$(CPLUS) CPP_LIB=$(CPP_LIB) \
	BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) \
	SOURCE_DIR=$(SOURCE_DIR) SHLIBLDFLAGS=$(SHLIBLDFLAGS) SHLIBEXT=$(SHLIBEXT) \
	CLINK=$(CLINK) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) \
	ARCH=$(ARCH) GAME_DIR=$(GAME_DIR) MOD_CONFIG=$(MOD_CONFIG) NAME=$(NAME) \
	XERCES_INC_DIR=$(XERCES_INC_DIR) XERCES_LIB_DIR=$(XERCES_LIB_DIR)

# Project Makefile
MAKE_MOD=Makefile.server
MAKE_VCPM=Makefile.vcpm
MAKE_PLUGIN=Makefile.plugin

ll: check vcpm server

check:
	if [ -z "$(CC)" ]; then echo "Compiler not defined."; exit; fi
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi
	cd $(BUILD_DIR)

vcpm:
	$(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES)

server: vcpm
	if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)tier0_i486.so .; fi
	if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)vstdlib_i486.so .; fi
	if [ ! -f "steam_api_i486.so" ]; then ln -s $(GAME_DIR)steam_api_i486.so .; fi
# When running over samba we need to copy the files because symlinking isn't possible.
#	cp -f $(GAME_DIR)tier0_i486.so .
#	cp -f $(GAME_DIR)vstdlib_i486.so .
#	cp -f $(GAME_DIR)steam_api_i486.so .

	./vcpm $(MOD_PROJ)
	$(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES)

plugin:
	$(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES)

clean:
	 $(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES) clean

install:
	cp -f $(NAME)_$(ARCH).so $(MOD_DIR)/bin/$(NAME)_$(ARCH).so

installrelease:
	cp -f $(NAME)_$(ARCH).so $(MOD_DIR)/bin/$(NAME)_$(ARCH).so
	strip $(MOD_DIR)/bin/$(NAME)_$(ARCH).so
