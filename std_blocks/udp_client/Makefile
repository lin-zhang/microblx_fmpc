ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

udp_client.so: udp_client.o $(INCLUDE_DIR)/libubx.so 
	${CC} $(CFLAGS_SHARED) -o udp_client.so udp_client.o $(INCLUDE_DIR)/libubx.so

udp_client.o: udp_client.c $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CC} -fPIC -I$(INCLUDE_DIR) -c $(CFLAGS) udp_client.c

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
