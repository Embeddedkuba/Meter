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
    struct meter_hw_registers_s meters; 
    char terminator;
}message_meter;


