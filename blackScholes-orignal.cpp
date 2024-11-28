#ifndef bs_cpp
#define bs_cpp

#include <iostream>
#include <math.h>
#include <cmath>
//#include <conio.h>
#ifndef PI
#define PI 3.141592653589793238462643
#endif
using namespace std;

class BlackScholes{
    double strikePrice;
    double stockPrice;
    double rate;
    double timeToExpiry;
    double dividends;
    double volatility;
    double timeSqrt;
    double d1;
    double d2;
    double callP;
    double putP;

public:
//    BlackScholes(){
//        //Initialize option variables S,X,r,d,vol,t
//        strikePrice = 5000;
//        stockPrice = 5050;
//        rate = 0;
//        timeToExpiry = (double) 10/365; //in number of years
//        dividends = 0;
//        volatility = 0.3;
//        timeSqrt = sqrt(timeToExpiry);
//        d1 = (log(stockPrice/strikePrice)+rate*timeToExpiry)/(volatility*timeSqrt)+0.5*volatility*timeSqrt;
//        d2 = d1 - (volatility*timeSqrt);
//    };
    //Constructor with parameters
        BlackScholes(double X, double S, double r, double div, double vol, double t){
        //Initialize option variables S,X,r,d,vol,t
        strikePrice = X;
        stockPrice = S;
        rate = r;
   //     rate = 0.08; //should be equal to mibor if underlying is index value, but 0 if underlying is index future
        timeToExpiry = t;
        dividends = div;
        volatility = vol;
    //    cout<<"vol = "<<vol<<endl;
        timeSqrt = sqrt(timeToExpiry);

        d1 = getd1();
        d2 = getd2();
    };

    double getd1(){

        double t = (log(stockPrice/strikePrice)+rate*timeToExpiry)/(volatility*timeSqrt)+0.5*volatility*timeSqrt;
    //    cout<<"t1 ="<<t<<endl;
   //     getch();
        return t;
    }
    double getd2(){
        double t = d1 - (volatility*timeSqrt);
    //    cout<<"t2 ="<<t<<endl;
   //     getch();
        return t;
    }

    void setVolatility(double impVol){
        volatility = impVol;

    }

    double callPrice(){

        d1 = getd1();

        d2 = getd2();
        callP = 0.0001;
        callP = stockPrice * N(d1) - strikePrice*exp(-rate*timeToExpiry) * N(d2);
     //   c = N(0.5);
     //   c = stockPrice * 1 - strikePrice*exp(-rate*timeToExpiry) * 1;
    //    cout<<callP<<"callPrice"<<endl;
        return callP;
    }

    double putPrice(){

        d1 = getd1();
        d2 = getd2();
        putP = strikePrice*exp(-rate*timeToExpiry) * N(-d2) - stockPrice * N(-d1);
        return putP;
    };

    double getDelta(char t){
        double delta;
        if (t=='c'){
            delta = N(d1);
        }
        else{
            delta = (-1)*(1-N(d1));
        }
        return delta;
    };


    void test(){
        cout<<"call p = "<<callPrice()<<endl;
        cout<<"put p = "<<putPrice()<<endl;
        cout<<"delta call = "<<getDelta('c')<<endl;
        cout<<"delta put = "<<getDelta('p')<<endl;
        cout<<"gamma = "<<getGamma()<<endl;
        cout<<"theta = "<<getTheta()<<endl;
        cout<<"vega = "<<getVega()<<endl;
        cout<<"theta = "<<getTheta()<<endl;

    };

