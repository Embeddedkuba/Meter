/*
 * common.h
 *
 *  Created on: 7 lis 2020
 *      Author: kuba
 */

#ifndef COMMON_H_
#define COMMON_H_

#define VERSION_NUMBER "AAA"
#define METER_NUMBER "12345678"

typedef struct _test_struct{

    unsigned int a;
    unsigned int b;
} test_struct_t;


typedef struct result_struct_{

    unsigned int c;

} result_struct_t;

struct meter_hw_registers_per_phase_s {
    uint32_t v;     /* instatntenous voltage in mV */
    uint32_t i;     /* instatntenous current in mA */
    uint64_t ai;    /* A+ since start in Ws */
    uint64_t ae;    /* A- since start in Ws */
};

#define PHASE_CNT 3


typedef struct meter_hw_registers_s {
    struct meter_hw_registers_per_phase_s per_phase[PHASE_CNT];
    uint32_t voltage_angles[3]; /* [0] -> L1-L2, [1] -> L2-L3, [3] -> L3-L1 */
    uint32_t current_angles[3]; /* [0] -> L1-L2, [1] -> L2-L3, [3] -> L3-L1 */
}meter_hw_registers_t;

typedef struct message_meter_hw_registers_t {
    char identity;
    meter_hw_registers_t meters;
}message_meter;

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


//for GX
typedef struct min_max_voltage {
	uint32_t umin[PHASE_CNT];
	uint32_t umax[PHASE_CNT];
	uint32_t imin[PHASE_CNT];
	uint32_t imax[PHASE_CNT];
}uimax_uimin_meters;
typedef struct minmaxframe { //GX
	char identity[2];
	uimax_uimin_meters voltages;
}uimax_uimin_msg;

#endif /* COMMON_H_ */
