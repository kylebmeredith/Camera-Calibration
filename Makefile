CXX = g++
ARUCO_DIR = src/aruco-2.0.19

CPPFLAGS = -O2 -W -g -Wall -std=c++11 -I$(ARUCO_DIR)/src -Isrc/
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -laruco -L$(ARUCO_DIR)/build/src

SRC = src/calibration.cpp src/calibrateWithSettings.cpp
OBJ = build/calibration.o build/calibrateWithSettings.o
BIN = build/calibrateWithSettings
# LIB = build/calibrationLib.a

all: build/calibrateWithSettings

build/calibration.o: src/calibration.cpp src/calibration.h
	$(CXX) $(CPPFLAGS) -c -o $@ $<

build/calibrateWithSettings.o: src/calibrateWithSettings.cpp src/calibration.h
	$(CXX) $(CPPFLAGS) -c src/calibrateWithSettings.cpp -o build/calibrateWithSettings.o

build/calibrateWithSettings: build/calibration.o build/calibrateWithSettings.o
	$(CXX) $(LDLIBS) build/calibration.o build/calibrateWithSettings.o -o build/calibrateWithSettings

clean:
	rm -f $(OBJ) $(BIN)

# calibrationLib.a: build/calibration.o
# 	ar -r build/calibrationLib.a build/calibration.o