    //Cumulative Normal distribution function
    double N(double z){
          if( z == (int) 0){
              double n = 0.5;
              return n;
          }
          const double b1 =  0.319381530;
          const double b2 = -0.356563782;
          const double b3 =  1.781477937;
          const double b4 = -1.821255978;
          const double b5 =  1.330274429;
          const double p  =  0.2316419;
          const double c2  =  0.39894228;

          double a=fabs(z);
          double t=1.0/(1.0+a*p);
          double b=c2*exp((-z)*(z/2.0));
          double n=((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
          n=1-b*n;
          if(z<0){
              n=((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
              n=n*b;

          }
          return n;
    };

    //Standard normal distribution...probability
    double n(double z){
        return (1.0/sqrt(2.0*PI))*exp(-0.5*z*z);
    };


    double getGamma(){
        double gamma;
        gamma = n(d1)/(stockPrice*volatility*timeSqrt);
        return gamma;
    };

    double getTheta(){
        double theta;
        theta = (-1)*(stockPrice*volatility*n(d1))/(2*timeSqrt) - rate*strikePrice*exp(-rate*timeToExpiry)*N(d2);
        return theta;

    };

    double getVega(){
        double vega;
        d1 = (log(stockPrice/strikePrice)+rate*timeToExpiry)/(volatility*timeSqrt)+0.5*volatility*timeSqrt;
        d2 = d1 - (volatility*timeSqrt);
        vega = stockPrice*timeSqrt*n(d1);
        return vega;
    };

    //Easy to implement for blackscholes, not easy for binomial pricing models where derivatives are not known
    double getInitialIVEstimate(double cPrice){
        double uPrice = stockPrice;

        double eeta = strikePrice/stockPrice;
        double sigma_corrado = sqrt(2*PI)*(cPrice - ((uPrice - strikePrice)/2) + sqrt(pow((cPrice - (uPrice - strikePrice)/2),2) - (pow((uPrice - strikePrice),2)/PI)*(1+eeta)/2))/((uPrice + strikePrice)*sqrt(timeToExpiry));

   //     cout<<"corrado sigma = "<<sigma_corrado<<endl;
   //     getch();
        return sigma_corrado;
    }

    double getImpliedVol_NewtonRaphson(double optionPrice, char c ){

        const int MAX_ITERATIONS=100;
        const double ACCURACY=1.0e-8;
    //    cout<<ACCURACY<<endl;
    //    cout<<MAX_ITERATIONS<<endl;
        double optionThPrice;

        double impliedVol = 1.0e-6;
        volatility = impliedVol;

        if (c=='c'){
            optionThPrice = callPrice();
       //     cout<<"option th price"<<optionThPrice<<","<<optionPrice<<endl;
        }
        else{
            optionThPrice = putPrice();
      //      cout<<"option th price put"<<optionThPrice<<","<<optionPrice<<endl;
        }

        if (optionThPrice>optionPrice){
     //       cout<<"Something wrong th, op"<<optionThPrice<<", "<<optionPrice<<endl;
            return 0.0;
        }


    //    impliedVol = max(getInitialIVEstimate(optionPrice),0.1);

        impliedVol = max(((optionPrice/stockPrice)/(0.398*timeSqrt)),0.1);
     //   cout<<impliedVol<<endl;
      //  getch();
     //   cout<<"stock RPice, timesqrt"<<stockPrice<<","<<timeSqrt<<endl;
     //   cout<<"implied vol initial = ,"<<impliedVol<<","<<optionPrice<<endl;
     //   volatility = impliedVol;
        double diff,vega;
        for(int i =0;i<MAX_ITERATIONS;i++){
            if (c=='c'){
                optionThPrice = callPrice();
             //   cout<<optionThPrice<<endl;
            }
            else{
                optionThPrice = putPrice();
             //   cout<<optionThPrice<<"put"<<endl;
            }
            diff = optionPrice - optionThPrice;
      //      cout << "diff = "<<diff<<endl;
            if(fabs(diff)<ACCURACY){
                return impliedVol;
            }
            double td1 = ((log(stockPrice/strikePrice)+rate*timeToExpiry)/(impliedVol*timeSqrt)) +0.5*impliedVol*timeSqrt;

            vega = stockPrice*timeSqrt*n(td1);
      //      cout<<"td1 ="<<td1<<","<<diff<<","<<vega<<endl;
      //      getch();
      //      cout<<"vega = "<<vega<<endl;
          //  getch();
      //      cout<<"Implied Vol ="<<impliedVol<<endl;
            impliedVol = impliedVol + diff/vega;
            volatility = impliedVol;
        //    cout<<"iteration number = "<<i<<endl;
        };
     //   cout<<"diff = "<<diff<<endl;
     //   cout<<"iv , diff , vega, vol "<<impliedVol<<","<<diff<<", "<<vega<<","<<volatility<<endl;
        return getImpliedVolatility_Bisections(optionPrice, c);

    };


    ///////////////////////////////////////////
    double getImpliedVolatility_Bisections(double optionPrice, char c){

        double sigmaLow = 1.0e-6;
        const double ACCURACY = 1.0e-5; // make this smaller for higher accuracy
        const int MAX_ITERATIONS = 100;
        const double HIGH_VALUE = 1e10;
        const double ERROR = -1e40;
        double sigmaHigh = 0.3;
        double optionThPrice;
        volatility = sigmaLow;
         if (c=='c'){
            optionThPrice = callPrice();
       //     cout<<"option th price"<<optionThPrice<<","<<optionPrice<<endl;
        }
        else{
            optionThPrice = putPrice();
      //      cout<<"option th price put"<<optionThPrice<<","<<optionPrice<<endl;
        }

        if(optionThPrice > optionPrice){
            return 0.0;
        }
        while (optionThPrice < optionPrice) {
            sigmaHigh = 2*sigmaHigh;
            volatility = sigmaHigh;
             if (c=='c'){
                optionThPrice = callPrice();
            //     cout<<"option th price"<<optionThPrice<<","<<optionPrice<<endl;
            }
            else{
                optionThPrice = putPrice();
      //      cout<<"option th price put"<<optionThPrice<<","<<optionPrice<<endl;
            }
            if (volatility>HIGH_VALUE) return ERROR; // panic, something wrong.
        };

        for (int i=0;i<MAX_ITERATIONS;i++){
         //   cout<<"bisection iteration no = "<<i<<endl;
            double sigma = (sigmaLow+sigmaHigh)*0.5;
            volatility = sigma;

              if (c=='c'){
                optionThPrice = callPrice();
            //     cout<<"option th price"<<optionThPrice<<","<<optionPrice<<endl;
            }
            else{
                optionThPrice = putPrice();
      //      cout<<"option th price put"<<optionThPrice<<","<<optionPrice<<endl;
            }

            double test =  (optionThPrice-optionPrice);
            if (fabs(test)<ACCURACY) { return volatility; };
            if (test < 0.0) { sigmaLow = sigma; }
            else { sigmaHigh = sigma; }
        };


 //   cout<<"bisecting"<<endl;
   // getch();
  //  return ERROR;
      return 0.0;
    }

    void set_d1_d2()
    {
	    d1=getd1();
	    d2=getd2();
    }

    /////////////////////////////////////////

};

#endif
