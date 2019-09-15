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

int main(int argc, char **argv){	
	fd_set rdfs;
	int s;
	struct ifreq ifr;
	struct sockaddr_can addr;
	struct can_frame frame;
		
	//memset(&ifr, 0x0, sizeof(ifr));
	//memset(&addr, 0x0, sizeof(addr));
	//memset(&frame, 0x0, sizeof(frame));
	
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

	while (running) {
		FD_ZERO(&rdfs);		
		FD_SET(s, &rdfs);
		if (FD_ISSET(s, &rdfs)) {				
				read(s, &frame, sizeof(frame));
				fprint_long_canframe(stdout, &frame, NULL, 0);				
				printf("\n");
			}
		out_fflush:
			fflush(stdout);
	}
	close(s);

	return 0;
}