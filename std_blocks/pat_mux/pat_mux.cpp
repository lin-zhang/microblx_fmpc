/*
 * A demo C++ block
 */

#ifndef DEBUG
#define DEBUG 1
#endif

//#define LOCAL_DEBUG

#include <iostream>
using namespace std;

#include "ubx.h"
#include <cstring>

#include <string.h>
#include <cstdlib>
#include <iostream>

#include <ctype.h>
#include <typeinfo>

#include "types/pat_mux_config.h"
#include "types/pat_mux_config.h.hexarr"

using namespace std;

ubx_type_t pat_mux_config_type = def_struct_type(struct pat_mux_config, &pat_mux_config_h);
/* function block meta-data
 * used by higher level functions.
 */

char pat_mux_meta[] =
	"{ doc='A pat_mux number generator function block',"
	"  license='LGPL',"
	"  real-time=true,"
	"}";

/* configuration
 * upon cloning the following happens:
 *   - value.type is resolved
 *   - value.data will point to a buffer of size value.len*value.type->size
 *
 * if an array is required, then .value = { .len=<LENGTH> } can be used.
 */
ubx_config_t pat_mux_config[] = {
	{.name="pat_mux_config", .type_name="struct pat_mux_config"},	
	{ NULL },
};

#define N_TYPES			16
#define MAX_LEN_DATA 		512 /*bytes*/
#define MAX_LEN_TYPE_DATA 	64 /*array length per data port*/

#define MUX 	0 	/*char string to data*/
#define DEMUX 	1	/*data to char string*/ 

ubx_port_t pat_mux_ports[] = {
        { .name="in_port",      .in_type_name="char",           .in_data_len=MAX_LEN_DATA },
        { .name="in_char",      .in_type_name="char",           .in_data_len=MAX_LEN_TYPE_DATA },
        { .name="in_short",     .in_type_name="short",          .in_data_len=MAX_LEN_TYPE_DATA },
        { .name="in_int",       .in_type_name="int",            .in_data_len=MAX_LEN_TYPE_DATA },
        { .name="in_long",      .in_type_name="long",           .in_data_len=MAX_LEN_TYPE_DATA },
        { .name="in_float",     .in_type_name="float",          .in_data_len=MAX_LEN_TYPE_DATA },
        { .name="in_double",    .in_type_name="double",         .in_data_len=MAX_LEN_TYPE_DATA },
	{ .name="in_total_bytes",	.in_type_name="int"},
	
        { .name="out_port",     .out_type_name="char",          .out_data_len=MAX_LEN_DATA },
        { .name="out_char",     .out_type_name="char",          .out_data_len=MAX_LEN_TYPE_DATA },
        { .name="out_short",    .out_type_name="short",         .out_data_len=MAX_LEN_TYPE_DATA },
        { .name="out_int",      .out_type_name="int",           .out_data_len=MAX_LEN_TYPE_DATA },
        { .name="out_long",     .out_type_name="long",          .out_data_len=MAX_LEN_TYPE_DATA },
        { .name="out_float",    .out_type_name="float",         .out_data_len=6},
        { .name="out_double",   .out_type_name="double",        .out_data_len=MAX_LEN_TYPE_DATA },
	{ .name="out_total_bytes",	.out_type_name="int"},
        { NULL },
};

/* block local info
 *
 * This struct holds the information needed by the hook functions
 * below.
 */
struct pat_mux_info {
        char mux_type;
        char *mux_pattern;

	char 	in_bytes[MAX_LEN_DATA];
	char  	in_char[MAX_LEN_TYPE_DATA];
	short 	in_short[MAX_LEN_TYPE_DATA];
	int	in_int[MAX_LEN_TYPE_DATA];
	long	in_long[MAX_LEN_TYPE_DATA];
	float	in_float[MAX_LEN_TYPE_DATA];
	double 	in_double[MAX_LEN_TYPE_DATA];
	int 	in_total_bytes;

	char 	out_bytes[MAX_LEN_DATA];
	char  	out_char[MAX_LEN_TYPE_DATA];
	short 	out_short[MAX_LEN_TYPE_DATA];
	int	out_int[MAX_LEN_TYPE_DATA];
	long	out_long[MAX_LEN_TYPE_DATA];
	float	out_float[6];
	double	out_double[MAX_LEN_TYPE_DATA];
	int 	out_total_bytes;

