#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <vector>
#include <opencv2/highgui.hpp>
#include <bitset>
#include "highlyreliablemarkers.h"
#include "dictionary.h"
#define MSize 6
#define NBITS 36
using namespace std;
bitset<64> fromString(std::string str){
    stringstream ssstr;ssstr<<str;
    bitset<64> bs;
    ssstr>>bs;
    return bs;
}

uint64_t process(string file)throw(std::runtime_error){
    cv::Mat im=cv::imread(file,cv::IMREAD_GRAYSCALE);
    if (im.total()!=100)throw std::runtime_error("required mini-no border");

    if(im.empty())throw std::runtime_error("Invalid image");
    //divide image in 10 parts and compute 8x8 inner code
    bitset<64> value;
    int bit=63;
    for(int y=1;y<9;y++)
        for(int x=1;x<9;x++)
        value[bit--]=im.at<uchar>(y,x)!=0;
    return value.to_ullong();



}


int main(int argc,char **argv){

    if (argc==1){cerr<<"Usage im1 im2 im3 ...  "<<endl;return -1;}

    vector<string> files;
    for(int i=1;i<argc;i++)
        files.push_back(argv[i]);

    auto getNumber=[](const string &s){
        //find point
        int point;
        for(point=s.size()-1;point>0;point--)
            if (s[point]=='.')break;
        int start=point-1;
        while( isdigit(s[start]) && start>=0) start--;
        start++;
        string n;
        for(int i=start;i<point;i++)
            n.push_back(s[i]);
        return stoi(n);
    };


    std::sort(files.begin(),files.end(),[&](const string &s1,const string &s2){return  getNumber(s1)< getNumber(s2);});

  //  for(auto fil:files)cout<<fil<<endl;exit(0);
    vector<uint64_t> numbers;
    for(auto file:files)
        numbers.push_back(process(file));

    //insert the elements
    cout<<"\n\nvector<uint64_t> codes={";
    int idx=0;
    for(int i=0;i<numbers.size();i++){
        cout<<"0x"<<std::hex<<numbers[i]<<"UL";
        if (i<numbers.size()-1)cout<<",";
    }
    cout<<"};\n\n";
}

