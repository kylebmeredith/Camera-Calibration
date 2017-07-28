
#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <chrono>
using namespace std;

void saveContour(const vector<cv::Point> &contour,ostream &file)
{
    for(const auto &p:contour)
        file<<p.x<<" "<<p.y<<" ";
    file<<endl;
}
void  drawApproxCurve(cv::Mat &in,const vector< cv::Point > &contour, cv::Scalar color) {
    for (unsigned int i = 0; i < contour.size(); i++) {
        cv::line(in, contour[i], contour[(i + 1) % contour.size()], color);
    }
}
int main(int argc,char **argv){

    try{
        if (argc<3){cerr<<"Usage: inimage outfile.txt [param1 wsize:7 default] [param2 thres:7 defautl]"<<endl;return -1;}

        cv::Mat image=cv::imread(argv[1]);
        cv::Mat image_grey;
        cv::cvtColor(image,image_grey,CV_BGR2GRAY);
         if (image.empty())throw std::runtime_error("Image not loaded");
        int param1=7,param2=7;
        if(argc>=4)param1=stoi(argv[3]);
        if(argc>=5)param2=stoi(argv[4]);

        cv::Mat out;
        cv::adaptiveThreshold(image_grey, out, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, param1, param2);
        cv::imshow("thres",out);
        std::vector< cv::Vec4i > hierarchy;
        std::vector< std::vector< cv::Point > > contours;

        float minSize=100;

        cv::findContours(out, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        cout<<"contours.size()=="<<contours.size()<<endl;
        cv::drawContours( image, contours, -1, cv::Scalar(0,0,255), 1   );
        //save all contours
        ofstream out_file(argv[2]);
        for (const auto &c:  contours )  saveContour(c,out_file);//save one line per contour

        //NOW, do polygon approx and consider time employed
        std::vector<std::vector<cv::Point> >approxCurve(contours.size());
        auto t1= std::chrono::high_resolution_clock::now();
        /// for each contour, analyze if it is a paralelepiped likely to be the marker
        for (unsigned int i = 0; i < contours.size(); i++)
                 approxPolyDP(contours[i], approxCurve[i], double(contours[i].size()) * 0.05, true);
        auto t2= std::chrono::high_resolution_clock::now();
        cout<<"total time="<<std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count()<<endl;

        //draw squares
        for (const auto &ac:  approxCurve )
            if (ac.size()==4)
                drawApproxCurve(image,ac,cv::Scalar(255,0,0));


        cv::imshow("image",image);

         while( cv::waitKey(0)!=27) ;//wait for esc

    }catch(std::exception &ex){
        cout<<ex.what()<<endl;
    }
}
