GCC=gcc
GXX=g++

GCC_PLUGIN_PATH=`gcc -print-file-name=plugin`/include
GCC_INSTALL_PATH=/home/kenny/MIHPS/Compilation/GCC\ Docs/Install/gcc-4.9.1/gcc

IFLAGS=-I./include -I${GCC_PLUGIN_PATH} -I${GCC_INSTALL_PATH}
CFLAGS=
LDFLAGS=-O1
LIBFLAGS=-shared -fno-rtti -fpic

all:

%_test: lib/vcheck_plugin.so tests/%.c
	${GCC} -fplugin=$^ -o bin/$@ ${IFLAGS} ${LDFLAGS}
	bin/$@

tmp/%.o: tests/%.c
	${GCC} -c $< -o $@

lib/vcheck_plugin.so: src/vectorization-plugin.cpp
	${GXX} $< ${LIBFLAGS} -o $@ -Wl,-soname,$@ ${IFLAGS} 
