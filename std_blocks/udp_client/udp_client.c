/*
 * udp_client
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
#include "types/udp_client_config.h"
#include "types/udp_client_config.h.hexarr"

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
#include <netdb.h>

#define BUF_SIZE 512

/* declare the type and give the char array type representation as the type private_data */
ubx_type_t udp_client_config_type = def_struct_type(struct udp_client_config, &udp_client_config_h);

/* function block meta-data
 * used by higher level functions.
 */
char udp_client_meta[] =
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
ubx_config_t udp_client_config[] = {
	{ .name="udp_client_config", .type_name = "struct udp_client_config" },
	{ NULL },
};

/* Ports
 */
ubx_port_t udp_client_ports[] = {
	{ .name="data_in", .in_type_name="char", .in_data_len=BUF_SIZE },
	{ NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct udp_client_info {
	unsigned char* host_ip;
	char buf[BUF_SIZE];
	unsigned int port;
	int n,s;
	struct sockaddr_in server;
	struct hostent *host;
};

/* convenience functions to read/write from the ports these fill a
 * ubx_data_t, and call port->[read|write](&data). These introduce
 * some type safety.
 */

def_read_arr_fun(read_bytes, char, BUF_SIZE)
//def_write_fun(write_uint, unsigned int)

/**
 * udp_client_init - block init function.
 *
 * for RT blocks: any memory should be allocated here.
 *
 * @param b
 *
 * @return Ok if 0,
 */
static int udp_client_init(ubx_block_t *b)
{
	int ret=0;
	DBG(" ");

	if ((b->private_data = calloc(1, sizeof(struct udp_client_info)))==NULL) {
		ERR("Failed to alloc memory");
		ret=EOUTOFMEM;
		goto out;
	}
	struct udp_client_info* inf=(struct udp_client_info*) b->private_data;
	
	unsigned int clen;
	struct udp_client_config* udp_client_conf;
	
        udp_client_conf = (struct udp_client_config*) ubx_config_get_data_ptr(b, "udp_client_config", &clen);

	inf->port = udp_client_conf->port;
	
	inf->host=gethostbyname((const char*)udp_client_conf->host_ip);
    	/* initialize socket */
    	if ((inf->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
    	}
/*
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if (setsockopt(inf->s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
                perror("Error");
        }
*/

    	/* initialize server addr */
    	memset((char *) &(inf->server), 0, sizeof(struct sockaddr_in));
	inf->server.sin_family = AF_INET;
    	inf->server.sin_port = htons(inf->port);
    	inf->server.sin_addr = *((struct in_addr*) inf->host->h_addr);
 out:
	return ret;
}

/**
 * udp_client_cleanup - cleanup block.
 *
 * for RT blocks: free all memory here
 *
 * @param b
 */
static void udp_client_cleanup(ubx_block_t *b)
{
	DBG(" ");
        struct udp_client_info* inf=(struct udp_client_info*) b->private_data;
	close(inf->s);
	free(b->private_data);
}

/**
 * udp_client_start - start the udp_client block.
 *
 * @param b
 *
 * @return 0 if Ok, if non-zero block will not be started.
 */
static int udp_client_start(ubx_block_t *b)
{
	DBG("in start");
	return 0; /* Ok */
}

/**
 * udp_client_step - this function implements the main functionality of the
 * block. Ports are read and written here.
 *
 * @param b
 */
static void udp_client_step(ubx_block_t *b) {
	int ret=0;
	struct udp_client_info* inf=(struct udp_client_info*) b->private_data;
	unsigned int len = sizeof(struct sockaddr_in);
	ubx_port_t* data_port = ubx_port_get(b, "data_in");
	ret=read_bytes(data_port, &(inf->buf));
	//printf("ret %d\n", ret);

	size_t intm;
	intm=(size_t)sizeof(inf->buf);

	if(ret>0){
	//printf("from server sending : strlen(inf->buf)%d  %s\n",(int)strlen(inf->buf),inf->buf);
	if (sendto(inf->s, inf->buf, intm, 0, (struct sockaddr *) &(inf->server), len) == -1) {
		perror("sendto()");
		return;
   	}
	}
	
}


/* put everything together
 *
 */
ubx_block_t udp_client_comp = {
	.name = "udp_client/udp_client",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = udp_client_meta,
	.configs = udp_client_config,
	.ports = udp_client_ports,

	/* ops */
	.init = udp_client_init,
	.start = udp_client_start,
	.step = udp_client_step,
	.cleanup = udp_client_cleanup,
};

/**
 * udp_client_module_init - initialize module
 *
 * here types and blocks are registered.
 *
 * @param ni
 *
 * @return 0 if OK, non-zero otherwise (this will prevent the loading of the module).
 */
static int udp_client_module_init(ubx_node_info_t* ni)
{
	ubx_type_register(ni, &udp_client_config_type);
	return ubx_block_register(ni, &udp_client_comp);
}

/**
 * udp_client_module_cleanup - de
 *
 * unregister blocks.
 *
 * @param ni
 */
static void udp_client_module_cleanup(ubx_node_info_t *ni)
{
	ubx_type_unregister(ni, "struct udp_client_config");
	ubx_block_unregister(ni, "udp_client/udp_client");
}

/* declare the module init and cleanup function */
UBX_MODULE_INIT(udp_client_module_init)
UBX_MODULE_CLEANUP(udp_client_module_cleanup)