	char 	typeArr[N_TYPES];
	int	typeLen[N_TYPES];
};

def_read_fun(read_total_bytes, int)
def_read_arr_fun(read_in_bytes, 	char, 		MAX_LEN_DATA)
def_read_arr_fun(read_in_char, 		char, 		MAX_LEN_TYPE_DATA)
def_read_arr_fun(read_in_short, 	short, 		MAX_LEN_TYPE_DATA)
def_read_arr_fun(read_in_int, 		int, 		MAX_LEN_TYPE_DATA)
def_read_arr_fun(read_in_long, 		long, 		MAX_LEN_TYPE_DATA)
def_read_arr_fun(read_in_float, 	float, 		MAX_LEN_TYPE_DATA)
def_read_arr_fun(read_in_double, 	double, 	MAX_LEN_TYPE_DATA)

def_write_fun(write_total_bytes, int)
def_write_arr_fun(write_out_bytes, 	char, 		MAX_LEN_DATA)
def_write_arr_fun(write_out_char, 	char, 		MAX_LEN_TYPE_DATA)
def_write_arr_fun(write_out_short, 	short, 		MAX_LEN_TYPE_DATA)
def_write_arr_fun(write_out_int, 	int, 		MAX_LEN_TYPE_DATA)
def_write_arr_fun(write_out_long, 	long, 		MAX_LEN_TYPE_DATA)
def_write_arr_fun(write_out_float, 	float, 		6)
def_write_arr_fun(write_out_double, 	double, 	MAX_LEN_TYPE_DATA)

template<typename T>
void showArray(T* var, int len){
	for(int i=0;i<len;i++){
		std::cout<<var[i]<<"\t";
		if(i%(sizeof(double))==sizeof(double)-1) std::cout<<std::endl;
	}
	printf("%s",typeid(T).name());
	std::cout<<std::endl;
}

template<typename T>
void bytes2t(char in_bytes[sizeof(T)], T* var){
	union {
		T a;
		unsigned char bytes[sizeof(T)];
	} conv;

	memcpy(conv.bytes, in_bytes, sizeof(T));
	*var=conv.a;
}

template<typename T>
void bytes2tArray(char* bytes, T* tArr, int beginIndexArr, int len){
	for(int i=beginIndexArr;i<len+beginIndexArr;i++){
		bytes2t(bytes, &tArr[i]);
		bytes+=sizeof(T);
	}
}

template<typename T>
void t2Bytes(char bytes_temp[sizeof(T)], T var){
	  union {
	    T a;
	    unsigned char bytes[sizeof(T)];
	  } conv;
	  conv.a = var;
	  memcpy(bytes_temp, conv.bytes, sizeof(T));
}

template<typename T>
void tArray2Bytes(char* bytes, T* tArr, int beginIndexArr, int len){
	for(int i=beginIndexArr;i<len+beginIndexArr;i++){
		t2Bytes(bytes, tArr[i]);
		bytes+=sizeof(T);
	}
}

int str2ByteInfo(char *str, char *typeArr, int *typeLen){
	char *p = str;
	int i=0;
	while (*p) { // While there are more characters to process...
	    if (isdigit(*p)) { // Upon finding a digit, ...
	        long val = strtol(p, &p, 10); // Read a number, ...
	        typeLen[i]=val;
	        i++;
	        //printf("%ld\n", val); // and print it.
	    } else { // Otherwise, move on to the next character.
	    	typeArr[i]=*p;
	        p++;
	    }
	}
	typeArr[i]='\0';
	//printf("%s\n", typeArr); // and print it.
	return i;
	}

