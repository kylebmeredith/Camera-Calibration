#include "markerlabeler.h"
#ifdef USE_SVM_LABELER
#include "markerlabelers/svmmarkers.h"
#endif
#include "markerlabelers/dictionary_based.h"
namespace aruco{
cv::Ptr<MarkerLabeler> MarkerLabeler::create(Dictionary::DICT_TYPES dict_type,float error_correction_rate)throw (cv::Exception)
{
    Dictionary dict=Dictionary::loadPredefined(dict_type);
    DictionaryBased *db=new DictionaryBased();
    db->setParams(dict,error_correction_rate);
    return db;

}



cv::Ptr<MarkerLabeler> MarkerLabeler::create(std::string detector,std::string params)throw (cv::Exception){
(void)params;
    if (detector=="SVM"){
#ifdef USE_SVM_LABELER
        SVMMarkers *svm=new SVMMarkers;
        if (!svm->load( params)) throw cv::Exception( -1,"Could not open svm file :"+params,"Detector::create"," ",-1 );
        //*SVMmodel,dictsize, -1, 1, true);
        return svm;
#else
        throw cv::Exception( -1,"SVM labeler not compiled","Detector::create"," ",-1 );
#endif
    }
    else{
        Dictionary dict=Dictionary::load( detector);
        //try with one from file
        DictionaryBased *db=new DictionaryBased();
        db->setParams(dict,std::stof(params));
        return db;
    }

    throw cv::Exception( -1,"No valid labeler indicated:"+detector,"Detector::create"," ",-1 );

}


}
