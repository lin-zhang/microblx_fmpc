include ../make.conf
INCLUDE_DIR=../../src/

libubx.so: ubx.o
	${CC} $(LIBUBX_CFLAGS_SHARED) -o $@ $^ -lrt

ubx_proto.h: ubx.c
	cproto ubx.c > $@

ubx.o: ubx.c ubx_proto.h ubx.h ubx_types.h uthash.h uthash_ffi.h
	${CC} -fPIC -c ${LIBUBX_CFLAGS} ubx.c

clean:
	rm -f *.o *.so *~ core

check:
	splint ubx.c ubx.h ubx_types.h ubx_proto.h

mrproper:
	rm -f *.o *.so *~ core ubx_proto.h
