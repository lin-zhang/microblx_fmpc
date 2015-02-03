#!/usr/bin/luajit

ffi = require("ffi")
ubx = require("ubx")
time = require("time")
ubx_utils = require("ubx_utils")
ts = tostring

require"strict"
-- require"trace"

-- prog starts here.
ni=ubx.node_create("fmpc_pat_mux_udp");

-- load modules
ubx.load_module(ni, "std_types/stdtypes/stdtypes.so")
ubx.load_module(ni, "std_types/kdl/kdl_types.so")
ubx.load_module(ni, "std_blocks/webif/webif.so")
ubx.load_module(ni, "std_blocks/ptrig/ptrig.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/pat_mux/pat_mux.so")
ubx.load_module(ni, "std_blocks/lfds_buffers/lfds_cyclic.so")
ubx.load_module(ni, "std_blocks/udp_client/udp_client.so")
ubx.load_module(ni, "std_blocks/udp_server/udp_server.so")
ubx.load_module(ni, "std_blocks/youbot_driver/youbot_driver.so")
ubx.load_module(ni, "std_blocks/logging/file_logger.so")

ubx.load_module(ni, "std_blocks/fmpc_c/fmpc.so")



-- create necessary blocks
print("creating instance of 'webif/webif'")
webif1=ubx.block_create(ni, "webif/webif", "webif1", { port="8888" })

print("creating instance of 'pat_mux/pat_mux'")
pat_mux1=ubx.block_create(ni, "pat_mux/pat_mux", "pat_mux1", {pat_mux_config={
mux_type=1,
mux_pattern="F5",
--mux_pattern="C3F5I4D4S2F1I2D2S4C3",
}})

print("creating instance of 'pat_mux/pat_mux'")
pat_mux2=ubx.block_create(ni, "pat_mux/pat_mux", "pat_mux2", {pat_mux_config={
mux_type=0,
mux_pattern="F2",
--mux_pattern="C3F5I4D4S2F1I2D2S4C3",
}})

print("creating instance of 'udp_client/udp_client'")
udp_client1=ubx.block_create(ni, "udp_client/udp_client", "udp_client1", {udp_client_config={host_ip='localhost', port=62000}})

print("creating instance of 'udp_server/udp_server'")
udp_server1=ubx.block_create(ni, "udp_server/udp_server", "udp_server1", {udp_server_config={port=51068}})

print("creating instance of 'lfds_buffers/cyclic'")
fifo5=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo5", {element_num=4, element_size=512})

print("creating instance of 'youbot/youbot_driver'")
youbot1=ubx.block_create(ni, "youbot/youbot_driver", "youbot1", {ethernet_if="eth0" })

print("creating instance of 'std_triggers/ptrig'")
ptrig1=ubx.block_create(ni, "std_triggers/ptrig", "ptrig1",
                        { period={sec=0, usec=1000 }, sched_policy="SCHED_FIFO", sched_priority=80,
                          trig_blocks={ { b=youbot1, num_steps=1, measure=0 } } } )


print("creating instance of 'std_triggers/ptrig'")
ptrig4=ubx.block_create(ni, "std_triggers/ptrig", "ptrig4",
			{ period={sec=0, usec=100000 }, sched_policy="SCHED_FIFO", sched_priority=80,
			  trig_blocks={ { b=pat_mux1, num_steps=1, measure=0 },
					{ b=pat_mux2, num_steps=1, measure=0 },
					{ b=udp_server1, num_steps=1, measure=0},
					{ b=udp_client1, num_steps=1, measure=0} } } )

print("creating instance of 'fmpc/fmpc'")
fmpc1=ubx.block_create(ni, "fmpc/fmpc", "fmpc1", {fmpc_config={
param_kappa=5e-5,
param_iteration=12,
param_fence={0,0,0,0},
param_states_max={5,5,0.4,0.4},
param_states_min={-5,-5,-0.4,-0.4},
param_states_init={-5,0,0,0},
param_inputs_max={3.9195,3.9195,3.9195,3.9195},
param_inputs_min={-3.9195,-3.9195,-3.9195,-3.9195},
param_inputs_init={0,0,0,0},
param_obstacle={-3.5,0.1,0.5}
}})


print("creating instance of 'lfds_buffers/cyclic'")
fifo1=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo1", {element_num=4, element_size=48})
print("creating instance of 'lfds_buffers/cyclic'")
fifo2=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo2", {element_num=4, element_size=96})
print("creating instance of 'lfds_buffers/cyclic'")
fifo3=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo3", {element_num=4, element_size=48})

