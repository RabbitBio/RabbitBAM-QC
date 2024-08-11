#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "BamTools.h"
#include "BamReader.h"
#include "BamStatus.h"
#include <omp.h>


const int read_thread_num = 8;

const int round_size = 256;


void process_bam_blocks(BamReader* reader, long long &num, BamStatus* status) {
	bam1_t* b;
	if ((b = bam_init1()) == NULL) {
		fprintf(stderr, "[E::%s] Out of memory allocating BAM struct.\n", __func__);
	}
		
	bam_complete_block* BGZFBlock = nullptr;
	long long local_num = 0;
	while (true) {
		BGZFBlock = reader->getBamCompleteClock();
		if (BGZFBlock == nullptr) {
			break;
		}
		while (read_bam(BGZFBlock, b, 0) >= 0) {
			local_num++;
			status->statusbam(b);
		}
		reader->backBamCompleteBlock(BGZFBlock);
	}
	num = local_num;
}

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

	BamReader *reader = new BamReader(input_file, n_thread > read_thread_num ? read_thread_num : n_thread, false);


	n_thread -= read_thread_num;
	if(n_thread <= 0) n_thread = 1;

	std::cout << "QC thread number: " << n_thread << "\n";


	BamStatus *status[n_thread];
	for(int i = 0; i < n_thread; i++) {
		status[i] = new BamStatus;
	}

	long long nums[n_thread] = {0};

    std::thread **threads= new thread *[n_thread];
	for (int t = 0; t < n_thread; t++) {
        //cpu_set_t cpuset;
        //CPU_ZERO(&cpuset);
        //CPU_SET(t, &cpuset);
        threads[t] = new thread(process_bam_blocks, reader, std::ref(nums[t]), status[t]);
        //int rc = pthread_setaffinity_np(threads[t]->native_handle(), sizeof(cpu_set_t), &cpuset);
    }
    for (int t = 0; t < n_thread; t++) {
        if (threads[t]->joinable()) {
            threads[t]->join();
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
