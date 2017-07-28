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
int main(int argc, char **argv) {
    try {
        CmdLineParser cml(argc,argv);
        if (argc < 2 || cml["-h"]) {
            cerr << "Usage: X:Y [-d dictionary_name (ARUCO_MIP_36h12 default)] [-s <pixSize>] [-t <Type>(0: panel,1: chessboard )] [-r rand_seed] [-n numOfMaps] [-b border (0: no, 1: yes)] [-bc borderCoefficient (fraction of marker size)] [-i interMarkerDistance(0,1)]" << endl;
            cerr<<"\tDictionaries: "; for(auto dict:aruco::Dictionary::getDicTypes())    cerr<<dict<<" ";cerr<<endl;
            return -1;
        }
        int XSize, YSize;
        if (sscanf(argv[1], "%d:%d", &XSize, &YSize) != 2) {
            cerr << "Incorrect X:Y specification" << endl;
            return -1;
        }

        auto Dict=aruco::Dictionary::loadPredefined(cml("-d","ARUCO_MIP_36h12"));
        float pixSize = stoi(cml("-s","250"));
        int typeMarkerMap = stoi(cml("-t","1"));
        int rand_seed = stoi(cml("-r","0"));
        int numOfMaps = stoi(cml("-n","3"));
        int border = stoi(cml("-b","1"));
        float borderCoefficient = stof(cml("-bc","1"));
        float interMarkerDistance = stof(cml("-i","0.2"));
        //if border is wanted, add extra markers to become border
        if (border){
            XSize = XSize + 2;
            YSize = YSize + 2;
        }

        if ((interMarkerDistance > 1.f) || (interMarkerDistance < 0.f)) {
            cerr << "Incorrect interMarkerDistance '" << interMarkerDistance << "' -- needs to be [0,1]" << endl;
            return -1;
        }

        //get the list of all ids
        std::vector<int> allIds;
        for(auto d:Dict.getMapCode()) allIds.push_back(d.second);
        //random shuffle
        srand(rand_seed);
        std::random_shuffle(allIds.begin(),allIds.end());

        //create a list to store
        std::vector<int> ids;
        int idsPerMap = XSize*YSize;
        char imgName [50];
        char configName [50];

        for(int i=1; i<=numOfMaps; i++){
            ids.clear();
            for(int j=0; j<idsPerMap; j++){
                ids.push_back(allIds.back());
                allIds.pop_back();
            }
            aruco::MarkerMap map=Dict.createMarkerMap(Size(XSize, YSize), pixSize, pixSize * interMarkerDistance,ids,typeMarkerMap==1);
            //create a printable image to save
            cv::Mat MarkerMapImage=map.getImage();
            //if border input is true, add a border to the image
            if (border) {
                float bSize = pixSize * borderCoefficient - (pixSize * .1);
                cv::Mat cropped = MarkerMapImage(Rect((pixSize * .9), (pixSize * .9), float(XSize * pixSize) - (pixSize * 1.8),float(YSize * pixSize) - (pixSize * 1.8)));
                cropped.copyTo(MarkerMapImage);
                cv::copyMakeBorder(MarkerMapImage, MarkerMapImage, bSize, bSize, bSize, bSize, BORDER_REPLICATE);
            }
            //save
            sprintf(configName, "config%d.yml", i);
            sprintf(imgName, "markerMap%d.png", i);
            map.saveToFile(configName);
            imwrite(imgName, MarkerMapImage);
        }

    } catch (std::exception &ex) {
        cout << ex.what() << endl;
    }
}
