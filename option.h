#ifndef __OPTION_H_
#define __OPTION_H_
#include <bits/stdc++.h>

class Option {
	private:
	std::string symbol;
	double optionPrice;
	double underlyingPrice;
	double strikePrice;
	double delta;
	double vega;
	double gamma;
	double theta;
	int lotSize;
	double impliedVol;
	bool isCE; // false means PE
	int orderId;
	int orderStatus;
	std::string expiryDate;
	public:
	Option(std::string sym, double price, double strike, int lot){} 
}

#endif
