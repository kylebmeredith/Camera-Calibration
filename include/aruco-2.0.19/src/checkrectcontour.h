//Fast method to check if a contour is a rectangle or not
/*****************************
    Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

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
#ifndef _Aruco_CheckRectContour_H
#define _Aruco_CheckRectContour_H
#include "exports.h"
#include <opencv2/core/core.hpp>
#include <string>

namespace aruco {
/**\brief Checks if a contour is a rectangle
 */
class ARUCO_EXPORTS CheckRectContour {
public:
    //returns the approximation or empty if not a convex rectangle
    static std::vector<cv::Point> getConvexRect(std::vector<cv::Point> &points,float thres=2);
private:
    static void  calculate_emax_between_points(const std::vector<cv::Point> &contorno, unsigned int inicial, unsigned int final,
                                               float &errorMaximo, unsigned int &posicion);

    static  bool  check_rect_contour(const std::vector<cv::Point> & contorno, const float &umbral,
                                     std::vector<cv::Point> &aproximacion);
};
}
#endif
