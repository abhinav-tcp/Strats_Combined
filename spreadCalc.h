#ifndef __SPREADCALC_H__
#define __SPREADCALC_H__
#include "strategy.h"
#include<string>
bool isFutSymbol(std::string symbol);
class Strategy;
class SpreadCalc
{
    public:
        SpreadCalc(Strategy *);
        double getSpread(std::string symbol_);

    private:
	   Strategy *_strat;
   double getMultiplierForSymbol(std::string symbol);
   double getCurrentSpread(std::string symbol);

};
#endif
