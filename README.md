# Branch Prediction Simulator (Bimodal/Gshare/Hybrid)
This is a C++-based simulator that does branch prediction performance analysis for bimodal/gshare or hybrid configuration

## Overview
- The model runs through a trace file which has the addresses of the branch instructions and their actual results (taken / not taken).
- During runtime, the model accepts the following parameters:
   - model: bimodal, gshare or hybrid 
   - K - number of lower order PC bits to index the chooser table in hybrid mode
   - M1 - number of lower order PC bits to index the gshare counters
   - N - length of the Global branch history register
   - M2 - number of lower order PC bits to index the bimodal counters
   - trace file
- It always assumes that the N is smaller than M1 (discussed later)

## Features
- prints out the state of all counters at the end of the run along with the misprediction rate
- For gshare, the upper order of selected PC bits are XORed with the GBHR before indexing into the counters to get a prediction.
- All counters are 2-bit counters (both for prediction and in the chooser table). All counters saturate at their extreme values and do not roll-over. By default, they are set to 2 (weakly taken)
- When using hybrid model, in the chooser table if the counter is >= 2, gshare is choosen otherwise bimodal model is choosen for prediction.
- Also included are python scripts to plot data for comparing misprediction rates with various configurations to better understand the impact of each parameter.

## Instructions
1. Type "make" to build.  (Type "make clean" first if you already compiled and want to recompile from scratch.)
2. To run simulator:
   - bimodal - ./sim bimodal 6 gcc_trace.txt
   - gshare - ./sim gshare 9 3 gcc_trace.txt
   - hybrid - ./sim hybrid 8 14 10 5 gcc_trace.txt