#ifdef LOCAL_DEBUG
#define T_LEN 114/*test byte length*/
#define T_DATA_LEN 6
	int 	int_data[T_DATA_LEN]={1,2,3,4,5,6};
	float 	float_data[T_DATA_LEN]={1.0,2.0,3.0,4.0,5.0,6.0};
	double 	double_data[T_DATA_LEN]={1.1,2.1,3.1,4.1,5.1,6.1};
	short 	short_data[T_DATA_LEN]={10,20,30,40,50,60};
	char 	char_data[T_DATA_LEN]={'a', 'b', 'c', 'd', 'e', 'f'};	

	float 	char_float[T_DATA_LEN];
	short 	char_short[T_DATA_LEN];
	double 	char_double[T_DATA_LEN];
	int 	char_int[T_DATA_LEN];
	char 	char_char[T_DATA_LEN];
	
	char 	char_res[T_LEN]={
		0x61, 0x62, 0x63, 0x00, 	0x00, 0x80, 0x3f, 0x00, 	0x00, 0x00, 0x40, 0x00, 	0x00, 0x40, 0x40, 0x00, 	
		0x00, 0x80, 0x40, 0x01, 	0x00, 0x00, 0x00, 0x02, 	0x00, 0x00, 0x00, 0x03, 	0x00, 0x00, 0x00, 0x04, 	
		0x00, 0x00, 0x00, 0x9a, 	0x99, 0x99, 0x99, 0x99, 	0x99, 0xf1, 0x3f, 0xcd, 	0xcc, 0xcc, 0xcc, 0xcc, 	
		0xcc, 0x00, 0x40, 0xcd, 	0xcc, 0xcc, 0xcc, 0xcc, 	0xcc, 0x08, 0x40, 0x66, 	0x66, 0x66, 0x66, 0x66, 	
		0x66, 0x10, 0x40, 0x0a, 	0x00, 0x14, 0x00, 0x00, 	0x00, 0xa0, 0x40, 0x00, 	0x00, 0xc0, 0x40, 0x05, 	
		0x00, 0x00, 0x00, 0x06, 	0x00, 0x00, 0x00, 0x66, 	0x66, 0x66, 0x66, 0x66, 	0x66, 0x14, 0x40, 0x66, 	
		0x66, 0x66, 0x66, 0x66, 	0x66, 0x18, 0x40, 0x1e, 	0x00, 0x28, 0x00, 0x32, 	0x00, 0x3c, 0x00, 0x64, 	
		0x65, 0x66};
#endif


static int pat_mux_init(ubx_block_t *c)
{
	int ret=0;	
	DBG("pat_mux init start");
        if ((c->private_data = calloc(1, sizeof(struct pat_mux_info)))==NULL) {
                ERR("Failed to alloc memory");
                ret=EOUTOFMEM;
                return ret;
        }

        struct pat_mux_info* inf = (struct pat_mux_info*) c->private_data;

        unsigned int clen;
        struct pat_mux_config* pat_mux_conf;
        pat_mux_conf = (struct pat_mux_config*) ubx_config_get_data_ptr(c, "pat_mux_config", &clen);
	
	inf->mux_type=pat_mux_conf->mux_type;	
	inf->mux_pattern=pat_mux_conf->mux_pattern;	

	return 0;
}


static void pat_mux_cleanup(ubx_block_t *c)
{
	cout << "pat_mux_cleanup: hi from " << c->name << endl;
}

static int pat_mux_start(ubx_block_t *c)
{
	cout << "pat_mux_start: hi from " << c->name << endl;
	return 0; /* Ok */
}

