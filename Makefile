CXX = g++
ARUCO_DIR = src/aruco-2.0.19

# check if on unix ("g++ ...") or on Max ("Apple...")
CXXVERSION := $(shell g++ --version | head -c 3)

CPPFLAGS = -O2 -W -Wall -std=c++11 -I$(ARUCO_DIR)/src -Isrc/
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -laruco -L$(ARUCO_DIR)/build/src

ifeq "$(CXXVERSION)" "g++"
  LDLIBS += -fopenmp
endif

SRC = src/calibration.cpp src/calibrateWithSettings.cpp
BIN = build/calibrateWithSettings utils/createUniqueMarkermaps

all: build/calibrateWithSettings utils/createUniqueMarkermaps

build:
	mkdir -p build

build/calibrateWithSettings: src/calibration.cpp src/calibrateWithSettings.cpp src/calibration.h build
	$(CXX) $(CPPFLAGS) -o $@ src/calibration.cpp src/calibrateWithSettings.cpp $(LDLIBS)

utils/createUniqueMarkermaps: utils/createUniqueMarkermaps.cpp
	$(CXX) $(CPPFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BIN)
