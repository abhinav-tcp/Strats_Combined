#ifndef __QUADDIST_H__
#define __QUADDIST_H__

#include<map>
#include<vector>
#include "strategy.h"
#include "quadDistribution.h"
#include "cmath"
bool isFutSymbol(std::string symbol);
class Strategy;
class QuadDist
{
	public:
	QuadDist(Strategy* strat);
	Strategy* _strat;
	QuadDistribution *_quadDis;
	void currMinuteBarsFinished(std::string symbol);
	private:
	void buildPosition(std::string symbol,int qty);
	void buildPosition(std::map<std::string ,int > desiredQty);
	double getClose(std::string symbol);
	std::vector<int> getLots(std::string underlyingSymbol);
	void checkSignal(std::string symbol);
	void getCurrentPnl(std::string symbol);
	bool isStratSquaredOff(std::string symbol);

	void squareOffPositions(std::string symbol);
	void readStrategyConfig(std::string strategyConfigFilePath);


	
	//stratSpecific function and variables
	std::string _currTradingCall, _currTradingPut;

	int _strikesGap,_strikesGapMin;

	double _minMaxLotsRatio=0.3;

	double _optionsPremium=10;
	double resistDuration=120;  //duration In which one can resist
	double _strikesAggresiveness=1.0;
	long long _tradeTime,_squareOffTime;

	bool _positionTaken=false;

	
	//hedger Varibales
	bool _addHedger=false;
	double _hedgerPremium=0.2;
	double _hedgerLotsratio=1;
	double _percStrikeChange=0.5;

	void handleHedger(std::string underlying); //this will handle hedger with respect to current lots // will be called each time after  **** can we make this dynamic too ? like hedger profit booking ??




	std::vector<int> getLots();
	bool checkForStratExit(std::string symbol);
	bool buildQuadPosition(std::string symbol);
	//std::vector<std::vector<std::string>> getChangeStrikeOrNot(std::string underlying);   //return vector of string ["aggr/pass","callOrPut option symbol"] if it need to change strike else return empty strings
	std::string getOptionAtPrice(std::string symbol,std::string callOrPut,double optionPremium);
	//std::vector<std::string> getCurrentTradingSymbols();
	void changeStrikes(std::vector<std::vector<std::string>> symbolInfo);
	std::vector<int> maxMinLotsAtmAndExtDist(std::string symbol);
	std::vector<int> getExtremeOptionsStrikes(std::string symbol);
	std::string getSymbol(std::string symbol,int atm,int strike,std::string callOrPut);
	std::vector<std::pair<std::string,int>> getLotsDistribution(std::string symbol);

	void updateCurrTradingOptions(std::string option1,std::string option2);
};


#endif
