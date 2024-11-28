#include "priceChase.h"

PriceChase::PriceChase(Strategy * strat_)
{
	_strat=strat_;
	readStrategyConfig(_strat->_configPath);
}

void PriceChase::currMinuteBarsFinished(std::string underlying)
{
	//checkSignal(underlying);
	if(isFutSymbol(underlying))
		checkSignal(underlying);
}

double PriceChase::getClose(std::string symbol)
{
	if(_strat->stocks.find(symbol)!=_strat->stocks.end())
	{
		return _strat->stocks[symbol]->close;
	}	
#ifdef __DEBUGPRINT__
	std::cout<<"Symbol Not Found:"<<symbol<<std::endl;
#endif
	return 0;
}

bool PriceChase::isStratSquaredOff(std::string symbol)
{
	if(isFutSymbol(symbol))
	{
		return _strat->stocks[symbol]->_squareOffDone;
	}
	return false;
}

void PriceChase::squareOffPositions(std::string symbol)
{
	_strat->squareOffPositions(symbol);
}


void PriceChase::buildPosition(std::string symbol,int qty)
{
	_strat->buildPosition(symbol,qty);
}

void PriceChase::buildPosition(std::map<std::string ,int > desiredQty)
{
	_strat->buildPosition(desiredQty);
}




//STRAT SPECIFIC DEFINITIONS
//
std::vector<int> PriceChase::getLots()
{
	//TO DO
	//
	int totalLots=_strat->getTotalLots();

	int callLots=_callLegSquaredOff?0:totalLots/2;
	int putLots=_putLegSquaredOff?0:((callLots==0)?totalLots/2:totalLots-callLots);
	return {callLots,putLots};
}

void PriceChase::updateCurrTradingOptions(std::string symbol1, std::string symbol2)
{
	for (auto &sym : {symbol1, symbol2})
	{
		auto info = split(sym, '_');
		if (info.size() == 4)
		{
			if (info[3] == "CE")
				_currTradingCall = sym;
			else if (info[3] == "PE")
				_currTradingPut = sym;
		}
	}
}
bool PriceChase::buildEntryPosition(std::string symbol)
{
	std::string callOptions=getOptionAtPrice(symbol,"CE",_optionsPremium*100);
	std::string putOptions=getOptionAtPrice(symbol,"PE",_optionsPremium*100);
#ifdef __DEBUGPRINT__
	std::cout<<"Inside Building First Time position:"<<callOptions<<" "<<putOptions<<std::endl;
#endif
	if(_strat->finalOptions.find(callOptions)!=_strat->finalOptions.end() and _strat->finalOptions.find(putOptions)!=_strat->finalOptions.end())
	{
#ifdef __DEBUGPRINT__
	std::cout<<"SYMBOLS FOUND TAKING POSITIONS"<<std::endl;
#endif	
	auto lots=getLots();

		std::map<std::string , int> desiredPos;

		desiredPos[callOptions]=-lots[0]*_strat->stocks[callOptions]->lot;
		desiredPos[putOptions]=-lots[1]*_strat->stocks[putOptions]->lot;
		updateCurrTradingOptions(callOptions,putOptions);
		// _currTradingCall=callOptions;
		// _currTradingPut=putOptions;

		buildPosition(desiredPos);
		return true;
	}
	return false;

}




bool PriceChase::checkForStratExit(std::string symbol)
{
	//checkSLTP
	return (_strat->checkSLTP(symbol) or  _strat->checkTrailingSL(symbol) or _strat->curTime>=_squareOffTime);

}

std::vector<std::string> PriceChase::getCurrentTradingSymbols()
{

    return {_currTradingCall,_currTradingPut};
}
std::vector<std::vector<std::string>> PriceChase::getChangeStrikeOrNot(std::string underlying)   //return vector of string ["aggr/pass","callOrPut option symbol"] if it need to change strike else return empty strings
{
    auto tradingSymbols = getCurrentTradingSymbols(); //this should return call first and then put option


    //get current trading symbol price
    //if price decayed oppoertuity to go closer
    	std::vector<std::vector<std::string>> toChange;


	for(auto &sym : tradingSymbols)
	{
		std::string callOrPut="CE";
		auto splitSym=split(sym,'_');

		if(splitSym.size()==4)
			callOrPut=splitSym[3];
		else continue;

		std::string optionsNew= getOptionAtPrice(underlying,callOrPut,_optionsPremium*100);

		if(optionsNew!=sym)
		{
			#ifdef __DEBUGPRINT__

				std::cout<<"CHANGING STRIKES| OLD:"<<sym<<"| NEW:"<<optionsNew<<std::endl;
			#endif

				toChange.push_back(std::vector<std::string>({sym,optionsNew}));

		}
	}
	return toChange;
}


