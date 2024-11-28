#ifndef __STRATEGY_H__
#define __STRATEGY_H__
#include <bits/stdc++.h>
//#include "/home/infra/Simulator/Backtester/strategyConnector.h"
//#include "/home/infra/Simulator/Backtester/strategyConnector.h"
#include "/home/infra/InternalSimulatorUsers/Backtester/strategyConnector.h"
#include "blackScholes-orignal.cpp"
#include "options-structs.h"
#include "spreadCalc.h"
#include "priceChase.h"
#include "strat_QuadDist.h"
std::string getOption(std::string symbol, int close,int strikeLevel, std::string finalExpiry, std::string callPut,std::string moneyness);
int minutesSinceMidnight(long long int time);
vector<std::string> split(std::string str, char delimiter);
double time_diff(string date,string exp,long long s);
long long int time_diff(long long int time1, long long int time2);
bool isOption(std::string symbol);
bool isFutSymbol(std::string symbol);
bool isSpotSymbol(std::string symbol);

std::string getTrimmedDate(std::string runDate);
long double getPercDiff(long double avgTradePrice, int close);
long double getPercDiff(long long maxPNL, long long currentPNL);

class SpreadCalc;
class PriceChase;
class QuadDist;
class Strategy{
	public:
		Strategy(std::string strategyConfigFilePath);
		void updateData(std::string symbol,uint8_t,long long int time,uint8_t side,long long int price,int quantity,std::string expiry,int strike,int lot,std::string callput,BookLevel booklvl[10], long long int orderId1, long long int orderId2){}
 		void updateData(std::string smbl,int open,int high,int low,int close,float volume,int LTP,int LTQ,int LTT,int ATP,int lot,std::string expiry,int bidPriceArray[5],int bidQuantityArray[5],int     askPriceArray[5],int askQuantityArray[5]/*std::vector<std::pair<int,int>> buyDepth,std::vector<std::pair<int,int>> sellDepth*/,long long int time){}
		void updateData(std::string symbol,std::string date,long long int time,int open,int high, int low, int close, float volume,int OI, int lot, std::string expiry, std::string callput);
		void checkSignal(std::string symbol);
		void checkSL(std::string symbol);
		void onResponse(char , long long int, std::string, char, int,int);
		//void onResponse(char responseType, long long int orderID, std::string symbol, char side, int quantity,int price, int16_t errorCode, char orderType, int openQuantity);
		void onResponse(char responseType, int16_t errorCode, long long int orderID, std::string symbol, char side, int openQuantity,int fillQuantity,int price, char orderType);
		void setStrategyConnector(StrategyConnector *stc);
        	//void send(char side, int qty, int px, string symbol, char orderType,long long orderID, bool matchRemainingInNeXtBarOverRide);
		void send(char side, int qty, int px, string symbol, char orderType,long long oid,bool matchRemainingInNeXtBarOverRide,int fillQuantity=0,int disclosedQuantity=0,int ioc=0);
        void readStrategyConfig(std::string strategyConfigFilePath);
		void updateVolatility(std::string symbol);
        void checkForStrikeChange(std::string call,std::string put);
        std::string getSymbolAtStrike(std::string ,int strike,std::string callorPut);
        std::string getNextStrikeCallOrPutOption(std::string symbol,int strikeGap=100);
        std::string getNextStrikeCallOrPutOption(std::string symbol,int strikeGap=100,bool moveInwards=false);
        vector<std::string> getTradingOptionsAtVol(std::string symbol,double strikesAggresiveness);
        void squareOffPositions(std::string index);

        void printCurrentPosition();
        bool checkSLTP(std::string index);
                long long int curTime;

        std::string _configPath;
        int _strikesGap=100;
	    int _strikesGapMin=100;
        bool checkTrailingSL(std::string index);
        long double _trailingSLThreshold=1000000;
        long double _trailingSLPerc=100000;

        std::set<std::string> finalOptions;
        std::string getSpotSymbol(){return _spotSymbol;}

        int getStrikesGap(){return _strikesGap;}



	//getter functions
	//
	int getTotalLots(){return _totalLots;}


        std::vector<std::string> getStrikeChangeTradingOptions(std::string currTradingCall,std::string currTradingPut);
        int getSkewness(int callLot,int putLot);

        int lastSkew=0;
        std::pair<int,int> lastDistribution;

	std::shared_ptr<SpreadCalc> _spreadCalc;
	std::shared_ptr<PriceChase> _priceChase;
	std::shared_ptr<QuadDist> _quadDist_strat;
        bool isStrikeLevelBreached(std::string symbol,std::string aggOrPass);

	private:
		long long OIDstart;
		StrategyConnector *connector;

    private:
        std::string runDate;

    private:
        long long int _squareOffTime;
        long long int _subscriptionTime;
        long long int _tradeTime;
        int _totalLots=100;
	bool _totalLotsSet=false;
	long long _totalQuantity=-1;
        std::map<std::string, int> _prevPositions;
    private:
        std::string _hedgeType="FUTURES";
        std::string _slType="SUM_DELTA";
        long double _optionsChangeVal = 0.05;
        int _earlySquareOff=10;
	
