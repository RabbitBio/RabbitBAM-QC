# Paths
HTSLIB_INSTALL_PATH=/home/user_home/ylf/someGit/rbam-1.20/htslib-1.20-install
RABBITBAM_INSTALL_PATH=/home/user_home/ylf/RabbitBAM
LIBDEFLATE_INSTALL_PATH=/home/user_home/ylf/someGit/rbam-1.20/libdeflate-1.20-install

# Compiler and flags
CXX=g++
CXXFLAGS=-O3 -g -ffast-math -std=c++11 -fopenmp

# Include directories
INCLUDES = -I$(HTSLIB_INSTALL_PATH)/include \
           -I$(RABBITBAM_INSTALL_PATH) \
           -I$(RABBITBAM_INSTALL_PATH)/htslib \
           -I$(LIBDEFLATE_INSTALL_PATH)/include \
           -I$(LIBDEFLATE_INSTALL_PATH)

# Library directories
LIBS = -L$(HTSLIB_INSTALL_PATH)/lib \
       -L$(RABBITBAM_INSTALL_PATH) \
       -L$(LIBDEFLATE_INSTALL_PATH)/lib \
       -L$(LIBDEFLATE_INSTALL_PATH)/lib64

# Libraries to link
LDFLAGS = $(LIBS) -lhts -lz -fopenmp -lpthread -lrabbitbamtools -lrabbitbamread -lrabbitbamwrite -lstdc++

# Final flags
CXXFLAGS += $(INCLUDES)

# Automatically find all .cpp files
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# Source files for bam_qc_single_thread
SINGLE_THREAD_SRCS = bam_qc_single_thread.cpp
SINGLE_THREAD_OBJS = $(SINGLE_THREAD_SRCS:.cpp=.o)

# Source files for bam_qc (excluding bam_qc_single_thread)
BAM_QC_SRCS = $(filter-out bam_qc_single_thread.cpp, $(SRCS))
BAM_QC_OBJS = $(BAM_QC_SRCS:.cpp=.o)

# Example target
all: bam_qc bam_qc_single_thread

bam_qc: $(BAM_QC_OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

bam_qc_single_thread: $(filter-out bam_qc.o, $(OBJS))
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o bam_qc bam_qc_single_thread

