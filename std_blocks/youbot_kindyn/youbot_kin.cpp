/*
 * youbot_kin microblx function block
 */

#include "ubx.h"

#define YOUBOT_NR_OF_JOINTS	5
#define IK_WDLS_LAMBDA		0.5

/* Register a dummy type "struct youbot_kin" */
// #include "types/youbot_kin.h"
// #include "types/youbot_kin.h.hexarr"

#include <kdl/chain.hpp>
#include <kdl/frames.hpp>
#include <kdl/jntarrayvel.hpp>
#include <kdl/chainfksolvervel_recursive.hpp>
#include <kdl/chainiksolvervel_wdls.hpp>

using namespace KDL;

/* from youbot_driver */
#include <motionctrl_jnt_state.h>

/* from std_types/kdl */
#include <kdl.h>


// ubx_type_t types[] = {
//	def_struct_type(struct youbot_kin, &youbot_kin_h),
//	{ NULL },
// };

/* block meta information */
char youbot_kin_meta[] =
	" { doc='',"
	"   license='',"
	"   real-time=true,"
	"}";

/* declaration of block configuration */
ubx_config_t youbot_kin_config[] = {
	{ .name="robot_model", .type_name = "char" },
	{ NULL },
};

/* declaration port block ports */
ubx_port_t youbot_kin_ports[] = {
	/* FK */
	{ .name="arm_in_jntstate", .in_type_name="struct motionctrl_jnt_state" },
	{ .name="arm_out_msr_ee_pose", .out_type_name="struct kdl_frame" },
	{ .name="arm_out_msr_ee_twist", .out_type_name="struct kdl_twist" },

	/* IK */
	{ .name="arm_in_cmd_ee_twist", .in_type_name="struct kdl_twist" },
	{ .name="arm_out_cmd_jnt_vel", .out_type_name="double", .out_data_len=YOUBOT_NR_OF_JOINTS },
	{ NULL },
};

/* define a structure that contains the block state. By assigning an
 * instance of this struct to the block private_data pointer, this
 * struct is available the hook functions. (see init)
 */
struct youbot_kin_info
{
	Chain* chain;
	ChainFkSolverVel_recursive* fpk;	// forward position and velocity kinematics
	ChainIkSolverVel_wdls* ivk;		// Inverse velocity kinematic

	FrameVel *frame_vel;
	JntArrayVel *jnt_array;

	ubx_port_t *p_arm_in_jntstate;
	ubx_port_t *p_arm_in_cmd_ee_twist;
	ubx_port_t *p_arm_out_cmd_jnt_vel;
	ubx_port_t *p_arm_out_msr_ee_pose;
	ubx_port_t *p_arm_out_msr_ee_twist;

};

/* declare convenience functions to read/write from the ports */
def_read_fun(read_jntstate, struct motionctrl_jnt_state)
def_read_fun(read_kdl_twist, struct kdl_twist)
def_write_arr_fun(write_jnt_arr, double, YOUBOT_NR_OF_JOINTS)
def_write_fun(write_kdl_frame, struct kdl_frame)
def_write_fun(write_kdl_twist, struct kdl_twist)

