#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "terminal.h"
#include "lib.h"

static volatile int running = 1;

unsigned char* slaveAdress = "601#";

// ***************************
// EPOS2 CANOpen Communication
// ***************************  // Data comes with Lowest Bit First

// Statemachine
unsigned char *set_preoperational = "8000";
unsigned char *set_operational = "0100";
unsigned char *enable_epos = "2B4060000F000000";
unsigned char *disable_epos = "2B40600006000000";

// PDO Configuration
unsigned char *pdo_sync = "00";

unsigned char *pdo_actual_position_1 = "2202180181030000";
unsigned char *pdo_actual_position_2 = "2202180201000000";

unsigned char *pdo_actual_velocity_1 = "2202180181030000";
unsigned char *pdo_actual_velocity_2 = "2202180201000000";

unsigned char *pdo_actual_current_1 = "22001A0000000000";
unsigned char *pdo_actual_current_2 = "22001A0110007860";
unsigned char *pdo_actual_current_3 = "22001A0210002720";
unsigned char *pdo_actual_current_4 = "2200180201000000";
unsigned char *pdo_actual_current_5 = "22001A0001000000";

// *****************
// Object Writing
// *****************

// // Objects [Index(LSB), Index(MSB), Sub-Index]
// unsigned char modes_of_operation[8] = {0x60, 0x60, 0x00};
// unsigned char set_homing_method[8] = {0x98, 0x60, 0x00};
// unsigned char controlword[8] = {0x40, 0x60, 0x00};
// unsigned char position_setting_value[8] = {0x00, 0x62, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};

// Parameters
unsigned char *set_max_following_error = "22656000D0070000";
unsigned char *set_max_acceleration = "22C5600088130000";
unsigned char *set_max_profile_velocity = "227F6000D0070000";
unsigned char *set_min_position_limit = "227D60017F7BE1FF"; // -300000qc = 0xFFFB6C20  -2000000qc=0xFFE17B7F
unsigned char *set_max_position_limit = "227D600280841E00"; // 300000qc = 0x0493E0  2000000qc=0x1E8480

// Commanding
unsigned char *setpoint_position = "2262200000000000";

// *****************
// Object Reading
// *****************

// Actual Values
unsigned char *get_actual_position = "4064600000000000";
unsigned char *get_actual_velocity = "406C600000000000";
unsigned char *get_actual_current =  "4078600000000000";

int canSend(char* code){	
	int s; /* can raw socket */ 
	int nbytes;
	struct sockaddr_can addr;
	struct can_frame frame;
	struct ifreq ifr;

	if (parse_canframe(code, &frame)){
		printf("nope\n");		
		return 1;
	}

	/* open socket */
	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return 1;
	}	

	addr.can_family = AF_CAN;

	strcpy(ifr.ifr_name, "can0");
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	/* disable default receive filter on this RAW socket */
	/* This is obsolete as we do not read from the socket at all, but for */
	/* this reason we can remove the receive list in the Kernel to save a */
	/* little (really a very little!) CPU usage.                          */
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	/* send frame */
	if ((nbytes = write(s, &frame, sizeof(frame))) != sizeof(frame)) {
		perror("write");
		return 1;
	}

	//fprint_long_canframe(stdout, &frame, "\n", 0);

	close(s);
	return 0;
}

void buildMsg(unsigned char **data, unsigned char **msg){
	//unsigned char *slave = "123#";
	*msg = malloc(strlen(slaveAdress)+1+strlen(*data)); 
	strcpy(*msg, slaveAdress); 
	strcat(*msg, *data); 	
}

//********************
// EPOS COMMUNICATION
//********************

void doStart(){

	unsigned char* msg;

	buildMsg(&disable_epos, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&enable_epos, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&set_max_following_error, &msg);
	canSend(msg);	
	usleep(10000);

	buildMsg(&set_max_acceleration, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&set_max_profile_velocity, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&set_min_position_limit, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&set_max_position_limit, &msg);
	canSend(msg);
	usleep(10000);

	free(msg);
}

void PDOconfig(){

	unsigned char* msg;

	buildMsg(&set_preoperational, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_position_1, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_position_2, &msg);
	canSend(msg);
	usleep(10000);	
	/*
	buildMsg(&pdo_actual_velocity_1, &msg);
	canSend(msg);

	buildMsg(&pdo_actual_velocity_2, &msg);
	canSend(msg);
	*/
	buildMsg(&pdo_actual_current_1, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_current_2, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_current_3, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_current_4, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&pdo_actual_current_5, &msg);
	canSend(msg);
	usleep(10000);

	buildMsg(&set_operational, &msg); /// no ModExo.h é 0x00 (= broadcast (eu acho))
	canSend(msg);
	usleep(10000);

	free(msg);

}

