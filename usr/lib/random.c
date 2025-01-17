#include "usersp.h"
#include "rng.h"

#define STATE_SIZE  312
#define MIDDLE      156
#define INIT_SHIFT  62
#define TWIST_MASK  0xb5026f5aa96619e9
#define INIT_FACT   6364136223846793005
#define SHIFT1      29
#define MASK1       0x5555555555555555
#define SHIFT2      17
#define MASK2       0x71d67fffeda60000
#define SHIFT3      37
#define MASK3       0xfff7eee000000000
#define SHIFT4      43
 
#define LOWER_MASK  0x7fffffff
#define UPPER_MASK  (~(uint64_t)LOWER_MASK)
static uint64_t mt_state_[STATE_SIZE];
static size_t mt_index_ = STATE_SIZE + 1;
 
void srng(uint64_t s) {
	mt_index_ = STATE_SIZE;
	mt_state_[0] = s;
	for(size_t i = 1; i < STATE_SIZE; i++)
		mt_state_[i] = (INIT_FACT * (mt_state_[i - 1] ^ (mt_state_[i - 1] >> INIT_SHIFT))) + i;
}
 
static void mt_twist_(void) {
    for(size_t i = 0; i < STATE_SIZE; i++) {
        uint64_t x = (mt_state_[i] & UPPER_MASK) | (mt_state_[(i + 1) % STATE_SIZE] & LOWER_MASK);
        x = (x >> 1) ^ (x & 1? TWIST_MASK : 0);
        mt_state_[i] = mt_state_[(i + MIDDLE) % STATE_SIZE] ^ x;
    }
    mt_index_ = 0;
}
 
uint64_t grng(void) {
    if(mt_index_ >= STATE_SIZE) mt_twist_();
    uint64_t y = mt_state_[mt_index_];
    y ^= (y >> SHIFT1) & MASK1;
    y ^= (y << SHIFT2) & MASK2;
    y ^= (y << SHIFT3) & MASK3;
    y ^= y >> SHIFT4;
    mt_index_ += 1;
    return y;
}

uint64_t rngmax(void) { return UINT64_MAX; }

static const double dpf_rec_rngmax = 1.0 / (double)UINT64_MAX;
double grngdpf(void) { return (double)grng() * dpf_rec_rngmax; }
float grngspf(void) { return (float)grngdpf(); }
