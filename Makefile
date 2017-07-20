CXX = g++
ARUCO_DIR = include/aruco-2.0.19

CPPFLAGS = -O2 -W -g -Wall -std=c++11 -I$(ARUCO_DIR)/src
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -laruco -L$(ARUCO_DIR)/build/src

OBJ = calibration.o

# main: main.o
# 	$(CXX) $(CPPFLAGS) -o main main.o

# main.o: main.cpp calibration.h

calibrationLib.a: calibration.o
	ar -r calibrationLib.a calibration.o

calibration.o: calibration.cpp calibration.h

all: calibration.a main

clean:
	rm -f $(all) *.o core*

# SRC = calibration.cpp
#
# OBJ = calibration.o
#
# main: calibration.o
#
# calibrationLib.a: $(OBJS)
#
# BIN = main
#
#
#
# $(OBJS): $(SRCS)
#
# $(LIB): $(OBJ)
# 	ar -r $(LIB) $(OBJ)
#
# $(BIN): main.cpp
#
#