void goToPosition(unsigned int angled){

	unsigned char p1 = angled & 0xFF; 
    unsigned char p2 = (angled >> 8) & 0xFF;
    unsigned char p3 = (angled >> 16) & 0xFF;
    unsigned char p4 = (angled >> 24) & 0xFF;

    /*
  	if angled = 0xDEADBEEF, then:
  	setpoint_position[4] = EF
  	setpoint_position[5] = BE
  	setpoint_position[6] = AD
  	setpoint_position[7] = DE
  	*/

    char *pos;
    pos = malloc(sizeof(p1)); ///segmentation fault: core damped SOLVED
    sprintf(pos, "%02X", p1);
    //printf("%s\n", pos);
    
    char *init = "22622000";
    unsigned char *data;
    
    data = malloc(16); 
    strcpy(data, init); 
    strcat(data, pos);

    sprintf(pos, "%02X", p2);
    strcat(data, pos);
    //printf("%s\n", pos);

    sprintf(pos, "%02X", p3);
    strcat(data, pos);
    //printf("%s\n", pos);

    sprintf(pos, "%02X", p4);
    strcat(data, pos);
    //printf("%s\n", pos);
    
    //printf("%s\n", data);

    //usleep(5000000);

    unsigned char* msg;

    buildMsg(&data, &msg);
	canSend(msg);

	free(data);
	free(pos);
}

int position = -1;
int velocity = 0;
int current = 0;

void deCode(struct can_frame frame){
	int data;
	data = frame.data[7];
    data <<=8;    
    data = data | frame.data[6];
    data <<=8;    
    data = data | frame.data[5];
    data <<=8;
    data = data | frame.data[4];

    int code;
	code = frame.data[3];
    code <<=8;    
    code = code | frame.data[2];
    code <<=8;    
    code = code | frame.data[1];
    code <<=8;
    code = code | frame.data[0];
    
    if(code == 0x00606443){
    	position = data;
    }
    if(code == 0x00606C43){
    	velocity = data;
    }
    if(code == 0x0060784B){
    	current = data;
    }
}

int main(int argc, char **argv){	
	fd_set rdfs;
	int s;
	struct ifreq ifr;
	struct sockaddr_can addr;
	struct can_frame frame;
		
	memset(&ifr, 0x0, sizeof(ifr));
	memset(&addr, 0x0, sizeof(addr));
	memset(&frame, 0x0, sizeof(frame));
	
	// open CAN_RAW socket 
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	// convert interface sting "can0" into interface index 
	strcpy(ifr.ifr_name, "can0");
	ioctl(s, SIOCGIFINDEX, &ifr);
	// setup address for bind 
	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;
	// bind socket to the can0 interface 
	bind(s, (struct sockaddr *)&addr, sizeof(addr));	

	doStart();
	PDOconfig();		

	unsigned char *getPos;
	buildMsg(&get_actual_position, &getPos);

	unsigned char *getVel;
	buildMsg(&get_actual_velocity, &getVel);

	unsigned char *getCur;
	buildMsg(&get_actual_current, &getCur);

	unsigned int moveTo;

	if(argc == 2){
		char *p = argv[1];
		moveTo = atoi(p);
	}else{
		moveTo = 16000;
	}	
	
	usleep(5000000); //wait 5 senconds

	struct timeval tv;
    int rc;
    
    int flag = 0;

	while (running) {
		FD_ZERO(&rdfs);		
		FD_SET(s, &rdfs);

		tv.tv_sec = 0;
	    tv.tv_usec = 1000; // 1000 microseconds -> 1kHz 

	    rc = select(s+1, &rdfs, NULL, NULL, &tv);

	    // rc == 0 - timeout
	    if (!rc) {
	         // write your CAN frame	    	
	        canSend(getPos);
	        canSend(getVel);
	        canSend(getCur);	
	        goToPosition(moveTo);	         
	    	printf("| position: %06d qc |", position);	  
	    	printf(" velocity: %06d rpm |", velocity);
	    	printf(" current: %06d mA | \n", current);
	    	if(position == moveTo){
	    		running = 0;
	    	}	
	    }
		if (FD_ISSET(s, &rdfs)) {							
				read(s, &frame, sizeof(frame));
				//fprint_long_canframe(stdout, &frame, NULL, 0);				
				//printf("\n");			
				if(frame.can_id == 0x581){
					deCode(frame);					
				}
			}
		out_fflush:
			fflush(stdout);
	}
	close(s);
	return 0;
}