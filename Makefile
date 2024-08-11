# Paths
HTSLIB_INSTALL_PATH=/home/user_home/ylf/someGit/rbam-1.20/htslib-1.20-install
RABBITBAM_INSTALL_PATH=/home/user_home/ylf/RabbitBAM
LIBDEFLATE_INSTALL_PATH=/home/user_home/ylf/someGit/rbam-1.20/libdeflate-1.20-install

# Compiler and flags
CXX=g++
CXXFLAGS=-O3 -g -std=c++11

# Include directories
INCLUDES = -I$(HTSLIB_INSTALL_PATH)/include \
           -I$(RABBITBAM_INSTALL_PATH) \
           -I$(RABBITBAM_INSTALL_PATH)/htslib \
           -I$(LIBDEFLATE_INSTALL_PATH)/include

# Library directories
LIBS = -L$(HTSLIB_INSTALL_PATH)/lib \
       -L$(RABBITBAM_INSTALL_PATH) \
       -L$(LIBDEFLATE_INSTALL_PATH)/lib \
       -L$(LIBDEFLATE_INSTALL_PATH)/lib64

# Libraries to link
LDFLAGS = $(LIBS) -lhts -lz -lpthread -lrabbitbamtools -lrabbitbamread -lrabbitbamwrite -lstdc++

# Final flags
CXXFLAGS += $(INCLUDES)

# Example target
all:bam_qc 

sort: bam_qc.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o bam_qc

