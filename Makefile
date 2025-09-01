PACKAGE=cabaliser

LIB=c_lib

.PHONY: lib_cabaliser.so ${PACKAGE}

all: ${PACKAGE}

${PACKAGE}: lib_cabaliser.so
	pip install -r requirements.txt
	pip install -e .

test:
	cd ${LIB}; \
	${MAKE} test; \
	for i in tests/*.out; do echo $$i; ./$$i; done
	pytest

clean:
	cd ${LIB}; \
	${MAKE} clean
	rm ${LIB}/lib_cabaliser.so
	pip uninstall -y ${PACKAGE}

lib_cabaliser.so: ${LIB}/lib_cabaliser.so

${LIB}/%.so:
	cd ${LIB}; \
	${MAKE}
