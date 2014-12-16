OBJ_DIR=.
OBJECTS=$(OBJ_DIR)/fmpc_functions.o $(OBJ_DIR)/fmpc_solver_top.o
ROOT_DIR=$(CURDIR)/../..
include $(ROOT_DIR)/make.conf
INCLUDE_DIR=$(ROOT_DIR)/src/
MOL_DIR=./mol/
FMPC_DIR=$(MOL_DIR)/fmpc

TYPES:=$(wildcard types/*.h)
HEXARRS:=$(TYPES:%=%.hexarr)

fmpc.so: fmpc.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so 
	${CXX} $(CFLAGS_SHARED) -o fmpc.so fmpc.o $(OBJECTS) $(INCLUDE_DIR)/libubx.so

fmpc.o: fmpc.cpp $(INCLUDE_DIR)/ubx.h $(INCLUDE_DIR)/ubx_types.h $(INCLUDE_DIR)/ubx.c $(HEXARRS)
	${CPP} -fPIC -I$(INCLUDE_DIR) -I$(MOL_DIR) -I$(FMPC_DIR) -c $(CFLAGS) fmpc.cpp

fmpc_functions.o: $(FMPC_DIR)/fmpc_functions.cpp
	${CXX} -c -fPIC $(FMPC_DIR)/fmpc_functions.cpp -I$(MOL_DIR) -I$(FMPC_DIR)

fmpc_solver_top.o: $(FMPC_DIR)/fmpc_solver_top.cpp
	${CXX} -c -fPIC $(FMPC_DIR)/fmpc_solver_top.cpp -I$(MOL_DIR) -I$(FMPC_DIR)

clean:
	rm -f *.o *.so *~ core $(HEXARRS)