print("creating instance of 'lfds_buffers/cyclic'")
fifo4=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo4", {element_num=4, element_size=512})

print("creating instance of 'lfds_buffers/cyclic'")
fifo6=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo6", {element_num=4, element_size=512})

print("creating instance of 'lfds_buffers/cyclic'")
fifo7=ubx.block_create(ni, "lfds_buffers/cyclic", "fifo7", {element_num=4, element_size=512})

print("creating instance of 'logging/file_logger'")

rep_conf=[[
{
   --{ blockname='youbot1', portname="base_motorinfo", buff_len=2, },
   --{ blockname='youbot1', portname="base_msr_twist", buff_len=2, },
   { blockname='youbot1', portname="base_msr_odom", buff_len=2, },
   --{ blockname='fmpc1', portname="cmd_twist", buff_len=2, },    
   --{ blockname='fmpc1', portname="cmd_vel", buff_len=2, }     
}
]]

file_log1=ubx.block_create(ni, "logging/file_logger", "file_log1",
                           {filename='report.dat',
                            separator=',',
                            report_conf=rep_conf})

print("creating instance of 'std_triggers/ptrig'")
ptrig2=ubx.block_create(ni, "std_triggers/ptrig", "ptrig2",
                        { period={sec=0, usec=100000 },
                          trig_blocks={
                                        { b=fmpc1, num_steps=1, measure=0} } } )

print("creating instance of 'std_triggers/ptrig'")
ptrig3=ubx.block_create(ni, "std_triggers/ptrig", "ptrig3",
                        { period={sec=0, usec=100000 },
                          trig_blocks={
                                        } } )


--- Create a table of all inversely connected ports:
local yb_pinv={}
ubx.ports_map(youbot1,
	      function(p)
		 local pname = ubx.safe_tostr(p.name)
		 yb_pinv[pname] = ubx.port_clone_conn(youbot1, pname)
	      end)

local fmpc_pinv={}
ubx.ports_map(fmpc1,
              function(p)
                 local pname = ubx.safe_tostr(p.name)
                 fmpc_pinv[pname] = ubx.port_clone_conn(fmpc1, pname)
              end)


__time=ffi.new("struct ubx_timespec")
function gettime()
   ubx.clock_mono_gettime(__time)
   return {sec=tonumber(__time.sec), nsec=tonumber(__time.nsec)}
end

cm_data=ubx.data_alloc(ni, "int32_t")

--- Configure the base control mode.
-- @param mode control mode.
-- @return true if mode was set, false otherwise.
function base_set_control_mode(mode)
   ubx.data_set(cm_data, mode)
   ubx.port_write(yb_pinv.base_control_mode, cm_data)
   local res = ubx.port_read_timed(yb_pinv.base_control_mode, cm_data, 3)
   return ubx.data_tolua(cm_data)==mode
end

grip_data=ubx.data_alloc(ni, "int32_t")
function gripper(v)
   ubx.data_set(grip_data, v)
   ubx.port_write(yb_pinv.arm1_gripper, grip_data)
end

--- Configure the arm control mode.
-- @param mode control mode.
-- @return true if mode was set, false otherwise.
function arm_set_control_mode(mode)
   ubx.data_set(cm_data, mode)
   ubx.port_write(yb_pinv.arm1_control_mode, cm_data)
   local res = ubx.port_read_timed(yb_pinv.arm1_control_mode, cm_data, 3)
   return ubx.data_tolua(cm_data)==mode
end

--- Return once the youbot is initialized or raise an error.
function base_initialized()
   local res=ubx.port_read_timed(yb_pinv.base_control_mode, cm_data, 5)
   return ubx.data_tolua(cm_data)==0 -- 0=MOTORSTOP
end

--- Return once the youbot is initialized or raise an error.
function arm_initialized()
   local res=ubx.port_read_timed(yb_pinv.arm1_control_mode, cm_data, 5)
   return ubx.data_tolua(cm_data)==0 -- 0=MOTORSTOP
end


calib_int=ubx.data_alloc(ni, "int32_t")
function arm_calibrate()
   ubx.port_write(yb_pinv.arm1_calibrate_cmd, calib_int)
end

base_twist_data=ubx.data_alloc(ni, "struct kdl_twist")
base_null_twist_data=ubx.data_alloc(ni, "struct kdl_twist")

