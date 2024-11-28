#ifndef __OPTIONS_STRUCTS_H__
#define __OPTIONS_STRUCTS_H__
vector<std::string> split(std::string str, char delimiter);
double time_diff(string date,string exp,long long s);
bool isOption(std::string symbol);
std::string getTrimmedDate(std::string runDate);
struct Position{
	bool _optionsSubscribed=false;
	std::string symbol;
	struct Stock *stock;
    int pendingPosition;
    int openPosition;
    int currentPosition;
    long double stopLoss;
    long double takeProfit;
    long double stopLossPerc;
    long double takeProfitPerc;
    int lastOrderQuantity;
	int lastOrderPrice;
	long long int totalBuyValue;
	long long int totalSellValue;
    int _entryPrice=0;
    long long _maxPNL;
    long long _currentPNL;
    int getTotalPosition()
    {
        return openPosition+pendingPosition+currentPosition;
    }
    Position(std::string symbol_, struct Stock  *stock_)
    {
        symbol=symbol_;
        stock=stock_;
        pendingPosition=0;
        openPosition=0;
        currentPosition=0;
        stopLoss=1000000000;
        takeProfit=1000000000;
		totalBuyValue=0;
        totalSellValue=0;
        resetPosition();
    };
    void resetPosition()
    {
        pendingPosition=0;
        openPosition=0;
        currentPosition=0;
        stopLoss=1000000000;
        takeProfit=1000000000;
        totalBuyValue=0;
        totalSellValue=0;
        _entryPrice=0;
    }
    void addOrder(char side, int quantity)
    {
        pendingPosition+=(side=='B'?quantity:-quantity);
    }
    void onOrderResponse(char side, int quantity, int price, char responseType)
    {
        lastOrderQuantity=quantity;
        lastOrderPrice=price;
        stopLoss = stopLossPerc * (long double) quantity * (long double) price;
        takeProfit = takeProfitPerc * (long double) quantity * (long double) price;
        if(responseType=='N')
        {
            pendingPosition-=side=='B'?quantity:-quantity;
            openPosition+=side=='B'?quantity:-quantity;
        }
        else if((responseType=='T')||(responseType=='P'))
        {
            openPosition-=side=='B'?quantity:-quantity;
            currentPosition+=side=='B'?quantity:-quantity;
			long long int value = quantity*price;
            if(side=='B')
            {
                totalBuyValue+=value;
            }
            else if(side=='S')
            {
                totalSellValue+=value;
            }
        }
    }
    long double getPercChange(long double close)
    {
        return (close-lastOrderPrice)/lastOrderPrice;
    }
};

struct Stock{
	bool _tradesTaken=false;
	bool _squareOffDone=false;
	bool _slHit=false;
    bool _volReady=false;
    bool _optionSLTPHit=false;
    long long int time;
    std::string date;
    int open;
    int high;
    int low;
    int close;
    float volume;
    int lot;
    int strikePrice;
    std::string expiry;
    std::string callput;
    double delta;
    double gamma;
    double vega;
    double theta;
    std::string symbol;
    int entryPrice;
    int _prevClose;
    double _volatility;
    long long _maxPNL=0;

    bool _thresholdHit=false;
    long long int _squareOffTime;
    Stock()
    {
        time=open=high=low=close=volume=lot=0;
    }
    void updateData(std::string symbol_, std::string date_,long long int time_, int open_, int high_, int low_, int close_, float volume_, int OI_, int lot_, std::string expiry_, std::string callput_)
    {
        _prevClose=close;
        symbol=symbol_;
        date=date_;
        time=time_;
        open=open_;
        high=high_;
        low=low_;
        close=close_;
        volume=volume_;
        lot=lot_;
        expiry=expiry_;
        callput=callput_;
    }
    void calcGreeks(double underlying_close, double passed_vol)
    {
    cout.precision(8);
        if(!isOption(symbol))
        {
            delta=1;
            gamma=0;
            theta=0;
            vega=10;
            return;
        }
        strikePrice=std::stoi(split(symbol,'_')[2]);
#ifdef __SIMMODE__
        strikePrice*=100;
#endif
        double volatility=1.0;
        double rate=0.0;
        double dividends=0.0;
		double exp_time=time_diff(getTrimmedDate(date),expiry,time);
		cout.precision(8);
#ifdef diffModel
        BlackScholes obj(strikePrice,underlying_close,rate,dividends,volatility,exp_time);
        double final_vol=(obj.getImpliedVol_NewtonRaphson(close,'c') + obj.getImpliedVol_NewtonRaphson(close,'p'))/2;
#endif
#ifndef diffModel
        double final_vol=passed_vol;
#endif
	
	#ifdef __DEBUGPRINT__
		std::cout<<"BlackScholes|Strike:"<<strikePrice<<"|UnderlyingClose:"<<underlying_close<<"|Rate:"<<rate<<"|Dividends:"<<dividends<<"|Volatility:"<<final_vol<<"|ExpiryTime:"<<exp_time<<std::endl;
	#endif
	       	BlackScholes obj2(strikePrice,underlying_close,rate,dividends,final_vol,exp_time);
        delta=obj2.getDelta(tolower(callput[0]));
        gamma=obj2.getGamma();
        theta=obj2.getTheta();
        vega=obj2.getVega();

    }
	int getStrike(std::string symbol)
	{
		int strikePrice=std::stoi(split(symbol,'_')[2]);
#ifdef __SIMMODE__
		strikePrice*=100;
#endif
		return strikePrice;
	}
};
#endif
