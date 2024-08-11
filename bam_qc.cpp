#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "BamTools.h"
#include "BamReader.h"
#include "BamStatus.h"
#include <omp.h>


const int read_thread_num = 12;

const int round_size = 256;

int main(int argc, char* argv[]) {
	std::string input_file;
    std::string output_file;
    int n_thread = 1;

    int opt;
    while ((opt = getopt(argc, argv, "i:t:o:")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 't':
                n_thread = std::atoi(optarg);
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -i input_file -t thread_num -o output_file\n";
                return EXIT_FAILURE;
        }
    }

    if (input_file.empty() || output_file.empty() || n_thread <= 0) {
        std::cerr << "Invalid arguments. Usage: " << argv[0] << " -i input_file -t thread_num -o output_file\n";
        return EXIT_FAILURE;
    }

    std::cout << "Input file: " << input_file << "\n";
    std::cout << "Thread number: " << n_thread << "\n";
    std::cout << "Output file: " << output_file << "\n";

    printf("Starting Running Bam Analyze\n");
    samFile *sin;
    sam_hdr_t *hdr;
    ofstream fout;
    fout.open(output_file);
    if ((sin = sam_open(input_file.c_str(), "r")) == NULL) {
        printf("Can`t open this file!\n");
        return 0;
    }
    if ((hdr = sam_hdr_read(sin)) == NULL) {
        return 0;
    }



    double t0 = GetTime();
    n_thread -= read_thread_num;
    if(n_thread <= 0) n_thread = 1;

    std::cout << "Thread number: " << n_thread << "\n";

    
    BamReader *reader = new BamReader(input_file, n_thread > read_thread_num ? read_thread_num : n_thread, false);


    bam1_t *b[n_thread];
    BamStatus *status[n_thread];

    for(int i = 0; i < n_thread; i++) {
        if ((b[i] = bam_init1()) == NULL) {
            fprintf(stderr, "[E::%s] Out of memory allocating BAM struct.\n", __func__);
        }
        status[i] = new BamStatus;
    }

    long long nums[n_thread] = {0};
    bam_complete_block *BGZFBlock;
    bam_complete_block* b_vec[round_size];
    bool done = false;
    while(!done) {
        int not_null_size = round_size;
        for(int i = 0; i < round_size; i++) {
            BGZFBlock = reader->getBamCompleteClock();
            if(BGZFBlock == nullptr) {
                done = true;
                not_null_size = i;
                break;
            }
            b_vec[i] = BGZFBlock;
        }
#pragma omp parallel for num_threads(n_thread)
        for(int i = 0; i < not_null_size; i++) {
            int tid = omp_get_thread_num();
            int ret = 0;
            nums[tid] += b_vec[i]->data_size;
            while((ret = (read_bam(b_vec[i], b[tid], 0))) >= 0) {
                nums[tid]++;
                status[tid]->statusbam(b[tid]);
            }
        }
        
        for(int i = 0; i < not_null_size; i++) {
            reader->backBamCompleteBlock(b_vec[i]);
        }
    }

    long long num = nums[0];
    for(int i = 1; i < n_thread; i++) {
        status[0]->add(status[i]);
        num += nums[i];
    }

    printf("num %lld\n", num);
    printf("time %lf\n", GetTime() - t0);
    status[0]->statusAll();
    status[0]->reportHTML(&fout);
    //status->print();
    sam_close(sin);

    return 0;
}