--- Move with a given twist.
-- @param twist table.
-- @param dur duration in seconds
function base_move_twist(twist_tab, dur)
   base_set_control_mode(2) -- VELOCITY
   ubx.data_set(base_twist_data, twist_tab)
   local ts_start=ffi.new("struct ubx_timespec")
   local ts_cur=ffi.new("struct ubx_timespec")

   ubx.clock_mono_gettime(ts_start)
   ubx.clock_mono_gettime(ts_cur)

   while ts_cur.sec - ts_start.sec < dur do
      ubx.port_write(yb_pinv.base_cmd_twist, base_twist_data)
      ubx.clock_mono_gettime(ts_cur)
   end
   ubx.port_write(yb_pinv.base_cmd_twist, base_null_twist_data)
end

base_vel_data=ubx.data_alloc(ni, "int32_t", 4)
base_null_vel_data=ubx.data_alloc(ni, "int32_t", 4)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel velocity
-- @param dur time in seconds to apply velocity
function base_move_vel(vel_tab, dur)
   base_set_control_mode(2) -- VELOCITY
   ubx.data_set(base_vel_data, vel_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.base_cmd_vel, base_vel_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.base_cmd_vel, base_null_vel_data)
end

base_cur_data=ubx.data_alloc(ni, "int32_t", 4)
base_null_cur_data=ubx.data_alloc(ni, "int32_t", 4)

--- Move each wheel with an individual current value.
-- @param table of size 4 for with wheel current
-- @param dur time in seconds to apply currents.
function base_move_cur(cur_tab, dur)
   base_set_control_mode(6) -- CURRENT
   ubx.data_set(base_cur_data, cur_tab)

   local ts_start=ffi.new("struct ubx_timespec")
   local ts_cur=ffi.new("struct ubx_timespec")

   ubx.clock_mono_gettime(ts_start)
   ubx.clock_mono_gettime(ts_cur)

   while ts_cur.sec - ts_start.sec < dur do
      ubx.port_write(yb_pinv.base_cmd_cur, base_cur_data)
      ubx.clock_mono_gettime(ts_cur)
   end
   ubx.port_write(yb_pinv.base_cmd_cur, base_null_cur_data)
end


arm_vel_data=ubx.data_alloc(ni, "double", 5)
arm_null_vel_data=ubx.data_alloc(ni, "double", 5)

--- Move each joint with an individual rad/s value.
-- @param table of size for with wheel velocity
-- @param dur time in seconds to apply velocity
function arm_move_vel(vel_tab, dur)
   arm_set_control_mode(2) -- VELOCITY
   ubx.data_set(arm_vel_data, vel_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_vel, arm_vel_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_vel, arm_null_vel_data)
end

arm_eff_data=ubx.data_alloc(ni, "double", 5)
arm_null_eff_data=ubx.data_alloc(ni, "double", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel effocity
-- @param dur time in seconds to apply effocity
function arm_move_eff(eff_tab, dur)
   arm_set_control_mode(6) --
   ubx.data_set(arm_eff_data, eff_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_eff=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_eff, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_eff, arm_eff_data)
      ts_eff=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_eff, arm_null_eff_data)
end

arm_cur_data=ubx.data_alloc(ni, "int32_t", 5)
arm_null_cur_data=ubx.data_alloc(ni, "int32_t", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel curocity
-- @param dur time in seconds to apply curocity
function arm_move_cur(cur_tab, dur)
   arm_set_control_mode(6) --
   ubx.data_set(arm_cur_data, cur_tab)
   local dur = {sec=dur, nsec=0}
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}

   while true do
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      ubx.port_write(yb_pinv.arm1_cmd_cur, arm_cur_data)
      ts_cur=gettime()
   end
   ubx.port_write(yb_pinv.arm1_cmd_cur, arm_null_cur_data)
end


arm_pos_data=ubx.data_alloc(ni, "double", 5)
-- arm_null_pos_data=ubx.data_alloc(ni, "double", 5)

--- Move each wheel with an individual RPM value.
-- @param table of size for with wheel posocity
-- @param dur time in seconds to apply posocity
function arm_move_pos(pos_tab)
   arm_set_control_mode(1) -- POS
   ubx.data_set(arm_pos_data, pos_tab)
   ubx.port_write(yb_pinv.arm1_cmd_pos, arm_pos_data)
end

function arm_tuck() arm_move_pos{2.588, 1.022, 2.248, 1.580, 2.591 } end
function arm_home() arm_move_pos{0,0,0,0,0} end


