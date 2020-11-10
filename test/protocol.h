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
typedef struct aiae {
	uint64_t ae0;
	uint64_t ai0;
	uint64_t ae1;
	uint64_t ai1;
	uint64_t ae2;
	uint64_t ai2;
}integrals;

typedef struct aiaeframe {
	char identity[2]; //GS
	integrals integrals_retained;
}integrals_msg;
//server
void prepare_integrals_msg(integrals set, integrals_msg *prepared_msg2) {

	prepared_msg2->identity[0]='G';
	prepared_msg2->identity[1]='S';
	prepared_msg2->integrals_retained.ae0 = set.ae0;
	prepared_msg2->integrals_retained.ai0 = set.ai0;
	prepared_msg2->integrals_retained.ae1 = set.ae1;
	prepared_msg2->integrals_retained.ai1 = set.ai1;
	prepared_msg2->integrals_retained.ae2 = set.ae2;
	prepared_msg2->integrals_retained.ai2 = set.ai2;

}
void prepare_uiminmax_msg(uimax_uimin_meters set, uimax_uimin_msg **prepared_minmax) {
	uimax_uimin_msg *result = malloc(sizeof(uimax_uimin_msg));
	result->identity[0]='G';
	result->identity[1]='X';
	for(uint8_t i=0;i<3;++i) {
		result->voltages.imax[i]=set.imax[i];
		result->voltages.umax[i]=set.umax[i];
		result->voltages.imin[i]=set.imin[i];
		result->voltages.umin[i]=set.umin[i];
	}
	*prepared_minmax=result;
}
void prepare_irentificator_frame(char **msg, char * msg_type,char * meternumber,char * version_number) {
	char *result = malloc(14);
	snprintf(result, 14, "%s%s%s", msg_type, meternumber,version_number);
	*msg = result;
}

void get_identificator_msg(identity_msg data_buffer) {
	printf("Meter Number: %.8s\n",data_buffer.identity_numbers.meter_number);
	printf("Version Number: %.3s\n",data_buffer.identity_numbers.version_number);
}

//void calculate_integrals( FILE * fp, integrals retained, integrals compare, integrals actual, integrals to_client)
//{
//	fscanf(fp,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
//				&retained.ae0, &retained.ai0, &retained.ae1, &retained.ai1,&retained.ae2, &retained.ai2,
//				&compare.ae0, &compare.ai0, &compare.ae1, &compare.ai1, &compare.ae2, &compare.ai2);
//	rewind(fp);
//	if (compare.ae0 < actual.ae0 ||	compare.ai0 < actual.ai0 || compare.ae1 < actual.ae1 ||
//		compare.ai1 < actual.ai1 || compare.ae2 < actual.ae2 || compare.ai2 < actual.ai2) {
//		to_client.ae0 = retained.ae0 + (actual.ae0-compare.ae0);
//		compare.ae0 =actual.ae0;
//		to_client.ai0 = retained.ai0 + (actual.ai0-compare.ai0);
//		compare.ai0 =actual.ai0;
//		to_client.ae1 = retained.ae1 + (actual.ae1-compare.ae1);
//		compare.ae1 =actual.ae1;
//		to_client.ai1 = retained.ai1 + (actual.ai1-compare.ai1);
//		compare.ai1 =actual.ai1;
//		to_client.ae2 = retained.ae2 + (actual.ae2-compare.ae2);
//		compare.ae2 =actual.ae2;
//		to_client.ai2 = retained.ai2 + (actual.ai2-compare.ai2);
//		compare.ai2 =actual.ai2;
//		rewind(fp);
//		fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", to_client.ae0, to_client.ai0, to_client.ae1, to_client.ai1, to_client.ae2, to_client.ae2,
//																		compare.ae0, compare.ai0, compare.ae1, compare.ai1, compare.ae2, compare.ai2);
//	} else if (compare.ae0==0 || compare.ai0==0|| compare.ae1==0 || compare.ai1==0 || compare.ae2==0 || compare.ai2==0 || actual.ae0==0 || actual.ai0==0 || actual.ae1==0 ||
//				actual.ai1==0 || actual.ae2==0 || actual.ai2==0) {
//		to_client.ae0 = retained.ae0 + actual.ae0;
//		compare.ae0 = actual.ae0;
//		to_client.ai0 = retained.ai0 + actual.ai0;
//		compare.ai0 = actual.ai0;
//		to_client.ae1 = retained.ae1 + actual.ae1;
//		compare.ae1 = actual.ae1;
//		to_client.ai1 = retained.ai1 + actual.ai1;
//		compare.ai1 = actual.ai1;
//		to_client.ae2 = retained.ae2 + actual.ae2;
//		compare.ae2 = actual.ae2;
//		to_client.ai2 = retained.ai2 + actual.ai2;
//		compare.ai2 = actual.ai2;
//		rewind(fp);
//		fprintf(fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", to_client.ae0, to_client.ai0, to_client.ae1, to_client.ai1, to_client.ae2, to_client.ae2,
//																		compare.ae0, compare.ai0, compare.ae1, compare.ai1, compare.ae2, compare.ai2);
//	}
//}
//uint8_t alarm_threshold(thresholds_msg msg,umax_umin_meters actual_thresholds,matrix_of_states *matrix) {
//	alarm = 0; //0- all fine, 1-under, 2-over, 3 - both
//	uint8_t i=0;
//	for(i=0;i<PHASE_CNT;i++) {
//		if ()
//	}
//
//	return alarm;
//}
#endif /* PROTOCOL_H_ */
