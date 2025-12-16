#include "sim_bp.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
uint32_t count = 0;
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
	Chooser* model;
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
		model = new Chooser(0, 0, 0, params.M2, 2);
        printf("COMMAND\n %s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
		//std::cout << "N = " << params.N << "\n";
		model = new Chooser(0, params.M1, params.N, 0, 2);
        printf("COMMAND\n %s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
		model = new Chooser(params.K, params.M1, params.N, params.M2, 2);
        printf("COMMAND\n %s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    //create chooser table and predictors
	
    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        if (outcome == 't') {
			//if(count < 10000) {
            //	printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
			//}
			model->predict(addr, true);
        } else if (outcome == 'n') {
			//if(count < 10000){
            //	printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly
			//}
			model->predict(addr, false);
		}
        /*************************************
            Add branch predictor code here
        **************************************/
    }
	model->print_stats();
	delete model;
    return 0;
}

result Predictor::predict(uint32_t addr, uint32_t gbhr) {
	uint32_t index = decode_ind(addr);
	uint32_t gbhr_shift = (gbhr << xor_shift);
	if(!is_bim) {
		index = (index ^ gbhr_shift) & index_mask;
	}
	if(dbg && count < 10000) {
		std::cout << "GP: " << index << " " << pred_table[index].r() << "\n";
		count++;
	}
	if(pred_table[index].r() >= level)
		return {index, true};
	return {index, false};
}

uint32_t Predictor::decode_ind(uint32_t addr) {
	return ((addr >> 2) & index_mask);
}

void Predictor::update_table(uint32_t index, bool taken) {
	if(taken) {
		pred_table[index].inc();
	} else {
		pred_table[index].dec();
	}
	if(dbg && count < 10000) {
		std::cout << "GU: " << index << " " << pred_table[index].r() << "\n";
		count++;
	}
}

//Chooser table
uint32_t Chooser::decode_ind(uint32_t addr) {
	return ((addr >> 2) & index_mask);
}

void Chooser::predict(uint32_t addr, bool taken) {
	predictions++;
	uint32_t chooser_index = decode_ind(addr);
	std::vector<result> model_res;
	for (size_t i = 0; i < chooser_models.size(); i++) {
		result temp = chooser_models[i].predict(addr, gbhr);
		model_res.emplace_back(temp);
	}
	if(chooser_table[chooser_index].r() < level) {
		if(model_res[0].taken != taken)
			mispredictions++;
		chooser_models[0].update_table(model_res[0].index, taken);
	} else {
		if(model_res[1].taken != taken)
			mispredictions++;
		chooser_models[1].update_table(model_res[1].index, taken);
	}
	if(chooser_models.size() > 1) {
		if(model_res[0].taken != model_res[1].taken) {
			if(model_res[0].taken == taken) {
				chooser_table[chooser_index].dec();
			} else {
				chooser_table[chooser_index].inc();
			}
		}
	}
	if(taken)
		gbhr = gbhr + (1u << gbhr_append);
	gbhr = gbhr >> 1;
	gbhr = gbhr & gbhr_mask;
}

void Chooser::print_stats() {
	std::cout << "OUTPUT\n";
	std::cout << std::dec;
	std::cout << std::left;
	std::cout << std::setw(27) << " number of predictions: " << predictions << "\n";
	std::cout << std::setw(27) << " number of mispredictions: " << mispredictions << "\n";
	std::cout << std::setw(27) << " misprediction rate: " << std::fixed << std::setprecision(2) << (static_cast<float>(mispredictions*100)/predictions) << "%\n";
	if(index_mask != 0) {
		std::cout << "FINAL CHOOSER CONTENTS\n";
		for(size_t i = 0; i < chooser_table.size(); i++) {
			std::cout << std::setw(8) << " " << i << chooser_table[i].r() << "\n";
		}
	}
	//std::cout << "size = " << chooser_models.size() << "\n";
	for(size_t j = chooser_models.size(); j > 0; j--) {
		if(chooser_models[j-1].is_bim) {
			std::cout << "FINAL BIMODAL CONTENTS\n";
			for(size_t k = 0; k < chooser_models[j-1].pred_table.size(); k++) {
				std::cout << " " << std::setw(7) << k << chooser_models[j-1].pred_table[k].r() << "\n";
			}
		} else {
			std::cout << "FINAL GSHARE CONTENTS\n";
			for(size_t k = 0; k < chooser_models[j-1].pred_table.size(); k++) {
				std::cout << " " << std::setw(7) << k << chooser_models[j-1].pred_table[k].r() << "\n";
			}
		}
	}
}