std::string isCallOrPut(std::string symbol)
{
	auto info=split(symbol,'_');
	if(info.size()==4)return info[3];

	return "";
}
void PriceChase::changeStrikes(std::vector<std::vector<std::string>> symbolInfo)
{
	std::map<std::string ,int > desiredQty;
	auto lots=getLots();

	auto currSymbols=getCurrentTradingSymbols();
	std::string callSym=currSymbols[0],putSym=currSymbols[1];
	for(auto &info : symbolInfo)
	{
		if(isCallOrPut(info[0])==isCallOrPut(info[1]) and isCallOrPut(info[1])!="")
		{
			bool isCall=isCallOrPut(info[1])=="CE";
			desiredQty[info[0]]=0;
			desiredQty[info[1]]=isCallOrPut(info[0])=="CE" ? lots[0]:lots[1];

			isCall?callSym=info[1]:putSym=info[1];
		}
	}
	updateCurrTradingOptions(callSym,putSym);
	buildPosition(desiredQty);
	
}
void PriceChase::checkSignal(std::string symbol)
{
	#ifdef __DEBUGPRINT__
        	std::cout<<"In Price Chase Check Signal"<<symbol<<"|TradeTime:"<<_tradeTime<<"|SqoFFTime:"<<_squareOffTime<<"|currTime:"<<_strat->curTime<<std::endl;
	#endif
	//if position is not build build position first
	if(_strat->curTime >=_tradeTime and _positionTaken==false and _strat->curTime < _squareOffTime and !isStratSquaredOff(symbol))
	{
	#ifdef __DEBUGPRINT__
		std::cout<<"Taking Position first time"<<std::endl;
	 #endif
		_positionTaken=buildEntryPosition(symbol);
	}
	else if(_positionTaken and !isStratSquaredOff(symbol))
	{
	   //if position taken check for any sl or trailing sl or tp

	    bool exitStrat=checkForStratExit(symbol);   // check for any type of sl , tp or trailing SLTP

	    if(exitStrat)
	    {
		    squareOffPositions(symbol);
		    return;
	    }
            auto strikeChangeInfo=getChangeStrikeOrNot(symbol);
	#ifdef __DEBUGPRINT__
	   for(auto &info : strikeChangeInfo)
	   {
		 std::cout<<"DEBUG Strike Change:TIME"<<_strat->curTime<<"|OLD:"<<info[0]<<"|NEW:"<<info[1]<<std::endl;
	   }
	 #endif
            if(strikeChangeInfo.size()!=0)
           {
                changeStrikes(strikeChangeInfo);
           }
	}	
}
std::string PriceChase::getOptionAtPrice(std::string symbol,std::string callOrPut,double optionPremium)
{

	bool lessOnly=true; //just a handy varibale to chosse whether selction options wth premium diff in absolute or just smaller premium
	int close=getClose(symbol);
	int strikesGap=_strat->getStrikesGap();
	int atmStrike=close/(100*strikesGap)*strikesGap; //strike in rupees

	//int multiplier=callOrPut=="CE" ?1:-1;

	std::string underlying=split(symbol,'_')[0];

	// int strikesGap=_strat->_strikesGap; //get options at strike gap of strikesGap only not strikeGapMin

	std::vector<std::string> options; //contains list of all options with premium near to give premium
	int depthToSearchFor=20;


	for(int i=0;i<depthToSearchFor;i++)   //assumption that our required strike won't be far away from 10th strike
	{

		int otmLevel=callOrPut=="CE" ? i+1: -1*i;
		int currLevelStrike=atmStrike+otmLevel*strikesGap;

		std::string currOption=underlying+"_"+_strat->weeklyExpiry+"_"+std::to_string(currLevelStrike)+"_"+callOrPut;

		if(_strat->finalOptions.find(currOption)!=_strat->finalOptions.end() and _strat->stocks.find(currOption)!=_strat->stocks.end())
		{
			options.push_back(currOption);
		}

	}

	double closest=999999999;

	std::string closestPriceOption;

	for(auto option: options)
	{
		int optionLtp=getClose(option);
		std::cout<<"DEBUG "<<option<<" close:"<<optionLtp<<std::endl;

		int diff=(optionPremium-optionLtp);
		if( (lessOnly==false and std::abs(diff)<closest) or (lessOnly==true and diff>=0 and diff<closest))
		{
		#ifdef __DEBUGPRINT__
			std::cout<<"OPT:"<<option<<"|pre:"<<optionLtp<<std::endl;
		#endif
			closestPriceOption=option;
			closest=diff;
		}



	}


	if(closestPriceOption=="" or closest==999999999)//then we probably didn't got any option in given range
	{
		#ifdef __DEBUGPRINT__
			std::cout<<"Premium not within the limit taking the last strik"<<std::endl;
		#endif
		for(int i=depthToSearchFor-1;i>=0;i--)
		{
			if(_strat->finalOptions.find(options[i])!=_strat->finalOptions.end())
			{
				closestPriceOption=options[i];
				break;
			}
		}
	}
	
#ifdef __DEBUGPRINT__
	std::cout<<"closestPriceOption:"<<closestPriceOption<<"|price:"<<getClose(closestPriceOption)<<std::endl;
#endif
	return closestPriceOption;

	


}


void PriceChase::readStrategyConfig(std::string strategyConfigFilePath)
{
    ifstream myFile(strategyConfigFilePath);
    if(!myFile.is_open())
    {
        //std::cout<<"here"<<std::endl;
        cout << "error opening config data file\n";
        return;
    }

    while(!myFile.eof())
    {
        string tempString,tempString2;
        getline(myFile,tempString);
        stringstream ss(tempString);

        getline(ss, tempString, '=');
        getline(ss, tempString2, '=');
        // cout << tempString << " " << tempString2 << endl;

        if(tempString == "squareOffTime")
            _squareOffTime = std::stoll(tempString2);
	else if(tempString=="tradeTime")
	    _tradeTime=std::stoll(tempString2);
	else if(tempString=="optionsPremium")
		_optionsPremium=std::stod(tempString2);
		

		
	}
}


void PriceChase::handleHedger(std::string underlying)
{
	//getCurrent call and put lots 
	
	//checkifHedgeAlready exist

	//EXIST-
	//check for moving close or away
	//
}
