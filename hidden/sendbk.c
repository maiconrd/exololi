#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "lib.h"

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

    char *pos;
    sprintf(pos, "%02X", p1);
    printf("%s\n", pos);
    
    char *init = "22622000";
    unsigned char *data;
    
    data = malloc(16); 
    strcpy(data, init); 
    strcat(data, pos);

    sprintf(pos, "%02X", p2);
    strcat(data, pos);
    printf("%s\n", pos);

    sprintf(pos, "%02X", p3);
    strcat(data, pos);
    printf("%s\n", pos);

    sprintf(pos, "%02X", p4);
    strcat(data, pos);
    printf("%s\n", pos);
    
    printf("%s\n", data);

    usleep(5000000);

    unsigned char* msg;

    buildMsg(&data, &msg);
	canSend(msg);

	free(data);
	
	/*
	char pos[9]; 	 
    sprintf(pos, "%X", position);
    printf("%s\n", pos);    
    int length = strlen(pos);
    char *init = "22622000";
    unsigned char *data;
    
    data = malloc(16); 
    strcpy(data, init); 
    
    for(int i=0; i<(8-length); i++){
       strcat(data, "0");
    }
    strcat(data, pos); 
    printf("%s\n", data);

    unsigned char* msg;

    buildMsg(&data, &msg);
	canSend(msg);

	free(data);
	free(msg);*/
}

int main(int argc, char **argv){		

	doStart();

	PDOconfig();

	char *p = argv[1];
	unsigned int position = atoi(p);

	printf("the position is %d\n", position);	

	goToPosition(position);

	



	/*
	unsigned int angled = 3200;
	unsigned char setpoint_position[8] = {0x22, 0x62, 0x20, 0x00, 0, 0, 0, 0};

	setpoint_position[4] = angled & 0xFF; 
  	setpoint_position[5] = (angled >> 8) & 0xFF;
  	setpoint_position[6] = (angled >> 16) & 0xFF;
  	setpoint_position[7] = (angled >> 24) & 0xFF;
  	*/

  	/*
  	if angled = 0xDEADBEEF, then:
  	setpoint_position[4] = EF
  	setpoint_position[5] = BE
  	setpoint_position[6] = AD
  	setpoint_position[7] = DE
  	*/

  	/*
  	for(int i=0; i<8; i++){
  		printf("%X\n", setpoint_position[i]);
  	}
  	*/
  	

  	

  	/*

  	unsigned char* msg;
  	unsigned char *turn = "22622000D0070000";


	buildMsg(&turn, &msg);
	canSend(msg);*/

	

	return 1;	
}
