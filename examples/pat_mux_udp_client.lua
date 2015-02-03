#!/usr/bin/luajit

ffi = require("ffi")
ubx = require("ubx")
time = require("time")
ubx_utils = require("ubx_utils")
ts = tostring

require"strict"
-- require"trace"

-- prog starts here.
ni=ubx.node_create("pat_mux_udp");

-- load modules
ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/pat_mux/pat_mux.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/udp_client/udp_client.so")

-- create necessary blocks
print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'pat_mux/pat_mux'")
pat_mux1=ubx.block_create(ni, "pat_mux/pat_mux", "pat_mux1", {pat_mux_config={
mux_type=0,
mux_pattern="C3F4I4D4S2F2I2D2S4C3",
}})

print("creating instance of 'udp_client/udp_client'")
udp_client1=ubx.block_create(ni, "udp_client/udp_client", "udp_client1", {udp_client_config={host_ip='localhost', port=51068}})

print("creating instance of 'lfds_buffers/cyclic'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo1", {element_num=4, element_size=512})

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
			{ period={sec=0, usec=100000 }, sched_policy="SCHED_FIFO", sched_priority=80,
			  trig_blocks={ { b=pat_mux1, num_steps=1, measure=0 },
					{ b=udp_client1, num_steps=1, measure=0} } } )

p_pat_mux_out_port=ubx.port_get(pat_mux1, "out_port");
p_udp_client_data_in=ubx.port_get(udp_client1, "data_in");
ubx.port_connect_out(p_pat_mux_out_port, fifo1);
ubx.port_connect_in(p_udp_client_data_in, fifo1);

ubx.block_init(fifo1);
assert(ubx.block_init(webif1)==0)
assert(ubx.block_init(ptrig1)==0)
assert(ubx.block_init(pat_mux1)==0)
assert(ubx.block_init(udp_client1)==0)

ubx.block_start(fifo1);
assert(ubx.block_start(webif1)==0)
assert(ubx.block_start(pat_mux1)==0)
assert(ubx.block_start(ptrig1)==0)
assert(ubx.block_start(udp_client1)==0)
