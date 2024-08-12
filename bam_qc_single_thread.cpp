#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "BamTools.h"
#include "BamReader.h"
#include "BamStatus.h"


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
    std::cout << "Total thread number: " << n_thread << "\n";
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

    BamReader *reader = new BamReader(input_file, n_thread, true);


    BamStatus *status = new BamStatus;
    bam1_t *b;
    if ((b = bam_init1()) == NULL) {
        fprintf(stderr, "[E::%s] Out of memory allocating BAM struct.\n", __func__);

    }
    long long num = 0;
    while (reader->getBam1_t(b)) {
        num++;
        status->statusbam(b);
    }

    printf("num %lld\n", num);
    printf("time %lf\n", GetTime() - t0);
    status->reportHTML(&fout);
    sam_close(sin);

    return 0;
}
