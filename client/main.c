#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include "common.h"
#include "protocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>


#define DEST_PORT            2000
#define SERVER_IP_ADDRESS   "127.0.0.1"

identity_msg received_identity;
message_meter received_meters;
message_meter *received_meters_ptr;
meter_hw_registers_t measurements;
integrals_msg integrals_retained;
integrals_msg *received_integral_ptr;
uimax_uimin_msg uiminmax;
uimax_uimin_msg *uiminmax_ptr;

message_meter hwregs;

void
setup_tcp_communication(){

    /*Step 1 : Initialization*/
    /*Socket handle*/
    int sockfd = 0,
        sent_recv_bytes = 0;

    int addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    /*to store socket addesses : ip address and port*/
    struct sockaddr_in dest;

    /*Step 2: specify server information*/
    /*Ipv4 sockets, Other values are IPv6*/
    dest.sin_family = AF_INET;

    /*Client wants to send data to server process which is running on server machine, and listening on
     * port on DEST_PORT, server IP address SERVER_IP_ADDRESS.
     * Inform client about which server to send data to : All we need is port number, and server ip address. Pls note that
     * there can be many processes running on the server listening on different no of ports,
     * our client is interested in sending data to server process which is lisetning on PORT = DEST_PORT*/
    dest.sin_port = DEST_PORT;
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    /*Step 3 : Create a TCP socket*/
    /*Create a socket finally. socket() is a system call, which asks for three paramemeters*/
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    /*Step 4 : get the data to be sent to server*/
    /*Our client is now ready to send data to server. sendto() sends data to Server*/

    /*hardware data imitation*/



    while (1)
    {
    	int choose;
    	printf("Choose action: \n"
    		   "1 - show meter number and verion number \n"
    		   "2 - show instant data \n"
    		   "3 - show angles \n"
     		   "4 - show integrals \n"
    		   "5 - show min/max values per phase \n");
    	scanf("%d", &choose);

    	if (choose==1) {

    		  sent_recv_bytes = sendto(sockfd, "GM",
										 2, //sizeof()
										 0,
										 (struct sockaddr *)&dest,
										 sizeof(struct sockaddr));
    		  sent_recv_bytes =  recvfrom(sockfd, &received_identity, sizeof(received_identity), 0,
    		                      (struct sockaddr *)&dest, &addr_len);
    		  get_identificator_msg(received_identity);
    		  //printf("Meter Number: %s\n",&received_identity.identity_numbers.meter_number);
    		  //printf("Version Number :%s\n",received_identity.identity_numbers.version_number);

    	 } else if(choose ==2) {
    		 sent_recv_bytes = sendto(sockfd, "GA",
									 2, //sizeof()
									 0,
									 (struct sockaddr *)&dest,
									 sizeof(struct sockaddr));
    		 sent_recv_bytes =  recvfrom(sockfd, &received_meters, sizeof(received_meters), 0,
    		     		                      (struct sockaddr *)&dest, &addr_len);
    		 received_meters_ptr=&received_meters;
    		 get_instant_data(received_meters_ptr);
    	 } else if(choose ==3) {
    		 sent_recv_bytes = sendto(sockfd, "GA", //on purpose - code would be smaller if we receive a whole frame
    		 									 2, //sizeof()
    		 									 0,
    		 									 (struct sockaddr *)&dest,
    		 									 sizeof(struct sockaddr));
			 sent_recv_bytes =  recvfrom(sockfd, &received_meters, sizeof(received_meters), 0,
										  (struct sockaddr *)&dest, &addr_len);
			 received_meters_ptr=&received_meters;
			 get_current_angles(received_meters_ptr);
		 } else if(choose ==4) {
			//retained integrals
			 sent_recv_bytes = sendto(sockfd, "GS", //on purpose - code would be smaller if we receive a whole frame
												 2, //sizeof()
												 0,
												 (struct sockaddr *)&dest,
												 sizeof(struct sockaddr));
			 sent_recv_bytes =  recvfrom(sockfd, &integrals_retained, sizeof(integrals_retained), 0,
			 										  (struct sockaddr *)&dest, &addr_len);
			 received_integral_ptr=&integrals_retained;
			 get_retained_integrals(received_integral_ptr);
		 } else if(choose ==5) {
			 //get min/max per-phase voltage and current since last reset
			 sent_recv_bytes = sendto(sockfd, "GX", //on purpose - code would be smaller if we receive a whole frame
												2, //sizeof()
												0,
												(struct sockaddr *)&dest,
												sizeof(struct sockaddr));
			 sent_recv_bytes =  recvfrom(sockfd, &uiminmax, sizeof(uiminmax), 0,
													  (struct sockaddr *)&dest, &addr_len);
			 printf("get: %d\n",uiminmax.voltages.imax[0]);
			 uiminmax_ptr=&uiminmax;
			 get_uimaxmin(uiminmax_ptr);


		 }
    }
}


int
main(int argc, char **argv){

    setup_tcp_communication();

    return 0;
}