/* init */
static int youbot_kin_init(ubx_block_t *b)
{
	struct youbot_kin_info* inf;
	int ret = -1;

	/* allocate memory for the block state */
	if ((b->private_data = calloc(1, sizeof(struct youbot_kin_info)))==NULL) {
		ERR("youbot_kin: failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}

	inf = (struct youbot_kin_info*) b->private_data;

	inf->chain = new Chain();

	/* youbot arm kinematics */
	inf->chain->addSegment(Segment(Joint(Joint::RotZ), Frame(Vector(0.024, 0.0, 0.096))));
	inf->chain->addSegment(Segment(Joint(Joint::RotY), Frame(Vector(0.033, 0.0, 0.019))));
	inf->chain->addSegment(Segment(Joint(Joint::RotY), Frame(Vector(0.000, 0.0, 0.155))));
	inf->chain->addSegment(Segment(Joint(Joint::RotY), Frame(Vector(0.000, 0.0, 0.135))));
	inf->chain->addSegment(Segment(Joint(Joint::RotZ), Frame(Vector(-0.002, 0.0, 0.130))));

	/* create and configure solvers */
	inf->fpk = new ChainFkSolverVel_recursive(*inf->chain);
	inf->ivk = new ChainIkSolverVel_wdls(*inf->chain, 1.0);

	inf->ivk->setLambda(IK_WDLS_LAMBDA);

	/* extra data */
	inf->frame_vel = new FrameVel();
	inf->jnt_array = new JntArrayVel(YOUBOT_NR_OF_JOINTS);

	/* cache port ptrs */
	assert(inf->p_arm_in_jntstate = ubx_port_get(b, "arm_in_jntstate"));
	assert(inf->p_arm_in_cmd_ee_twist = ubx_port_get(b, "arm_in_cmd_ee_twist"));
	assert(inf->p_arm_out_cmd_jnt_vel = ubx_port_get(b, "arm_out_cmd_jnt_vel"));
	assert(inf->p_arm_out_msr_ee_pose = ubx_port_get(b, "arm_out_msr_ee_pose"));
	assert(inf->p_arm_out_msr_ee_twist = ubx_port_get(b, "arm_out_msr_ee_twist"));

	ret=0;
out:
	return ret;
}

/* start */
static int youbot_kin_start(ubx_block_t *b)
{
	return 0;
}

/* stop */
static void youbot_kin_stop(ubx_block_t *b)
{
}

/* cleanup */
static void youbot_kin_cleanup(ubx_block_t *b)
{
	struct youbot_kin_info* inf;
	inf = (struct youbot_kin_info*) b->private_data;

	delete inf->frame_vel;
	delete inf->jnt_array;
	delete inf->fpk;
	delete inf->ivk;
	delete inf->chain;
	free(b->private_data);
}

/* step */
static void youbot_kin_step(ubx_block_t *b)
{
	int ret;
	struct motionctrl_jnt_state jnt_state;
	struct kdl_twist ee_twist;
	double jnt_vel[YOUBOT_NR_OF_JOINTS] = { 0, 0, 0, 0, 0};

	Twist const *KDLTwistPtr;
	Frame KDLPose;
	Twist KDLTwist;

	struct youbot_kin_info* inf;
	inf = (struct youbot_kin_info*) b->private_data;

	/* read jnt state and  compute forward kinematics */
	if(read_jntstate(inf->p_arm_in_jntstate, &jnt_state) == 1) {
		for(int i=0;i<YOUBOT_NR_OF_JOINTS;i++){
			inf->jnt_array->q(i) = jnt_state.pos[i];
			inf->jnt_array->qdot(i) = jnt_state.vel[i];
		}

		/* compute and write out current EE Pose and Twist */
		inf->fpk->JntToCart(*inf->jnt_array, *inf->frame_vel);

		KDLPose = inf->frame_vel->GetFrame();
		KDLTwist = inf->frame_vel->GetTwist();

		write_kdl_frame(inf->p_arm_out_msr_ee_pose, (struct kdl_frame*) &KDLPose);
		write_kdl_twist(inf->p_arm_out_msr_ee_twist, (struct kdl_twist*) &KDLPose);

	}

	/* read cmd_ee_twist and compute inverse kinematics */
	if(read_kdl_twist(inf->p_arm_in_cmd_ee_twist, &ee_twist) == 1) {
		KDLTwistPtr = (Twist*) &ee_twist; /* uh */
		// kdl_twist = reinterpret_cast<Twist*>(&ee_twist);
		ret = inf->ivk->CartToJnt(inf->jnt_array->q, *KDLTwistPtr, inf->jnt_array->qdot);

		if(ret >= 0) {
			for(int i=0;i<YOUBOT_NR_OF_JOINTS;i++)
				jnt_vel[i] = inf->jnt_array->qdot(i);
		} else {
			ERR("%s: Failed to compute inverse velocity kinematics", b->name);
			/* set jnt_vel to zero (jnt_vel is initialized to zero) */
		}
		/* write out desired jnt velocities */
		write_jnt_arr(inf->p_arm_out_cmd_jnt_vel, &jnt_vel);
	}
}


/* put everything together */
ubx_block_t youbot_kin_block = {
	.name = "youbot_kin",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = youbot_kin_meta,
	.configs = youbot_kin_config,
	.ports = youbot_kin_ports,

	/* ops */
	.init = youbot_kin_init,
	.start = youbot_kin_start,
	.stop = youbot_kin_stop,
	.cleanup = youbot_kin_cleanup,
	.step = youbot_kin_step,
};


/* youbot_kin module init and cleanup functions */
static int youbot_kin_mod_init(ubx_node_info_t* ni)
{
	DBG(" ");
	int ret = -1;
	// ubx_type_t *tptr;

	// for(tptr=types; tptr->name!=NULL; tptr++) {
	//	if(ubx_type_register(ni, tptr) != 0) {
	//		goto out;
	//	}
	// }

	if(ubx_block_register(ni, &youbot_kin_block) != 0)
		goto out;

	ret=0;
out:
	return ret;
}

static void youbot_kin_mod_cleanup(ubx_node_info_t *ni)
{
	DBG(" ");
	// const ubx_type_t *tptr;

	// for(tptr=types; tptr->name!=NULL; tptr++)
	//	ubx_type_unregister(ni, tptr->name);

	ubx_block_unregister(ni, "youbot_kin");
}

/* declare module init and cleanup functions, so that the ubx core can
 * find these when the module is loaded/unloaded */
UBX_MODULE_INIT(youbot_kin_mod_init)
UBX_MODULE_CLEANUP(youbot_kin_mod_cleanup)
