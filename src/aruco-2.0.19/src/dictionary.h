#ifndef ARUCO_DICTIONARY_
#define ARUCO_DICTIONARY_
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <stdint.h>
#include "exports.h"
#include <opencv2/core/core.hpp>
namespace aruco{
class MarkerMap;
/**Represents a set of valid marker ids with a maximum size of 8x8 = 64 bits.
 * In our approach, markers are seen as a pair code-id. The code is the internal binary code printed on the marker. Maximum size is 8x8 bits.
 * The id is a smaller number you can use to identify it. You will use only the id
 *
 * See enum DICT_TYPES for the set of dicitionaries availables
 */

class ARUCO_EXPORTS Dictionary
{
public:

    //loads from a set of predefined ones
    enum DICT_TYPES {ARUCO,//original aruco dictionary. By default
                     ARUCO_MIP_25h7,
                     ARUCO_MIP_16h3,
                     ARUCO_MIP_36h12,//*** recommended
                     ARTAG,//
                     ARTOOLKITPLUS,
                     ARTOOLKITPLUSBCH,//
                     TAG16h5,TAG25h7,TAG25h9,TAG36h11,TAG36h10, //april tags
                     CHILITAGS,//chili tags dictionary . NOT RECOMMENDED. It has distance 0. Markers 806 and 682 should not be used!!!
                     CUSTOM//for used defined dictionaries  (using loadFromfile).
                     };
    //indicates if a code is in the dictionary
    bool is(uint64_t code)const{return _code_id.find(code)!=_code_id.end();}

    DICT_TYPES getType()const{return _type;}

    //reutnr the numerber of ids
    uint64_t size()const{return _code_id.size();}
    //returns the total number of bits of the binary code
    uint32_t nbits()const{return _nbits;}
    //returns the dictionary distance
    uint32_t tau()const{return _tau;}
    //returns the name
    std::string getName()const{return _name;}
    //return the set of ids
    const std::map<uint64_t,uint16_t> & getMapCode()const{return _code_id;}

    //returns the id of a given code.
    int operator[](uint64_t code) { return _code_id[code];  }


    //returns the image of the marker indicated by its id. It the id is not, returns empty matrix
    //@param id of the marker image to return
    //@param bit_size of the image will be  AxA, A=(nbits()+2)*bit_size
    //@param enclosed_corners if true, extra rectagles are added touching the marker corners. it can be used to allow subpixel refinement
    cv::Mat getMarkerImage_id(int id,int bit_size,bool addWaterMark=true,bool enclosed_corners=false);

    //used for boards
    MarkerMap  createMarkerMap( cv::Size gridSize,int MarkerSize,int MarkerDistance, const std::vector<int> &Ids,bool chess_board=false) throw (cv::Exception);


    static Dictionary loadPredefined(DICT_TYPES type) throw(cv::Exception);
    static Dictionary loadPredefined(std::string type) throw(cv::Exception);

    /** loads a dictionary defined in a file
    * Please note that the parsing is very basic and you must be very strict.

    * Here is an example of a 3x3 dictionary of 3 markers
    * 010    111    000
    * 001    101    001
    * 001    010    100
    *
    *
    * File:  myown.dict
    *-------------------------------------------
    * name MYOWN
    * nbits  9
    * 010001001
    * 111101010
    * 000001100
    */
    static Dictionary loadFromFile(std::string path) throw(cv::Exception);

    /**Loads a dictioanary using the string passed. If it is a string of the predefined dictionaries, then returns it.
     * Otherwise, tries to load from a file
      */
    static Dictionary load(std::string info)throw(cv::Exception);



//    //io functions
//    void saveToFile(std::string file)throw(cv::Exception);
//    void readFromFile(std::string file)throw(cv::Exception);
//    void saveToStream(std::ostream & str)throw(cv::Exception);
//    void readFromStream(std::istream &str)throw(cv::Exception);



    //returns the dictionary distance
    static uint64_t computeDictionaryDistance(const Dictionary &d);

    //given a string,returns the type
    static DICT_TYPES getTypeFromString(std::string str)  throw(cv::Exception);
    static std::string getTypeString(DICT_TYPES t)   throw(cv::Exception);
    static bool isPredefinedDictinaryString(std::string str)  throw(cv::Exception);
    static std::vector<std::string> getDicTypes() ;
private:

    void insert(uint64_t code,int id){_code_id.insert(std::make_pair(code,id));}
    static void fromVector(const std::vector<uint64_t> &codes,std::map<uint64_t,uint16_t>  &code_id_map);

    std::map<uint64_t,uint16_t> _code_id;//marker have and code (internal binary code), which correspond to an id.

    uint32_t _nbits;//total number of bits . So, there are sqrt(nbits) in each axis
    uint32_t _tau;//minimum distance between elements


    DICT_TYPES _type;
    std::string _name;
};
}

#endif

