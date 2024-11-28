#include "spreadCalc.h"


std::tm make_tm(std::string YYYYMMDD)
{
    int year = stoi(YYYYMMDD.substr(0, 4));
    int month = stoi(YYYYMMDD.substr(4, 2));
    int day = stoi(YYYYMMDD.substr(6, 2));
    std::tm tm = {0};
    tm.tm_year = year - 1900; // years count from 1900
    tm.tm_mon = month - 1;    // months count from January=0
    tm.tm_mday = day;         // days count from 1
    tm.tm_hour = 12;
    tm.tm_min = 0;
    return tm;
}
double getDaysBWDates(std::string startDate, std::string endDate)
{
    auto current_tm=make_tm(startDate);
    auto expiry_tm=make_tm(endDate);
    std::time_t current_seconds = std::mktime(&current_tm);
    std::time_t expiry_seconds = std::mktime(&expiry_tm);
    long double diff_seconds = std::difftime(expiry_seconds, current_seconds);
    long double diff_days = diff_seconds /(24*60*60);
    return diff_days;

}


SpreadCalc::SpreadCalc(Strategy* strat)
{
	
	_strat=strat;
}


double SpreadCalc::getSpread(std::string symbol)
{
	if(! isFutSymbol(symbol))
		return 0;

	double multiplier= getMultiplierForSymbol(symbol);

	double currentSpread= getCurrentSpread(symbol);

	return multiplier*currentSpread;

}


double SpreadCalc::getCurrentSpread(std::string symbol)
{
	//return spot and fut symbol values
	std::string spotSymbol=_strat->getSpotSymbol();

	if(_strat->stocks.find(spotSymbol)==_strat->stocks.end() or _strat->stocks.find(symbol)==_strat->stocks.end())
		return 0;
	
	double spotClose=_strat->stocks[spotSymbol]->close;
	double futClose=_strat->stocks[symbol]->close;

#ifdef __DEBUGPRINT__ 
	std::cout<<"SPOT ANF FUT CLOSE| spot:"<<spotClose<<" futClose:"<<futClose<<std::endl;
#endif
	return futClose-spotClose;
}

double SpreadCalc::getMultiplierForSymbol(std::string symbol)
{
	//get days b/w monthly and weekly expiries
	if(_strat->stocks.find(symbol)==_strat->stocks.end() or _strat->weeklyExpiry=="")
		return 0;
	std::string monthlyExiry=_strat->stocks[symbol]->expiry;
	std::string weeklyExpiry=_strat->weeklyExpiry;
	std::string currDate=_strat->stocks[symbol]->date;

	if(weeklyExpiry==monthlyExiry)return 0;


#ifdef __DEBUGPRINT__
	std::cout<<"MULTIPLIER : monthlyExiry:"<<monthlyExiry<<" weeklyExpiry:"<<weeklyExpiry<<" currDate:"<<currDate<<std::endl;	
#endif

	int daysBwWeeklyAndMonthly=getDaysBWDates(weeklyExpiry,monthlyExiry);
	int daysBwTodayAndMonthly=getDaysBWDates(currDate,monthlyExiry);


#ifdef __DEBUGPRINT__
	std::cout<<"DAYS TO MONTHLY:"<<daysBwTodayAndMonthly<<" DAYSINB/W month and week expiry:"<<daysBwWeeklyAndMonthly<<std::endl;
#endif

	return double(daysBwWeeklyAndMonthly)/double(daysBwTodayAndMonthly);
	//get days to monthly expirues
}