static void pat_mux_step(ubx_block_t *c) {
	int cnt_float	=0;
	int cnt_int	=0;
	int cnt_double	=0;
	int cnt_short	=0;
	int cnt_char	=0;
	int total_byte	=0;
	//int ret;
	struct pat_mux_info* inf = (struct pat_mux_info*) c->private_data;
	/* get ports */
        ubx_port_t* in_port	=	ubx_port_get(c, "in_port");		
        ubx_port_t* in_char	=	ubx_port_get(c, "in_char");	
        ubx_port_t* in_short	=	ubx_port_get(c, "in_short");	
        ubx_port_t* in_int	=	ubx_port_get(c, "in_int");	
        ubx_port_t* in_long	=	ubx_port_get(c, "in_long");	
        ubx_port_t* in_float	=	ubx_port_get(c, "in_float");	
        ubx_port_t* in_double	=	ubx_port_get(c, "in_double");	
        ubx_port_t* in_total_bytes	=	ubx_port_get(c, "in_total_bytes");	

        ubx_port_t* out_port	=	ubx_port_get(c, "out_port");	
        ubx_port_t* out_char	=	ubx_port_get(c, "out_char");	
        ubx_port_t* out_short	=	ubx_port_get(c, "out_short");	
        ubx_port_t* out_int	=	ubx_port_get(c, "out_int");	
        ubx_port_t* out_long	=	ubx_port_get(c, "out_long");	
        ubx_port_t* out_float	=	ubx_port_get(c, "out_float");	
        ubx_port_t* out_double	=	ubx_port_get(c, "out_double");	
        ubx_port_t* out_total_bytes	=	ubx_port_get(c, "out_total_bytes");	

	int nNumbers=str2ByteInfo(inf->mux_pattern, inf->typeArr, inf->typeLen);
	//printf("%d nNumbers\n", nNumbers);
	char* bytes_pt;
	total_byte=0;
	if(inf->mux_type==MUX){
		bytes_pt=inf->out_bytes;
		read_in_char(   in_char,        &(inf->in_char)         );
		read_in_short(  in_short,       &(inf->in_short)        );
		read_in_int(    in_int,         &(inf->in_int)          );
		read_in_long(   in_long,        &(inf->in_long)         );
		read_in_float(  in_float,       &(inf->in_float)        );
		read_in_double( in_double,      &(inf->in_double)       );	
/*local debug*/
#ifdef LOCAL_DEBUG
#define P_STR_LEN 21 /*Data pattern string length*/
	DBG("[WARN] Local debug mode, configuration ports are ingored.");
	//char str[P_STR_LEN]="C3F4I4D4S2F2I2D2S4C3";
	//inf->mux_pattern=str;
	for(int i=0;i<T_DATA_LEN;i++){
	inf->in_char[i] = char_data[i];
	inf->in_short[i] = short_data[i];
	inf->in_int[i] = int_data[i];
	inf->in_float[i] = float_data[i];
	inf->in_double[i] = double_data[i];
	}
#ifdef LOCAL_PRINT
	for(int i=0;i<10;i++)
	printf("%3d typeLen[i] %c\n", inf->typeLen[i], inf->typeArr[i]);
#endif
#endif
		for(int k=0;k<nNumbers;k++){
			switch(inf->typeArr[k]){
			case 'f':
			case 'F':
				tArray2Bytes(bytes_pt,inf->in_float,	cnt_float,	inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(float);
				total_byte+=sizeof(float)*inf->typeLen[k];
				cnt_float+=inf->typeLen[k];
				break;
			case 'I':
			case 'i':
				tArray2Bytes(bytes_pt,inf->in_int,	cnt_int,	inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(int);
				total_byte+=sizeof(int)*inf->typeLen[k];
				cnt_int+=inf->typeLen[k];
				break;
			case 'D':
			case 'd':
				tArray2Bytes(bytes_pt,inf->in_double,	cnt_double,	inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(double);
				total_byte+=sizeof(double)*inf->typeLen[k];
				cnt_double+=inf->typeLen[k];
				break;
			case 'S':
			case 's':
				tArray2Bytes(bytes_pt,inf->in_short,	cnt_short,	inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(short);
				total_byte+=sizeof(short)*inf->typeLen[k];
				cnt_short+=inf->typeLen[k];
				break;
			case 'C':
			case 'c':
				tArray2Bytes(bytes_pt,inf->in_char,	cnt_char,	inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(char);
				total_byte+=sizeof(char)*inf->typeLen[k];
				cnt_char+=inf->typeLen[k];
				break;
			}
		}
		//showArray(inf->out_bytes, 114);
		write_out_bytes(out_port, &(inf->out_bytes));
		write_total_bytes(out_total_bytes, &total_byte);
	}
	
	
	int r_total_byte=0;
	if(inf->mux_type==DEMUX){
                read_in_bytes	(in_port,        	&(inf->in_bytes)        );
		read_total_bytes(in_total_bytes,	&r_total_byte);
#ifdef LOCAL_DEBUG
//	for(int i=0;i<T_LEN;i++)
//		inf->in_bytes[i]=char_res[i];
	r_total_byte=T_LEN;
#endif
	        bytes_pt=inf->in_bytes;
		for(int k=0;k<nNumbers;k++){
			switch(inf->typeArr[k]){
			case 'f':
			case 'F':
				bytes2tArray(bytes_pt,inf->out_float,cnt_float,inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(float);
				total_byte+=sizeof(float)*inf->typeLen[k];
				cnt_float+=inf->typeLen[k];
				break;
			case 'I':
			case 'i':
				bytes2tArray(bytes_pt,inf->out_int,cnt_int,inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(int);
				total_byte+=sizeof(int)*inf->typeLen[k];
				cnt_int+=inf->typeLen[k];
				break;
			case 'D':
			case 'd':
				bytes2tArray(bytes_pt,inf->out_double,cnt_double,inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(double);
				total_byte+=sizeof(double)*inf->typeLen[k];
				cnt_double+=inf->typeLen[k];
				break;
			case 'S':
			case 's':
				bytes2tArray(bytes_pt,inf->out_short,cnt_short,inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(short);
				total_byte+=sizeof(short)*inf->typeLen[k];
				cnt_short+=inf->typeLen[k];
				break;
			case 'C':
			case 'c':
				bytes2tArray(bytes_pt,inf->out_char,cnt_char,inf->typeLen[k]);
				bytes_pt+=inf->typeLen[k]*sizeof(char);
				total_byte+=sizeof(char)*inf->typeLen[k];
				cnt_char+=inf->typeLen[k];
				break;
			}
		}
		/*if(r_total_byte!=total_byte){
			printf("[WARN] total_byte is %d, but expected number of bytes from input port  is %d. Abort.  ", total_byte, r_total_byte);
		}
		else{*/
		write_out_char(	out_char,	&(inf->out_char)	);
		write_out_short(out_short,	&(inf->out_short)	);
		write_out_int(	out_int,	&(inf->out_int)		);
		write_out_long(	out_long,	&(inf->out_long)	);
		write_out_float(out_float,	&(inf->out_float)	);
		write_out_double(out_double,	&(inf->out_double)	);
		//}
	}
	
#ifdef LOCAL_DEBUG
if(inf->mux_type==MUX){
#ifdef LOCAL_PRINT
	printf("total_byte=%d \n", total_byte);
	int i;
	for(i=0;i<total_byte;i++){
		printf("0x%02x, ",(unsigned char)inf->out_bytes[i]);
		if(i%4==3)
			printf("\t");
		if(i%16==15)
			printf("\n");
	}
	printf("\n");
        showArray(inf->in_float,       T_DATA_LEN);
        showArray(inf->in_int,         T_DATA_LEN);
        showArray(inf->in_short,       T_DATA_LEN);
        showArray(inf->in_double,      T_DATA_LEN);
        showArray(inf->in_char,        T_DATA_LEN);
#endif
}

if(inf->mux_type==DEMUX){
	printf("received in_bytes: \n");
        printf("total_byte=%d \n", total_byte);
        int i;
        for(i=0;i<total_byte;i++){
                printf("0x%02x, ",(unsigned char)inf->in_bytes[i]);
                if(i%4==3)
                        printf("\t");
                if(i%16==15)
                        printf("\n");
        }
        printf("\n");

	showArray(inf->out_float,	T_DATA_LEN);
	showArray(inf->out_int,		T_DATA_LEN);
	showArray(inf->out_short,	T_DATA_LEN);
	showArray(inf->out_double,	T_DATA_LEN);
	showArray(inf->out_char,	T_DATA_LEN);
}
#endif
	
}


/* put everything together */
ubx_block_t pat_mux_comp = {
	.name = "pat_mux/pat_mux",
	.type = BLOCK_TYPE_COMPUTATION,
	.meta_data = pat_mux_meta,
	.configs = pat_mux_config,
	.ports = pat_mux_ports,

	/* ops */
	.init = pat_mux_init,
	.start = pat_mux_start,
	.step = pat_mux_step,
	.cleanup = pat_mux_cleanup,
};

static int pat_mux_init(ubx_node_info_t* ni)
{
	DBG("pat_mux_init 0");
        ubx_type_register(ni, &pat_mux_config_type);
	return ubx_block_register(ni, &pat_mux_comp);
}

static void pat_mux_cleanup(ubx_node_info_t *ni)
{
	DBG(" ");
        ubx_type_unregister(ni, "struct pat_mux_config");
	ubx_block_unregister(ni, "pat_mux/pat_mux");
}

UBX_MODULE_INIT(pat_mux_init)
UBX_MODULE_CLEANUP(pat_mux_cleanup)
