#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <arpa/inet.h>
#include "common.h"
#include "protocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_CLIENT_SUPPORTED    5
#define SERVER_PORT     2000 /*Server process is running on this port no. Client has to send data to this port no*/
uint32_t iko=0,jko=0;

test_struct_t test_struct;
result_struct_t res_struct;
meter_hw_registers_t meters;
char data_buffer[1024];
char transfer_buffer[256];
const char *data_buffer_ptr0=&data_buffer[0];
const char *data_buffer_ptr1=&data_buffer[1];
const char *data_buffer_ptr2=&data_buffer[104];
int monitored_fd_set[MAX_CLIENT_SUPPORTED+1];
uimax_uimin_meters global_uimax_uimin;
uimax_uimin_msg minmax_msg;
uimax_uimin_msg *minmax_msg_ptr;
integrals_msg prepared_msg;
integrals_msg *prepared_msg_ptr;

message_meter meters_to_other_clients_global;
message_meter *meters_to_other_clients;
uint64_t ai0_retain=0, ae0_retain=0, ai1_retain=0, ae1_retain=0, ai2_retain=0, ae2_retain=0;
integrals retained_values;
uint64_t ai0_to_client=0, ae0_to_client=0, ai1_to_client=0, ae1_to_client=0, ai2_to_client=0, ae2_to_client=0;
integrals values_to_client;
uint64_t ai0_cmp=0, ae0_cmp=0, ai1_cmp=0, ae1_cmp=0, ai2_cmp=0, ae2_cmp=0;
integrals values_to_compare;
integrals integral_values_to_client;
static void
intitiaze_monitor_fd_set(){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++)
        monitored_fd_set[i] = -1;
}

static void
add_to_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }
}

static void
re_init_readfds(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

static int
get_max_fd(){

    int i = 0;
    int max = -1;

    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    }

    return max;
}

