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
	char *ptr;
	struct sockaddr_can addr;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	struct iovec iov;
	struct msghdr msg;	
	struct can_frame frame;
	int nbytes;
	struct ifreq ifr;
	ptr = "can0";	
	int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	nbytes = strlen(ptr);
	addr.can_family = AF_CAN;
	memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
	strncpy(ifr.ifr_name, ptr, nbytes);	
	bind(s, (struct sockaddr *)&addr, sizeof(addr));
	/* these settings are static and can be held out of the hot path */
	iov.iov_base = &frame;
	msg.msg_name = &addr;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;

	while (running) {
		FD_ZERO(&rdfs);		
		FD_SET(s, &rdfs);		

		if (FD_ISSET(s, &rdfs)) {				
				/* these settings may be modified by recvmsg() */
				iov.iov_len = sizeof(frame);
				msg.msg_namelen = sizeof(addr);
				msg.msg_controllen = sizeof(ctrlmsg);  
				msg.msg_flags = 0;
				nbytes = recvmsg(s, &msg, 0);
				fprint_long_canframe(stdout, &frame, NULL, 0);				
				printf("\n");
			}
		out_fflush:
			fflush(stdout);
	}
	close(s);
	return 0;
}