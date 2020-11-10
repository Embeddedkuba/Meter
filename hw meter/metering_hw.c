#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>


#define DEST_PORT            2000
#define SERVER_IP_ADDRESS   "127.0.0.1"

test_struct_t client_data;
result_struct_t result;
meter_hw_registers_t measurements;
struct timespec tnow, tstart;
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


    clock_gettime(CLOCK_MONOTONIC, &tstart);
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &tnow);
        const int32_t secs_since_start = tnow.tv_sec - tstart.tv_sec;

        for (int phase = 0; phase < PHASE_CNT; ++phase) {
            hwregs.meters.per_phase[phase].v = (220 + phase * 10 + secs_since_start % 5) * 1000;
            hwregs.meters.per_phase[phase].i = phase * 1000 + (secs_since_start % 10) * 100;
            hwregs.meters.per_phase[phase].ai = secs_since_start * 100 + phase * 10;
            hwregs.meters.per_phase[phase].ae = secs_since_start * 10 + phase * 1;
        }
        hwregs.meters.voltage_angles[0] = 120;
        hwregs.meters.voltage_angles[1] = 120 - secs_since_start % 30;
        hwregs.meters.voltage_angles[2] = 240 - hwregs.meters.voltage_angles[1];

        hwregs.meters.current_angles[1] = 120;
        hwregs.meters.current_angles[2] = 120 - secs_since_start % 30;
        hwregs.meters.current_angles[0] = 240 - hwregs.meters.current_angles[2];
        hwregs.identity='D';
        hwregs.terminator='T';
            
           /*step 5 : send the data to server*/
        sent_recv_bytes = sendto(sockfd, 
                                 &hwregs,
                                 sizeof(hwregs), //+ Identity ="D"
                                 0, 
                                 (struct sockaddr *)&dest, 
                                 sizeof(struct sockaddr));

        //sent_recv_bytes =  recvfrom(sockfd, (char *)&result, sizeof(result_struct_t), 0,
        //                       (struct sockaddr *)&dest, &addr_len);
        //wymysl jak to zamknac
       //    printf("No of bytes recvd = %d\n", sent_recv_bytes);
   
        usleep(1000*200); // new data approximately 5 times a second
    }
}
    

int
main(int argc, char **argv){

    setup_tcp_communication();
 
    return 0;
}