function help()
   local help_msg=
      [[
youbot test script.
 Base:
      base_set_control_mode(mode)	mode: mstop=0, pos=1, vel=2, cur=6
      base_move_twist(twist_tab, dur)  	move with twist (as Lua table) for dur seconds
      base_move_vel(vel_tab, dur)       move each wheel with individual vel [rpm] for dur seconds
      base_move_cur(cur_tab, dur)       move each wheel with individual current [mA] for dur seconds

]]

   if nr_arms>=1 then
      help_msg=help_msg..[[

 Arm: run arm_calibrate() (after each power-down) _BEFORE_ using the other arm functions!!

      arm_calibrate()			calibrate the arm. !!! DO THIS FIRST !!!
      arm_set_control_mode(mode)	see base.
      arm_move_pos(pos_tab, dur)	move to pos. pos_tab is Lua table of len=5 [rad]
      arm_move_vel(vel_tab, dur)	move joints. vel_tab is Lua table of len=5 [rad/s]
      arm_move_eff(eff_tab, dur)        move joints. eff_tab is Lua table of len=5 [Nm]
      arm_move_cur(cur_tab, dur)        move joints. cur_tab is Lua table of len=5 [mA]
      arm_tuck()                        move arm to "tuck" position
      arm_home()                        move arm to "candle" position
]]
   end
   if nr_arms>=2 then
      help_msg=help_msg..[[

	    WARNING: this script does currently not support the second youbot arm!
      ]]
   end
   print(help_msg)
end

p_pat_mux_in_port=ubx.port_get(pat_mux1, "in_port");
p_udp_server_data_out=ubx.port_get(udp_server1, "data_out");
ubx.port_connect_in(p_pat_mux_in_port, fifo5);
ubx.port_connect_out(p_udp_server_data_out, fifo5);

p_pat_mux_out_port=ubx.port_get(pat_mux2, "out_port");
p_udp_client_data_in=ubx.port_get(udp_client1, "data_in");
ubx.port_connect_out(p_pat_mux_out_port, fifo7);
ubx.port_connect_in(p_udp_client_data_in, fifo7);

p_fmpc_cmd_vel=ubx.port_get(fmpc1, "cmd_vel")
p_fmpc_cmd_twist=ubx.port_get(fmpc1, "cmd_twist")
p_fmpc_odom_input=ubx.port_get(fmpc1, "fmpc_odom_port")
p_fmpc_twist_input=ubx.port_get(fmpc1, "fmpc_twist_port")

p_youbot_curr_input=ubx.port_get(youbot1, 'base_cmd_cur');
p_youbot_twist_input=ubx.port_get(youbot1, 'base_cmd_twist');
p_youbot_msr_odom=ubx.port_get(youbot1, 'base_msr_odom');
p_youbot_msr_twist=ubx.port_get(youbot1, 'base_msr_twist');

p_fmpc_msr_odom=ubx.port_get(fmpc1, 'youbot_info_port');

p_fmpc_obstacle=ubx.port_get(fmpc1, 'fmpc_obstacle');
p_fmpc_goal_pose=ubx.port_get(fmpc1, 'fmpc_goal_pose');
p_fmpc_virtual_fence=ubx.port_get(fmpc1, 'fmpc_virtual_fence');

p_fmpc_wm_info_in=ubx.port_get(fmpc1, 'fmpc_wm_info_in');
p_pat_mux_out_float=ubx.port_get(pat_mux1, "out_float");

p_fmpc_robot_pose=ubx.port_get(fmpc1, 'fmpc_robot_pose');
p_pat_mux_in_float = ubx.port_get(pat_mux2, "in_float");
ubx.port_connect_in(p_pat_mux_in_float, fifo7);
ubx.port_connect_out(p_fmpc_robot_pose, fifo7);

ubx.port_connect_in(p_fmpc_wm_info_in, fifo6);
ubx.port_connect_out(p_pat_mux_out_float, fifo6);


ubx.port_connect_out(p_fmpc_cmd_twist, fifo1);
ubx.port_connect_in(p_youbot_twist_input, fifo1);

ubx.port_connect_out(p_youbot_msr_odom,fifo2);
ubx.port_connect_in(p_fmpc_odom_input,fifo2);

ubx.port_connect_out(p_youbot_msr_twist,fifo3);
ubx.port_connect_in(p_fmpc_twist_input,fifo3);

