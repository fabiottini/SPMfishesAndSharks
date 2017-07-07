CC = g++
ICC = /opt/intel/composer_xe_2015.0.090/bin/intel64/icc -mmic
FLAGS = -fopenmp -std=c++0x
ICC_FLAGS =  -parallel -O3 -ip -m64 -fPIC -static -Wall
CC_FLAGS = -g
TARGET = fishsharks

all: $(TARGET)

fishsharks: fishsharks.cpp matrix.cpp fishsharks.h matrix.h
	$(CC) fishsharks.cpp matrix.cpp $(CC_FLAGS) $(FLAGS) -o fishsharks

fishsharks-mic: fishsharks.cpp matrix.cpp fishsharks.h matrix.h
	$(ICC)  fishsharks.cpp matrix.cpp $(FLAGS) $(ICC_FLAGS) -o fishsharks_mic  
 
	@echo " "
	@echo ***COPY TO MIC0
	scp fishsharks_mic mic0:

fishsharks-test: fishsharks_test.sh
	@echo ***COPY TO MIC0
	scp fishsharks_test.sh mic0:

clean:
	rm -f $(TARGET)
