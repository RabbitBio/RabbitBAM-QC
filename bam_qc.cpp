#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "BamTools.h"
#include "BamReader.h"
#include "BamStatus.h"


//#define use_read_parallel

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


#ifdef use_read_parallel
    int my_thread;
    if(n_thread > 8) my_thread = 8;
    else my_thread = n_thread;
    printf("init rbam reader\n");
    BamReader *reader = new BamReader(input_file, n_thread, false);
    const int RN = 40 << 10;
    std::vector<bam1_t*> b_vec[THREAD_NUM_P];
    for(int i = 0; i < THREAD_NUM_P; i++) {
        for(int j = 0; j < RN; j++) {
            bam1_t *item = bam_init1();
            b_vec[i].push_back(item);
        }
    }
#else
    BamReader *reader = new BamReader(input_file, n_thread, false);
#endif

    //BamStatus *status = new BamStatus(input_file);

    bam1_t *b;
    if ((b = bam_init1()) == NULL) {
        fprintf(stderr, "[E::%s] Out of memory allocating BAM struct.\n", __func__);
    }

    double t0 = GetTime();

    long long num = 0;
#ifdef use_read_parallel
    while(true) {
        auto res_vec = reader->getBam1_t_parallel(b_vec);
        int res_vec_size = res_vec.size();
        if (res_vec_size == 0) break;
        for(int i = 0; i < res_vec_size; i++) {
            num++;
        }
    }
#else
    bam_complete_block *BGZFBlock = new bam_complete_block;
    //while (reader->getBam1_t(b)) {
    //    num++;
    //    //status->statusbam(b);
    //}
    while((BGZFBlock = reader->getBamCompleteClock()) != nullptr) {
        int ret = 0;
        while((ret = (read_bam(BGZFBlock, b, 0))) >= 0) {
            num++;
        }
        reader->backBamCompleteBlock(BGZFBlock);
    }
#endif

    printf("num %lld\n", num);
    printf("time %lf\n", GetTime() - t0);
    //status->statusAll();
    //status->reportHTML(&fout);
    sam_close(sin);

    return 0;
}