-- FMPC controller
-- @param dur: time in seconds to run the controller
function fmpc_run(dur_in)
   base_set_control_mode(2) -- VELOCITY
   --ubx.block_stop(ptrig1);
   --ubx.data_set(twist_data, fifo_out_youbot_msr_twist_to_fmpc)
   local dur = {sec=0, nsec=0}
   dur.sec,dur.nsec=math.modf(dur_in)
   dur.nsec = dur.nsec * 1000000000
        print(dur.sec, dur.nsec)
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}
   ubx.block_start(ptrig2)
   ubx.block_start(ptrig3)
   while true do
      ubx.cblock_step(file_log1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   ubx.block_stop(ptrig2)
   ubx.block_stop(ptrig3)
   ubx.port_write(p_fmpc_cmd_twist, base_null_twist_data)
end

goal_arr_data=ubx.data_alloc(ni, "float", 2)
obs_arr_data=ubx.data_alloc(ni, "float", 3)


function fmpc_move(dur_in, goal_arr, obs_arr)
   base_set_control_mode(2) -- VELOCITY

   ubx.data_set(goal_arr_data, goal_arr)
   ubx.data_set(obs_arr_data, obs_arr)

   ubx.port_write(fmpc_pinv.fmpc_obstacle, obs_arr_data)
   ubx.port_write(fmpc_pinv.fmpc_goal_pose, goal_arr_data)
   --ubx.block_stop(ptrig1);
   --ubx.data_set(twist_data, fifo_out_youbot_msr_twist_to_fmpc)
   local dur = {sec=0, nsec=0}
   dur.sec,dur.nsec=math.modf(dur_in)
   dur.nsec = dur.nsec * 1000000000
        print(dur.sec, dur.nsec)
   local ts_start=gettime()
   local ts_cur=gettime()
   local diff = {sec=0,nsec=0}
   ubx.block_start(ptrig2)
   ubx.block_start(ptrig3)
   while true do
      ubx.cblock_step(file_log1)
      diff.sec,diff.nsec=time.sub(ts_cur, ts_start)
      if time.cmp(diff, dur)==1 then break end
      --ubx.port_write(p_cmd_twist, cmd_twist)          
      --assert(ubx.cblock_step(fmpc1)==0);      
      ts_cur=gettime()
   end
   ubx.block_stop(ptrig2)
   ubx.block_stop(ptrig3)
   ubx.port_write(p_fmpc_cmd_twist, base_null_twist_data)
end


ubx.block_init(fifo5);
assert(ubx.block_init(webif1)==0)
assert(ubx.block_init(ptrig4)==0)
assert(ubx.block_init(pat_mux1)==0)
assert(ubx.block_init(pat_mux2)==0)
assert(ubx.block_init(udp_client1)==0)
assert(ubx.block_init(udp_server1)==0)

-- start and init webif and youbot
ubx.block_init(fifo1);
ubx.block_init(fifo2);
ubx.block_init(fifo3);
ubx.block_init(fifo4);
ubx.block_init(fifo6);
ubx.block_init(fifo7);
assert(ubx.block_init(ptrig1))
assert(ubx.block_init(ptrig2))
assert(ubx.block_init(ptrig3))
assert(ubx.block_init(file_log1))
assert(ubx.block_init(fmpc1)==0)
assert(ubx.block_init(youbot1)==0)


nr_arms=ubx.data_tolua(ubx.config_get_data(youbot1, "nr_arms"))

ubx.block_start(fifo5);
assert(ubx.block_start(pat_mux1)==0)
assert(ubx.block_start(pat_mux2)==0)
assert(ubx.block_start(ptrig4)==0)
assert(ubx.block_start(udp_client1)==0)
assert(ubx.block_start(udp_server1)==0)
ubx.block_start(fifo1);
ubx.block_start(fifo2);
ubx.block_start(fifo3);
ubx.block_start(fifo4);
ubx.block_start(fifo6);
ubx.block_start(fifo7);
assert(ubx.block_start(webif1)==0)
assert(ubx.block_start(file_log1)==0)
assert(ubx.block_start(fmpc1)==0)
assert(ubx.block_start(youbot1)==0)
assert(ubx.block_start(ptrig1)==0)

-- make sure youbot is running ok.
base_initialized()

if nr_arms==1 then
   arm_initialized()
elseif nr_arms==2 then
   print("WARNING: this script does not yet support a two arm youbot (the driver does however)")
end

twst={vel={x=0.05,y=0,z=0},rot={x=0,y=0,z=0.1}}
vel_tab={1,1,1,1}
arm_vel_tab={0.002,0.002,0.003,0.002, 0.002}




print('Please run "help()" for information on available functions')
-- ubx.node_cleanup(ni)
                             
