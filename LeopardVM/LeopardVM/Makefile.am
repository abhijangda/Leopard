AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS)

bin_PROGRAMS = LeopardVM
LeopardVM_SOURCES = src/classinfo.cpp \
 src/classinfo.h \
 src/gc.cpp \
 src/gc.h \
 src/info.cpp \
 src/jit.cpp \
 src/jit.h \
 src/stack.cpp \
 src/heapallocator.cpp \
 src/main.cpp \
 src/memberinfo.cpp \
 src/memberinfo.h \
 src/methodinfo.cpp \
 src/methodinfo.h \
 src/vm.cpp \
 src/vm.h \
 src/helper_functions.h \
 src/helper_functions.cpp

LeopardVM_CXXFLAGS =
LeopardVM_LDADD = $(DEPS_LIBS) -lpthread -llightning
dist_noinst_SCRIPTS = autogen.sh
