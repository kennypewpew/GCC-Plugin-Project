GCC=gcc
GXX=g++

GCC_PLUGIN_PATH=`gcc -print-file-name=plugin`/include

IFLAGS=-I./include -I${GCC_PLUGIN_PATH}
CFLAGS=
LDFLAGS=

all:

%_test: lib/vcheck_plugin.so tests/%.c #tmp/main.o
	${GCC} -fplugin=$^ -o bin/$@ ${IFLAGS} ${LDFLAGS}

tmp/%.o: tests/%.c
	${GCC} -c $< -o $@

lib/vcheck_plugin.so: src/vectorization-plugin.cpp
	${GXX} $< -shared -fno-rtti -fpic -o $@ -Wl,-soname,$@ ${IFLAGS}
