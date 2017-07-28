#include <iostream>
#include "checkrectcontour.h"
using namespace std;
namespace aruco{

struct Line
{
public:

    Line(const cv::Point p1,const cv::Point p2){
        _a=(p1.y - p2.y);
        _b=(p2.x - p1.x);
        _c=(p1.x * (-(p1.y - p2.y)) + p1.y * (-(p2.x - p1.x)));
        inv_den=1./ pow( _a * _a + _b * _b, 0.5);
    }
    //Calcula la distancia de un punto a una recta
    float dist(const cv::Point p)const{    return fabs( _a * p.x + _b * p.y + _c) *inv_den;}
    int sign(cv::Point p){
        float valor = _a * p.x + _b * p.y + _c;
        if (valor > 0.0) return 1;
        else if (valor < 0.0) return -1;
        return 0;
    }

private:
    float _a,_b,_c,inv_den;
};

//Calcula el Emax en el segmento entre inicial y final
void CheckRectContour::calculate_emax_between_points(const vector<cv::Point> &contorno, unsigned int inicial, unsigned int final,
                                float &errorMaximo, unsigned int &posicion)
{
    assert (inicial != final);
    assert(  (inicial <  contorno.size()));
    assert((final >  0) && (final <  contorno.size()));

    errorMaximo = -1;
    posicion = inicial;
    float error = 0.0;
    unsigned int begin  = (inicial + 1) %  contorno.size();

    Line recta(contorno[inicial] , contorno[final] );
    for (unsigned int i = begin; i != final;)
    {
        error = recta.dist(contorno[i]);
        if (error > errorMaximo)
        {
            errorMaximo = error;
            posicion = i;
        }
        i = (i + 1);//do notuse % so many times, it is slow
        if(i>=contorno.size()) i=0;//if exceed the limits, go back to 0
    }
}


bool CheckRectContour::check_rect_contour(const vector<cv::Point> & contorno, const float &umbral,
                                   vector<cv::Point> &aproximacion)
{
    //primero seleccionamos dos puntos aleatorios que van a ser el inicial y el central
    unsigned int inicial=0, central= contorno.size() / 2;

    //Ahora se calculan los puntos más distantes a la recta que une esos puntos
    //uno en el recorrido inicial, central y otro en el recorrido central inicial

    //se supone que si es un cuadrilátero, estos dos puntos más distantes son vértices opuestos del cuadrilátero

    float eMax1, eMax3;
    unsigned int posicionMax1, posicionMax3;
  //  cout <<"CALCULO P1 \n";
    calculate_emax_between_points(contorno, inicial, central, eMax1, posicionMax1);
  //  cout << "posicion max 1 = " << posicionMax1 << endl;
    //cout <<"CALCULO P3 \n";
    calculate_emax_between_points(contorno, central, contorno.size() - 1, eMax3, posicionMax3);
    //cout << "posicion max 3 = " << posicionMax3 << endl;


    //Ahora comprobamos si es un triángulo. Para ello se comprueba el emax entre los vértices recién obtenidos
    float error1, error2;
    unsigned int pos1, pos2;

//    cout <<"CALCULO error entre P1 y P3 \n";
    calculate_emax_between_points(contorno, posicionMax1, posicionMax3, error1, pos1);
  //  cout <<"CALCULO error entre P3 y P1 \n";
    calculate_emax_between_points(contorno, posicionMax3, posicionMax1, error2, pos2);

    if (error1 <= umbral || error2 <= umbral)
    {
    //    cout << "ES UN TRIÁNGULO \n";
        return false;
    }
    else //Calculamos los dos vértices que faltan
    {
        float eMax2, eMax4;
        unsigned int posicionMax2, posicionMax4;
        calculate_emax_between_points(contorno, posicionMax1, posicionMax3, eMax2, posicionMax2);
        calculate_emax_between_points(contorno, posicionMax3, posicionMax1, eMax4, posicionMax4);
        //Ahora comprobamos el error máximo del cuadrilátero, que será la aproxmación poligonal
        float e1, e2, e3, e4; //Errores maximos
        unsigned int p1, p2, p3, p4; //Puntos de error maximo en cada lado

        calculate_emax_between_points(contorno, posicionMax1, posicionMax2, e1, p1);

        if (e1 > umbral) //no es cuadrilatero
            return false;
        else
        {
            calculate_emax_between_points(contorno, posicionMax2, posicionMax3, e2, p2);
            if (e2 > umbral) //no es cuadrilatero
                return false;
            else
            {
                calculate_emax_between_points(contorno, posicionMax3, posicionMax4, e3, p3);
                if (e3 > umbral) //no es cuadrilatero
                    return false;
                else
                {
                    calculate_emax_between_points(contorno, posicionMax4, posicionMax1, e4, p4);
                    if (e3 > umbral) //no es cuadrilatero
                        return false;
                    else //Es un cuadrilatero
                    {

                        //now, check if the signs of opposite points are different so that is convex
                        Line l1(contorno[posicionMax1],contorno[posicionMax3]);
                        if ( l1.sign(contorno[posicionMax2])==l1.sign(contorno[posicionMax4]))return false;
                        Line l2(contorno[posicionMax2],contorno[posicionMax4]);
                        if ( l2.sign(contorno[posicionMax1])==l2.sign(contorno[posicionMax3]))return false;

                        //Guardamos los vertices del cuadrilátero en la aproximacion
                        aproximacion.push_back(contorno[posicionMax1]);
                        aproximacion.push_back(contorno[posicionMax2]);
                        aproximacion.push_back(contorno[posicionMax3]);
                        aproximacion.push_back(contorno[posicionMax4]);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


vector<cv::Point> CheckRectContour::getConvexRect(vector<cv::Point> &points,float thres){

    std::vector<cv::Point> pres;

    if (check_rect_contour(points,thres,pres) )return pres;
    else return {};

}

}
