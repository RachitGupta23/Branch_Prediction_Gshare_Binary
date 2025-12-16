#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <inttypes.h>
#include <cmath>
#include <cassert>
#include <iomanip>
#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement

#endif

class _2bcounter {
private:
uint32_t val;

public:
_2bcounter(uint32_t init) {
	if(init <= 3)
		val = init;
	else
		val = 0;
}

void inc() {
	if(val < 3)
		val++;
}

void dec() {
	if(val > 0)
		val--;
}
uint32_t r() {//read counter
	return val;
}
};

struct result {
	uint32_t index = 0;
	bool taken = false;
};

class Predictor {
public:
std::vector<_2bcounter> pred_table;
uint32_t xor_shift = 0;
uint32_t index_mask = 0;
uint32_t level = 2;
bool is_bim = false;
bool dbg = false;
//uint32_t count = 0;

Predictor(uint32_t M, uint32_t N = 0, bool bim = false, uint32_t lev = 2) {
	//FIXME for gselect
	uint32_t num_count = 1u << M;	
	pred_table.resize(num_count, _2bcounter(2));
	is_bim = bim;
	xor_shift = M - N;
	index_mask = (M == 0) ? 0 : 0xFFFFFFFF >> (32 - M);
	level = lev;
}
result predict(uint32_t addr, uint32_t gbhr);
void update_table(uint32_t index, bool taken);
uint32_t decode_ind(uint32_t addr);
};

class Chooser {
public:
std::vector<Predictor> chooser_models;
std::vector<_2bcounter> chooser_table;
uint32_t index_mask = 0;
uint32_t gbhr = 0;
uint32_t level = 2;
uint32_t gbhr_mask = 0;
uint32_t gbhr_append = 0;
uint32_t predictions = 0;
uint32_t mispredictions = 0;
bool dbg = false;

Chooser(uint32_t K, uint32_t M1 = 0, uint32_t N = 0, uint32_t M2 = 0, uint32_t lev = 2) {
	uint32_t num_count = 1u << K;
	chooser_table.resize(num_count, _2bcounter(1));
	index_mask = (K == 0) ? 0 : 0xFFFFFFFF >> (32 - K);
	gbhr_mask = (N == 0) ? 0 : 0xFFFFFFFF >> (32 - N);
	gbhr_append = N;
	if ((M1 > 0) && (M2 > 0)) {	
		Predictor temp(M2, 0, true, 2);
		Predictor temp1(M1, N, false, 2);
		chooser_models.emplace_back(temp);
		chooser_models.emplace_back(temp1);
		level = lev;
	} else {
		if ((M1 == 0) && (M2 > 0)) {
			Predictor temp(M2, 0, true, 2);
			chooser_models.emplace_back(temp);
		} else if ((M1 > 0) && (M2 == 0)) {
			Predictor temp(M1, N, false, 2);
			chooser_models.emplace_back(temp);
		} else {
			assert(!"ERROR: invalid M1 M2 values");
		}
		level = 4;
	}
}
void predict(uint32_t addr, bool taken);
uint32_t decode_ind(uint32_t addr);
void print_stats();
};
