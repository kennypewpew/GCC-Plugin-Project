GCC=gcc
GXX=g++

GCC_INSTALL_PATH=`gcc -print-file-name=plugin`
GCC_PLUGIN_PATH=${GCC_INSTALL_PATH}/include

IFLAGS=-I./include -I${GCC_PLUGIN_PATH} -I${GCC_INSTALL_PATH}
CFLAGS=
LDFLAGS=-O1 -L ./ -lexternal
LIBFLAGS=-shared -fpic
PLIBFLAGS=${LIBFLAGS} -fno-rtti

all:

%_test: tests/%.c lib/vcheck_plugin.so lib/libexternal.so
	echo "------------------------------------------------"
	${GXX} -fplugin=lib/vcheck_plugin.so $< -o bin/$@ ${IFLAGS} ${LDFLAGS} -L./lib/ -I./lib/
	echo "------------------------------------------------"
	LD_LIBRARY_PATH=./lib/ ./bin/$@

%.o: %.c
	${GCC} -c $< -o $@

lib/vcheck_plugin.so: src/vectorization-plugin.cpp
	${GXX} $< ${PLIBFLAGS} -o $@ -Wl,-soname,$@ ${IFLAGS} 

lib/libexternal.so: src/externals.cpp
	${GXX} $< ${LIBFLAGS} -o $@

clean:
	rm -rf *.o *.so basic_add_test
	rm -rf lib/*.so
	rm -rf bin/basic_add_test
