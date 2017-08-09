/*****************************
Copyright 2016 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/

//Creation of MarkerMap to be printed in a piece of paper.
//Modified by Kyle Meredith

#include <string>
#include "markermap.h"
#include <cstdio>
#include "dictionary.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
class CmdLineParser{int argc; char **argv; public: CmdLineParser(int _argc,char **_argv):argc(_argc),argv(_argv){}  bool operator[] ( string param ) {int idx=-1;  for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i;    return ( idx!=-1 ) ;    } string operator()(string param,string defvalue="-1"){int idx=-1;    for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i; if ( idx==-1 ) return defvalue;   else  return ( argv[  idx+1] ); }};

using namespace std;
using namespace cv;
using namespace aruco;

int main(int argc, char **argv) {
    try {
        CmdLineParser cml(argc,argv);
        if (argc < 2 || cml["-h"]) {
            cerr << "Usage: X:Y (# of markers per row:column)\n"
            "   [-d <dict>]          #dictionary name (ARUCO_MIP_36h12 default)\n"
            "   [-n <numOfMaps>]    #number of unique marker maps to create (1 default)\n"
            "   [-s <pixelSize>]     #size of each marker in pixels (250 default)\n"
            "   [-t <patternType>]   #0: panel, 1: chessboard (1 default)\n"
            "   [-r <randSeed>]    #seed to randomize markers\n"
            "   [-b <border>         #1: save maps with border, 0: no border (1 default)\n"
            "   [-bf <borderFactor>]  #Width of border by factor of marker size (2: half of border, 2 default)\n"
            "   [-i <interMarkerDist>]  #Distance between markers (range=[0,1], 0.2 default)\n" << endl;

            cerr<<"\tDictionaries: "; for(auto dict:Dictionary::getDicTypes())  cerr<<dict<<" ";cerr<<endl;
            return -1;
        }

        float XSize, YSize;
        if (sscanf(argv[1], "%f:%f", &XSize, &YSize) != 2) {
            cerr << "Incorrect X:Y specification " << argv[1] << endl;
            return -1;
        }

        auto Dict=Dictionary::loadPredefined(cml("-d","ARUCO_MIP_36h12"));
        int numOfMaps = stoi(cml("-n","1"));
        float pixelSize = stoi(cml("-s","250"));
        int patternType = stoi(cml("-t","1"));
        int randSeed = stoi(cml("-r","0"));
        int border = stoi(cml("-b","1"));
        float borderFactor = stof(cml("-bf","2"));
        float interMarkerDist = stof(cml("-i","0.2"));
        //If border is true, add extra markers to become border
        if (border){
            XSize = XSize + 2;
            YSize = YSize + 2;
        }

        if ((interMarkerDist > 1.f) || (interMarkerDist < 0.f)) {
            cerr << "Incorrect interMarkerDist '" << interMarkerDist << "' -- needs to be [0,1]" << endl;
            return -1;
        }

        //Declare variables for pattern config file
        vector<string> configList;  // ArUco marker maps
        vector<string> planeList;   // Corresponding 3D planes for each marker map
        // The x y transformations to make the origin the bottom left corner
        int xOffset;
        int yOffset;
        int denominator;  // The denominator required to make all point values integers

        //Get the list of all dictionary ids
        vector<int> allIds;
        for(auto d:Dict.getMapCode()) allIds.push_back(d.second);
        //Random shuffle
        srand(randSeed);
        random_shuffle(allIds.begin(),allIds.end());

        //Create a list to store pattern ids
        vector<int> ids;
        int idsPerMap = XSize*YSize;

        char imgName [50];
        char configName [50];
        for(int i=1; i<=numOfMaps; i++){
            ids.clear();
            for(int j=0; j<idsPerMap; j++){
                ids.push_back(allIds.back());
                allIds.pop_back();
            }
            MarkerMap map = Dict.createMarkerMap(Size(XSize, YSize), pixelSize, pixelSize * interMarkerDist,ids,patternType==1);

            //create a printable image to save
            Mat MarkerMapImage=map.getImage();

            //if border input is true, add a border to the image
            if (border) {
                float bSize = (pixelSize / borderFactor) - (pixelSize * .1);
                // Crop so that just the white/black of the edge markers are showing, then extend this out
                Mat cropped = MarkerMapImage(Rect((pixelSize * .9), (pixelSize * .9), float(XSize * pixelSize) - (pixelSize * 1.8),float(YSize * pixelSize) - (pixelSize * 1.8)));
                cropped.copyTo(MarkerMapImage);
                copyMakeBorder(MarkerMapImage, MarkerMapImage, bSize, bSize, bSize, bSize, BORDER_REPLICATE);
            }

            // Save files
            sprintf(configName, "config%d.yml", i);
            sprintf(imgName, "markerMap%d.png", i);
            map.saveToFile(configName);
            imwrite(imgName, MarkerMapImage);

            // Add info to arucoPattern struct
            configList.push_back(configName);

            switch (i) {
                case 1:
                    planeList.push_back("XY");
                    break;
                case 2:
                    planeList.push_back("YZ");
                    break;
                case 3:
                    planeList.push_back("XZ");
                    break;
                default:
                    planeList.push_back("XY");
                    break;
            }
        }

        // Calculate parameters for ArUco config, which will be used in calculation of 3D points
        denominator = pixelSize / borderFactor;
        cout << (int)(((XSize-2)/2 + (1 / borderFactor)));
        if (border) {
            xOffset = (int)(((XSize-2)/2 + (1 / borderFactor)) * pixelSize);
            yOffset = (int)(((YSize-2)/2 + (1 / borderFactor)) * pixelSize);
        }
        else {
            xOffset = (int)(XSize/2 * pixelSize);
            yOffset = (int)(YSize/2 * pixelSize);
        }

        // Save configuration to file
        FileStorage fs( "arucoConfig.yml", FileStorage::WRITE );

        fs << "MarkerMap_Configs" << configList;
        fs << "Planes" << planeList;
        fs << "xOffset" << xOffset;
        fs << "yOffset" << xOffset;
        fs << "Denominator" << denominator;

    } catch (exception &ex) {
        cout << ex.what() << endl;
    }
}
