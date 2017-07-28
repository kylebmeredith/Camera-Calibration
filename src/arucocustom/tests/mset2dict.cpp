#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <vector>
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

int main(int argc,char **argv){


    if (argc!=2){cerr<<"Usage in   "<<endl;return -1;}
    aruco::hrm::Dictionary dic;
    dic.fromFile(argv[1]);
    //cout<<"md="<<dic.minimunDistance()<<endl;cin.ignore();
    aruco::Dictionary dict2;
    //insert the elements
    cout<<"\n\nvector<uint64_t> codes={";
    int idx=0;
    for(int i=0;i<dic.size();i++){
        uint64_t code=fromString(dic[i].toString()).to_ullong();
//        cout<<"std::pair<uint64_t,int>("<<code<<","<<i<<")";
        cout<<"0x"<<std::hex<<code<<"UL";
        if (i<dic.size()-1)cout<<",";
    }
    cout<<"};\n\n";

}