void
setup_tcp_server_communication(){

    /*Step 1 : Initialization*/
    /*Socket handle and other variables*/
    int master_sock_tcp_fd = 0, /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
        sent_recv_bytes = 0,
        addr_len = 0,
        opt = 1;

    int comm_socket_fd = 0;     /*client specific communication socket file descriptor, used for only data exchange/communication between client and server*/
    fd_set readfds;             /*Set of file descriptor on which select() polls. Select() unblocks whever data arrives on any fd present in this set*/
    /*variables to hold server information*/
    struct sockaddr_in server_addr, /*structure to store the server and client info*/
                       client_addr;

    /* Just drain the array of monitored file descriptors (sockets)*/
    intitiaze_monitor_fd_set();

    /*step 2: tcp master socket creation*/
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }

    /*Step 3: specify server Information*/
    server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
    server_addr.sin_port = SERVER_PORT;/*Server will process any data arriving on port no 2000*/
    server_addr.sin_addr.s_addr = INADDR_ANY; //3232249957; //( = 192.168.56.101); /*Server's IP address, means, Linux will send all data whose destination address = address of any local interface of this machine, in this case it is 192.168.56.101*/

    addr_len = sizeof(struct sockaddr);

    /* Bind the server. Binding means, we are telling kernel(OS) that any data
     * you recieve with dest ip address = 192.168.56.101, and tcp port no = 2000, pls send that data to this process
     * bind() is a mechnism to tell OS what kind of data server process is interested in to recieve. Remember, server machine
     * can run multiple server processes to process different data and service different clients. Note that, bind() is
     * used on server side, not on client side*/

    if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("socket bind failed\n");
        return;
    }

    /*Step 4 : Tell the Linux OS to maintain the queue of max length to Queue incoming
     * client connections.*/
    if (listen(master_sock_tcp_fd, 10)<0)
    {
        printf("listen failed\n");
        return;
    }




    /*Add master socket to Monitored set of FDs*/
    add_to_monitored_fd_set(master_sock_tcp_fd);

    /* Server infinite loop for servicing the client*/


    while(1){

        /*Step 5 : initialze and dill readfds*/
        //FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
        re_init_readfds(&readfds);               /*Copy the entire monitored FDs to readfds*/
        //FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

        printf("blocked on select System call...\n");

        /*Step 6 : Wait for client connection*/
        /*state Machine state 1 */
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL); /*Call the select system cal, server process blocks here. Linux OS keeps this process blocked untill the data arrives on any of the file Drscriptors in the 'readfds' set*/

        /*Some data on some fd present in monitored fd set has arrived, Now check on which File descriptor the data arrives, and process accordingly*/

        /*If Data arrives on master socket FD*/
        if (FD_ISSET(master_sock_tcp_fd, &readfds))
        {
            /*Data arrives on Master socket only when new client connects with the server (that is, 'connect' call is invoked on client side)*/
            printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");

            /* step 7 : accept() returns a new temporary file desriptor(fd). Server uses this 'comm_socket_fd' fd for the rest of the
             * life of connection with this client to send and recieve msg. Master socket is used only for accepting
             * new client's connection and not for data exchange with the client*/
            /* state Machine state 2*/
            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            if(comm_socket_fd < 0){

                /* if accept failed to return a socket descriptor, display error and exit */
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }

            add_to_monitored_fd_set(comm_socket_fd);
            printf("Connection accepted from client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        else /* Data srrives on some other client FD*/
        {

            int i = 0, comm_socket_fd = -1;
            for(; i < MAX_CLIENT_SUPPORTED; i++){


                if(FD_ISSET(monitored_fd_set[i], &readfds)){/*Find the clinet FD on which Data has arrived*/

                    comm_socket_fd = monitored_fd_set[i];

                    memset(data_buffer, 0, sizeof(data_buffer));
                    sent_recv_bytes = recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0, (struct sockaddr *)&client_addr, &addr_len);

                    printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes,
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    if(sent_recv_bytes == 0){
                        /*If server recvs empty msg from client, server may close the connection and wait
                         * for fresh new connection from client - same or different*/
                        close(comm_socket_fd);
                        remove_from_monitored_fd_set(comm_socket_fd);
                        break; /*goto step 5*/
                    }
                    if (*data_buffer_ptr0=='D'&&*data_buffer_ptr2=='T') {
                    		meters_to_other_clients = (message_meter *)data_buffer;
                    		meters_to_other_clients_global = *meters_to_other_clients;
                    		//if (meters_to_other_clients_global.meters.per_phase[2].ai>0) {


                    			FILE *fp = fopen ("/home/kuba/Pulpit/AIAE_RETAIN.txt","r+");
//for future                    			calculate_integrals( &fp, integrals retained, integrals compare, integrals actual, integrals to_client)
									fscanf(fp,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld ",
												&ae0_retain, &ai0_retain, &ae1_retain, &ai1_retain,&ae2_retain, &ai2_retain,
												&ae0_cmp, &ai0_cmp, &ae1_cmp, &ai1_cmp, &ae2_cmp, &ai2_cmp); //get ratained values
									rewind(fp);
									printf("actual+1:      %ld \n",meters_to_other_clients_global.meters.per_phase[2].ai);
									if (ai2_cmp<meters_to_other_clients_global.meters.per_phase[2].ai &&
										ai1_cmp<meters_to_other_clients_global.meters.per_phase[1].ai &&
										ai0_cmp<meters_to_other_clients_global.meters.per_phase[0].ai &&
										ae2_cmp<meters_to_other_clients_global.meters.per_phase[2].ae &&
										ae1_cmp<meters_to_other_clients_global.meters.per_phase[1].ae &&
										ae0_cmp<meters_to_other_clients_global.meters.per_phase[0].ae) {

										ae0_to_client = ae0_retain + (meters_to_other_clients_global.meters.per_phase[0].ae-ae0_cmp);
										iko++;
										ae0_cmp = meters_to_other_clients_global.meters.per_phase[0].ae;
										ai0_to_client = ai0_retain + (meters_to_other_clients_global.meters.per_phase[0].ai-ai0_cmp);
										ai0_cmp = meters_to_other_clients_global.meters.per_phase[0].ai;
										ae1_to_client = ae1_retain + (meters_to_other_clients_global.meters.per_phase[1].ae-ae1_cmp);
										ae1_cmp = meters_to_other_clients_global.meters.per_phase[1].ae;
										ai1_to_client = ai1_retain + (meters_to_other_clients_global.meters.per_phase[1].ai-ai1_cmp);
										ai1_cmp = meters_to_other_clients_global.meters.per_phase[1].ai;
										ae2_to_client = ae2_retain + (meters_to_other_clients_global.meters.per_phase[2].ae-ae2_cmp);
										ae2_cmp = meters_to_other_clients_global.meters.per_phase[2].ae;
										ai2_to_client = ai2_retain + (meters_to_other_clients_global.meters.per_phase[2].ai-ai2_cmp);
										ai2_cmp = meters_to_other_clients_global.meters.per_phase[2].ai;
										rewind(fp);
										fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", ae0_to_client, ai0_to_client, ae1_to_client, ai1_to_client, ae2_to_client, ai2_to_client,
																									    ae0_cmp, ai0_cmp, ae1_cmp, ai1_cmp, ae2_cmp, ai2_cmp);

									} else if (ae0_cmp==0 || ai0_cmp==0|| ae1_cmp==0 || ai1_cmp==0 || ae2_cmp==0 || ai2_cmp==0 ||
											meters_to_other_clients_global.meters.per_phase[2].ai==0 || meters_to_other_clients_global.meters.per_phase[1].ai==0 ||
											meters_to_other_clients_global.meters.per_phase[0].ai==0 || meters_to_other_clients_global.meters.per_phase[2].ae==0 ||
											meters_to_other_clients_global.meters.per_phase[1].ae==0 || meters_to_other_clients_global.meters.per_phase[0].ae==0) {

										ae0_to_client = ae0_retain + meters_to_other_clients_global.meters.per_phase[0].ae;
										ae0_cmp = meters_to_other_clients_global.meters.per_phase[0].ae;
										ai0_to_client = ai0_retain + meters_to_other_clients_global.meters.per_phase[0].ai;
										ai0_cmp = meters_to_other_clients_global.meters.per_phase[0].ai;
										ae1_to_client = ae1_retain + meters_to_other_clients_global.meters.per_phase[1].ae;
										ae1_cmp = meters_to_other_clients_global.meters.per_phase[1].ae;
										ai1_to_client = ai1_retain + meters_to_other_clients_global.meters.per_phase[1].ai;
										ai1_cmp = meters_to_other_clients_global.meters.per_phase[1].ai;
										ae2_to_client = ae2_retain + meters_to_other_clients_global.meters.per_phase[2].ae;
										ae2_cmp = meters_to_other_clients_global.meters.per_phase[2].ae;
										ai2_to_client = ai2_retain + meters_to_other_clients_global.meters.per_phase[2].ai;
										ai2_cmp = meters_to_other_clients_global.meters.per_phase[2].ai;
										jko++;
										printf("actual:      %ld \n",meters_to_other_clients_global.meters.per_phase[0].ae);
										rewind(fp);
										fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", ae0_to_client, ai0_to_client, ae1_to_client, ai1_to_client, ae2_to_client, ai2_to_client,
																										ae0_cmp, ai0_cmp, ae1_cmp, ai1_cmp, ae2_cmp, ai2_cmp);
									}
									//to client retained integrals:
									integral_values_to_client.ae0 = ae0_to_client;
									integral_values_to_client.ai0 = ai0_to_client;
									integral_values_to_client.ae1 = ae1_to_client;
									integral_values_to_client.ai1 = ai1_to_client;
									integral_values_to_client.ae2 = ae2_to_client;
									integral_values_to_client.ai2 = ai2_to_client;
									printf("Ilosc wykonan petli: i:%d, j:%d",iko,jko);
									fclose(fp);

									for(uint8_t i=0;i<3;i++) {
										if( meters_to_other_clients_global.meters.per_phase[i].v<global_uimax_uimin.umin[i] || global_uimax_uimin.umin[0]==0 ) {global_uimax_uimin.umin[i] = meters_to_other_clients_global.meters.per_phase[i].v;}
										if( meters_to_other_clients_global.meters.per_phase[i].i<global_uimax_uimin.imin[i] || global_uimax_uimin.imin[0]==0 ) {global_uimax_uimin.imin[i] = meters_to_other_clients_global.meters.per_phase[i].i;}
										if( meters_to_other_clients_global.meters.per_phase[i].v>global_uimax_uimin.umax[i]) {global_uimax_uimin.umax[i] = meters_to_other_clients_global.meters.per_phase[i].v;}
										if( meters_to_other_clients_global.meters.per_phase[i].i>global_uimax_uimin.imax[i]) {global_uimax_uimin.imax[i] = meters_to_other_clients_global.meters.per_phase[i].i;}
									}


                    } else if (*data_buffer_ptr0=='G') { //get
                    	if ( *data_buffer_ptr1=='M') {
                    		//get meter number (8 digits) and meter_srv version__==
                    		char *gm1="GM";
                    		char *identificator_msg_ptr;
                    		prepare_irentificator_frame(&identificator_msg_ptr,gm1,METER_NUMBER,VERSION_NUMBER);//"GM12345678aaa";//
                    		sent_recv_bytes = sendto(comm_socket_fd, identificator_msg_ptr, 13, 0,
                    		                                (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    		free(identificator_msg_ptr);
                        	printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);

                    	} else if (*data_buffer_ptr1=='A') {
                    		//get instantaneous per-phase values
                    		meters_to_other_clients = &meters_to_other_clients_global;
                    		uint32_t volts = meters_to_other_clients->meters.per_phase->v;
                    		printf("VOLTS: %d\n",volts);
                    		sent_recv_bytes = sendto(comm_socket_fd, meters_to_other_clients, sizeof(message_meter), 0,
                    		                   	   	(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    	} else if (*data_buffer_ptr1=='S') {
                    		//get per-phase time integral A+ and A- (sum of all phases)

                    		prepared_msg_ptr=&prepared_msg;
                    		prepared_msg_ptr->identity[0]='G';
                    		prepared_msg_ptr->identity[1]='S';
							prepared_msg_ptr->integrals_retained.ae0 = integral_values_to_client.ae0;
							prepared_msg_ptr->integrals_retained.ai0 = integral_values_to_client.ai0;
							prepared_msg_ptr->integrals_retained.ae1 = integral_values_to_client.ae1;
							prepared_msg_ptr->integrals_retained.ai1 = integral_values_to_client.ai1;
							prepared_msg_ptr->integrals_retained.ae2 = integral_values_to_client.ae2;
							prepared_msg_ptr->integrals_retained.ai2 = integral_values_to_client.ai2;

                    		//prepare_integrals_msg(integral_values_to_client,&prepared_msg_ptr);

                    		sent_recv_bytes = sendto(comm_socket_fd, prepared_msg_ptr, sizeof(prepared_msg), 0,
                    		                    		(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    	} else if (*data_buffer_ptr1=='R') {
                    		//get angles
                    	} else if (*data_buffer_ptr1=='X') {
                    		//get min/max per-phase voltage and current since last reset

                    		minmax_msg_ptr = &minmax_msg;

                    		prepare_uiminmax_msg(global_uimax_uimin,&minmax_msg_ptr);
                    		printf("vmax:%d\n",minmax_msg_ptr->voltages.umax[0]);
                    		sent_recv_bytes = sendto(comm_socket_fd, minmax_msg_ptr, sizeof(minmax_msg), 0,
                    		                    		                    		(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
                    	}



                    } else if (*data_buffer_ptr0=='S') {
                    	//set thresholds and send to all clients
                    } else if (*data_buffer_ptr0=='R') {
                     	//reset thresholds and send to all clients
                    } else if (*data_buffer_ptr0=='A') {
                    	//abort socket
                    } else {
                    	//error handling
                    }



                }
            }
        }

    }/*step 10 : wait for new client request again*/
}

int
main(int argc, char **argv){

    setup_tcp_server_communication();
    return 0;
}
