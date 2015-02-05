/*
 * udp_server
 *
 * This is to be a well (over) documented block to serve as a good
 * example.
 */

#define DEBUG 1 

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ubx.h"


/* declare and initialize a microblx type. This will be registered /
 * deregistered in the module init / cleanup at the end of this
 * file.
 *
 * Include regular header file and it's char array representation
 * (used for luajit reflection, logging, etc.)
 */
#include "types/udp_server_config.h"
#include "types/udp_server_config.h.hexarr"

//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 512

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t udp_server_config_type = def_struct_type(struct udp_server_config, &udp_server_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char udp_server_meta[] =
	"{ doc='A block to read data from serial port',"
	"  real-time=true,"
	"}";

/* configuration
 * upon cloning the following happens:
 *   - value.type is resolved
 *   - value.data will point to a buffer of size value.len*value.type->size
 *
 * if an array is required, then .value = { .len=<LENGTH> } can be used.
 */
ubx_config_t udp_server_config[] = {
	{ .name="udp_server_config", .type_name = "struct udp_server_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t udp_server_ports[] = {
	{ .name="data_out", .out_type_name="char", .out_data_len=BUF_SIZE },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct udp_server_info {
	char buf[BUF_SIZE];
	unsigned int port;
	int n,s;
	struct sockaddr_in self;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */
def_write_arr_fun(write_bytes, char, BUF_SIZE)
//def_read_fun(read_uint, unsigned int)
//def_write_fun(write_uint, unsigned int)

/**
 * udp_server_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int udp_server_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");
	if ((b->private_data = calloc(1, sizeof(struct udp_server_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}
	struct udp_server_info* inf=(struct udp_server_info*) b->private_data;
	
	unsigned int clen;
	struct udp_server_config* udp_server_conf;
	
        udp_server_conf = (struct udp_server_config*) ubx_config_get_data_ptr(b, "udp_server_config", &clen);

	inf->port = udp_server_conf->port;

    	/* initialize socket */
    	if ((inf->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
    	}
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if (setsockopt(inf->s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
                perror("Error");
        }

    	/* bind to server port */
    	memset((char *) &(inf->self), 0, sizeof(struct sockaddr_in));
    	inf->self.sin_family = AF_INET;
    	inf->self.sin_port = htons(inf->port);
    	inf->self.sin_addr.s_addr = htonl(INADDR_ANY);
    	if (bind(inf->s, (struct sockaddr *) &(inf->self), sizeof(inf->self)) == -1) {
    	    	perror("bind");
    	    	return 1;
    	}	
 out:
	return ret;
}

/**
 * udp_server_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void udp_server_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct udp_server_info* inf=(struct udp_server_info*) b->private_data;
	close(inf->s);
	free(b->private_data);
}

/**
 * udp_server_start - start the udp_server block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int udp_server_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * udp_server_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void udp_server_step(ubx_block_t *b) {
	
	//int i=0;
	ubx_port_t* data_port = ubx_port_get(b, "data_out");
	struct udp_server_info* inf=(struct udp_server_info*) b->private_data;
	unsigned int len = sizeof(struct sockaddr_in);
	struct sockaddr_in other;

	if ((inf->n = recvfrom(inf->s, inf->buf, BUF_SIZE, 0, (struct sockaddr *) &other, &len)) != -1) {
#ifdef LOCAL_PRINT
		printf("Received from %s:%d: ", 
		inet_ntoa(other.sin_addr), 
		ntohs(other.sin_port)); 
#endif
		fflush(stdout);
		write_bytes(data_port, &(inf->buf)); 
    	}
#ifdef LOCAL_PRINT
        write(1, "\n", 1);
        printf("prepared data to send out:\n");

        int i=0;
        for(i=0;i<114;i++){
                printf("0x%02x, ", (unsigned char)(inf->buf[i]));
                if(i%4==3) printf("\t");
                if(i%16==15) printf("\n");
        }
        printf("\n");
#endif
}


/* put everything together
 *
 */
ubx_block_t udp_server_comp = {
	.name = "udp_server/udp_server",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = udp_server_meta,
	.configs = udp_server_config,
	.ports = udp_server_ports,

	/* ops */
	.init = udp_server_init,
	.start = udp_server_start,
	.step = udp_server_step,
	.cleanup = udp_server_cleanup,
};

/**
 * udp_server_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int udp_server_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &udp_server_config_type);
	return ubx_block_register(ni, &udp_server_comp);
}

/**
 * udp_server_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void udp_server_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct udp_server_config");
	ubx_block_unregister(ni, "udp_server/udp_server");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(udp_server_module_init)
UBX_MODULE_CLEANUP(udp_server_module_cleanup)