	int _strikeLevel=1;
    int _maxOTMStrike=5;
        std::string _executionType="HEDGE";
        bool _positionExit=false;
        long double _minSLValue;
        long double _SLValue;
        long double _percStrikeChange=0.1;
        long double _maxSLValue;
        bool checkSL();
        bool canSL();
        double getTotalDelta();
        void hedgeWithFutures(std::string symbol);
        void hedgeWithOptionsRebalance(std::string index);
        void exitOptionPosition(std::string index);
        bool isSL(std::string symbol);
        long double getTotalDelta(std::string symbol);
        long double getIndividualSLCheck(std::string symbol);
        long double getIndexMovement(std::string symbol);
        public:
	void updateFutCloseWithSpread(std::string symbol);
        void currMinuteBarsFinished();
        void checkSignalForIndex(std::string symbol);
	map<string,struct Stock*> stocks;
        std::string weeklyExpiry="";

    private:
        std::string monthlyExpiry="";
        std::string _weeklyExpiryFile;
        std::string _monthlyExpiryFile;
        void getWeeklyExpiry();
        void getMonthlyExpiry();
        bool expiryWeek=false;
		double _volatility;
		double calculateVolatility(std::string callOption, std::string putOption, int underlying_close);
    private:
        std::string getFormattedDate();
    private:
        void subscribeToOptions(std::string symbol,int close);
        void subscribeToOption(std::string symbol);
	std::set<std::string> tempOptions;
    private:
        vector<std::string> getTradingOptions(std::string symbol);
        //vector<std::string> getTradingOptionsAtVol(std::string symbol);
	
	double _strikesAggresiveness=1; //greater than one represent safe strike and less than one represent agggressive strikes
        bool tradeOnPrice=false;
        map<string, struct Position*> positions;
	private:
		void takeTrades(std::map<std::string,int> desiredQuantities);
	
    private:
        void putSymbolinStockMap();
        void putSymbolinStockMap(std::string symbol);
        void initialiseAll(struct Stock *stp, string stk);
		bool _useSpot=false;
		std::map<std::string,double> futSpotDiff;
		float _optionPerc;
        int _reEntryCoolOff=15;
        bool checkReEntry(std::string index);
		long long getCurrentPNL(std::string index);
		long long int sl=5000000;
		long long int tp=5000000;
        long long getPNLForUnderlying(std::string index);
        bool slReEntry=false;
        bool tpReEntry=true;
	std::string _spotSymbol;


        //all functions and variables for delta change

        std::vector<std::string> getCurrentTradingSymbols(std::string underlying);
        double getPriceChange(std::string optionSymbol);
        void changeStrikes(std::string symbol,std::vector<std::vector<std::string>> strikeChangeInfo); 

        std::vector<std::vector<std::string>> getChangeStrikeOrNot(std::string);
	std::string getNewStrike(std::vector<std::string> strikeChangeInfo);

        std::unordered_map<std::string,double > _entryDeltaOfOption;
        std::unordered_map<std::string,double > _entryPriceOfOption;
        std::pair<std::string,std::string> _currTradingPairs;
	
        double _aggressivePriceChange=0.1;  // if delta increase by this move strike furthur 
	double _exitLegThreshold=_aggressivePriceChange+0.1; // if price of next aggressive price is less than given minPremium (say 5 rs) then it will increase its current _agressivePriceChangeValue to this calue
	long long _exitLegAfter=150000;
	double _passivePriceChange=0.2;  //if delta decreases by this move strike closer to atm
        double deltaMaxForPassive=0.4; //if current delta is already mores than this then don't go for passive strike change (i.e don't go step furthur in towards atm option)
        int strikeLevelMaxForPassive=0; // 0 atm| 1> otm |1 < itm
	double _optionsPremium=20;
	double _minPremium=1;
	double _maxPremium=10;
	int getSynthetic(std::string symbol,std::string expiry);
	bool _useSynthetic=false;
	// bool _callLegSquaredOff=false;
	// bool _putLegSquaredOff=false;
	std::string getOptionAtPrice(std::string symbol,std::string callOrPut,double optionPremium);	
	std::vector<int> getLots(double delta1, double delta2, int totalLots);
	public:
		void buildPosition(std::string symbol ,int qty);
		void buildPosition(std::map<std::string ,int > desiredQty);
		void onSignal1(){}
		void onSignal2(){}
		void onSignal3(){}
		void onSignal4(){}
		void timerEvent(){}
		void TopOfBook(std::string, BookLevel *){}
		void updateData(std::string symbol,uint8_t type,long long int time,uint8_t side,long long int price,int quantity,std::string expiry,int strike,int lot,std::string callput,BookLevel booklvl[10]){}
        long long getIndividualPNL(std::string symbol);
		std::string getOtherOption(std::string symbol);
		std::map<std::string, std::string> _optionPairs;
        double volMultiplier(std::string symbol);



        long long int getOrderId();
	void processTopOfBook(std::string symbol, BookLevel booklvl[]);


//useLess
void send(TCP_ORDER_MULTILEG Order) {
     connector->send(Order);
 }

 void onResponse(TCP_ORDER_MULTILEG order) {}
};
#endif
