CXX = g++
ARUCO_DIR = src/aruco-2.0.19

# check if on Linux ("g++ ...") or on Mac ("Apple...")
CXXVERSION := $(shell g++ --version | head -c 3)

CPPFLAGS = -O2 -W -Wall -std=c++11 -I$(ARUCO_DIR)/src -Isrc/
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -laruco -L$(ARUCO_DIR)/build/src

ifeq "$(CXXVERSION)" "g++"
  LDLIBS += -fopenmp
endif

SRC = src/calibration.cpp src/calibrateWithSettings.cpp
BIN = build/calibrateWithSettings utils/createArucoPatterns

all: build/calibrateWithSettings utils/createArucoPatterns

build:
	mkdir -p build

build/calibrateWithSettings: src/calibration.cpp src/calibrateWithSettings.cpp src/calibration.h build
	$(CXX) $(CPPFLAGS) -o $@ src/calibration.cpp src/calibrateWithSettings.cpp $(LDLIBS)

utils/createArucoPatterns: utils/createArucoPatterns.cpp
	$(CXX) $(CPPFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BIN)
