ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

pat_mux.so: pat_mux.o $(INCLUDE_DIR)/libubx.so 
	${CXX} $(CFLAGS_SHARED) -o pat_mux.so pat_mux.o $(INCLUDE_DIR)/libubx.so

pat_mux.o: pat_mux.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) pat_mux.cpp

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
