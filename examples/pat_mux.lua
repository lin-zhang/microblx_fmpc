#!/usr/bin/luajit

ffi = require("ffi")
ubx = require("ubx")
time = require("time")
ubx_utils = require("ubx_utils")
ts = tostring

require"strict"
-- require"trace"

-- prog starts here.
ni=ubx.node_create("pat_mux");

-- load modules
ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/pat_mux/pat_mux.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
-- create necessary blocks
print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'pat_mux/pat_mux'")
pat_mux1=ubx.block_create(ni, "pat_mux/pat_mux", "pat_mux1", {pat_mux_config={
mux_type=1,
mux_pattern="C3F4I4D4S2F2I2D2S4C3",
}})

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{ period={sec=0, usec=1000000 }, sched_policy="SCHED_FIFO", sched_priority=80,
			  trig_blocks={ { b=pat_mux1, num_steps=1, measure=0 } } } )


assert(ubx.block_init(webif1)==0)
assert(ubx.block_init(pat_mux1)==0)
assert(ubx.block_init(ptrig1)==0)

assert(ubx.block_start(webif1)==0)
assert(ubx.block_start(pat_mux1)==0)
assert(ubx.block_start(ptrig1)==0)
