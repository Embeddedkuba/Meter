/*
 * protocol.h
 *
 *  Created on: 7 lis 2020
 *      Author: kuba
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <string.h>
typedef struct meter_version_number {
	char meter_number[8];
	char version_number[3];
} identity;
typedef struct identity_frame {
	char  frame_type[2];
	identity identity_numbers;
} identity_msg;



//server
integrals_msg prepare_integrals_msg(integrals set) {
	integrals_msg prepared_msg;
	prepared_msg.identity[0]='G';
	prepared_msg.identity[1]='S';
	prepared_msg.integrals_retained = (integrals)set;

	return prepared_msg;
}
void prepare_irentificator_frame(char **msg,char * msg_type,char * meternumber,char * version_number) {
	char *result = malloc(14);
	snprintf(result, 14, "%s%s%s", msg_type, meternumber,version_number);
	*msg = result;
}

void get_identificator_msg(identity_msg data_buffer) {
	printf("Meter Number: %.8s\n",data_buffer.identity_numbers.meter_number);
	printf("Version Number: %.3s\n",data_buffer.identity_numbers.version_number);
}
void get_instant_data(message_meter *received_instant_data) {
	 printf("VOLTS0: %d	Ampers0: %d\n"
			"VOLTS1: %d Ampers0: %d\n"
			"VOLTS2: %d Ampers0: %d\n"
			"AI0:     %ld AE0:      %ld\n"
			"AI1:     %ld AE1:      %ld\n"
			"AI2:     %ld AE2:      %ld\n",
			received_instant_data->meters.per_phase[0].v,
			received_instant_data->meters.per_phase[0].i,
			received_instant_data->meters.per_phase[1].v,
			received_instant_data->meters.per_phase[1].i,
			received_instant_data->meters.per_phase[2].v,
			received_instant_data->meters.per_phase[2].i,
			received_instant_data->meters.per_phase[0].ai,
			received_instant_data->meters.per_phase[0].ae,
			received_instant_data->meters.per_phase[1].ai,
			received_instant_data->meters.per_phase[1].ae,
			received_instant_data->meters.per_phase[2].ai,
			received_instant_data->meters.per_phase[2].ae
	 );
}
void get_current_angles(message_meter *received_instant_data) {
	 printf("Current Angle0: %d Voltage Angle0: %d\n"
			"Current Angle1: %d Voltage Angle1: %d\n"
			"Current Angle2: %d Voltage Angle2: %d\n",
			received_instant_data->meters.current_angles[0],
			received_instant_data->meters.voltage_angles[0],
			received_instant_data->meters.current_angles[1],
			received_instant_data->meters.voltage_angles[1],
			received_instant_data->meters.current_angles[2],
			received_instant_data->meters.voltage_angles[2]
	 );
}
void get_retained_integrals(integrals_msg *received_retained_data) {
	printf("Ae0:%ld Ai0:%ld\n"
		   "Ae1:%ld Ai1:%ld\n"
		   "Ae2:%ld Ai2:%ld\n",
		   received_retained_data->integrals_retained.ae0,
		   received_retained_data->integrals_retained.ai0,
		   received_retained_data->integrals_retained.ae1,
		   received_retained_data->integrals_retained.ai1,
		   received_retained_data->integrals_retained.ae2,
		   received_retained_data->integrals_retained.ai2);
}
void get_uimaxmin(uimax_uimin_msg *uidata) {
	printf( "Umax per L1:%d Umax per L2:%d Umax per L2:%d\n"
			"Umin per L1:%d Umin per L2:%d Umin per L2:%d\n"
			"Imax per L1:%d Imax per L2:%d Imax per L2:%d\n"
			"Imin per L1:%d Imin per L2:%d Imin per L2:%d\n",
			uidata->voltages.umax[0], uidata->voltages.umax[1], uidata->voltages.umax[2],
			uidata->voltages.umin[0], uidata->voltages.umin[1], uidata->voltages.umin[2],
			uidata->voltages.imax[0], uidata->voltages.imax[1], uidata->voltages.imax[2],
			uidata->voltages.imin[0], uidata->voltages.imin[1], uidata->voltages.imin[2]);
}
#endif /* PROTOCOL_H_ */
