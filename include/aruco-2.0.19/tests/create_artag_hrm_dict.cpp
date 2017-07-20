#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <vector>
#include <bitset>

#define MSize 6
#define NBITS 36

using namespace std;
int main(int argc,char **argv){

    if (argc!=2){cerr<<"Usage in.c    "<<endl;return -1;}
    std::ifstream file(argv[1]);
    std::vector<uint64_t> tags;
    std::vector< bitset<NBITS> > bits;

    while(!file.eof()){
        string line;
        std::getline(file,line);
        bool start=0;
        string number;
        for(int i=0;i<line.size();i++)
            if (line[i]=='=') start=1;
            else{
                if( start==1 && line[i]!=';' && line[i]!='U' && line[i]!='L')
                    number.push_back(line[i]);
            }

        if(number.size()!=0){
            stringstream sstr;sstr<<number;
            uint64_t unumber;
            sstr>>std::hex>>unumber;
            tags.push_back(unumber);
            bits.push_back(bitset<NBITS>(unumber));
        }
    };
    //now,create the codes
    cout<<"vector<uint64_t> codes={";
     for(int i=0;i<bits.size();i++){
        uint64_t code=bits[i].to_ullong();
        cout<<"0x"<<std::hex<<code<<"UL";
        if (i<bits.size()-1)cout<<",";

    }
    cout<<"};";


}
