#include "strat_QuadDist.h"

QuadDist::QuadDist(Strategy * strat_)
{
	_strat=strat_;
	readStrategyConfig(_strat->_configPath);
	_quadDis=new QuadDistribution();
}


void QuadDist::currMinuteBarsFinished(std::string underlying)
{
	//checkSignal(underlying);
	if(isFutSymbol(underlying))
		checkSignal(underlying);
}

double QuadDist::getClose(std::string symbol)
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

bool QuadDist::isStratSquaredOff(std::string symbol)
{
	if(isFutSymbol(symbol))
	{
		return _strat->stocks[symbol]->_squareOffDone;
	}
	return false;
}

void QuadDist::squareOffPositions(std::string symbol)
{
	_strat->squareOffPositions(symbol);
}


void QuadDist::buildPosition(std::string symbol,int qty)
{
	_strat->buildPosition(symbol,qty);
}

void QuadDist::buildPosition(std::map<std::string ,int > desiredQty)
{
	_strat->buildPosition(desiredQty);
}




//STRAT SPECIFIC DEFINITIONS
//
// std::vector<int> QuadDist::getLots()
// {
// 	//TO DO
// 	//
// 	int totalLots=_strat->getTotalLots();

// 	int callLots=_callLegSquaredOff?0:totalLots/2;
// 	int putLots=_putLegSquaredOff?0:((callLots==0)?totalLots/2:totalLots-callLots);
// 	return {callLots,putLots};
// }

void QuadDist::updateCurrTradingOptions(std::string symbol1, std::string symbol2)
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


std::vector<int> QuadDist::maxMinLotsAtmAndExtDist(std::string symbol) //int maxLots,int minLots,int atm,int extremeDistCall,int extremeDistPut
{
	//slope will be maxLots/(otm at given price)
	auto extremeStrikes=getExtremeOptionsStrikes(symbol);
	int minLots=_strat->getTotalLots()*(_minMaxLotsRatio);
	int maxLots=_strat->getTotalLots()-minLots;

	int atm=int(getClose(symbol)/(_strikesGap*100))*(_strikesGap*100);

	std::cout<<"ATM is"<<atm<<std::endl;
	if(double(getClose(symbol)-atm)/double(_strikesGap*100)>=_percStrikeChange)
		atm+=(_strikesGap*100);

	
	return {maxLots,minLots,atm,extremeStrikes[0],extremeStrikes[1]};


	//
}
std::vector<int> QuadDist::getExtremeOptionsStrikes(std::string symbol)
{
	int close=getClose(symbol);
    std::string callOption;
    std::string putOption;
	
    if(_strat->stocks[symbol]->_volReady){
            auto options = _strat->getTradingOptionsAtVol(symbol,_strikesAggresiveness); //to do read from config and check for meaningFul values
	    callOption=options[0],putOption=options[1];
		
    }

    double premium=_optionsPremium*100; //user gives premium in rupees
    if(getClose(callOption)<=_optionsPremium)
	    callOption=getOptionAtPrice(symbol,"CE",premium);
    if(getClose(putOption)<=_optionsPremium)
	    putOption=getOptionAtPrice(symbol,"PE",premium);
    //int strikeLevel=_strikeLevel;

    int callStrike=split(callOption,'_').size()==4?std::stoi(split(callOption,'_')[2])*100:0;
    int putStrike=split(putOption,'_').size()==4?std::stoi(split(putOption,'_')[2])*100:0;

    return {putStrike,callStrike};


}

std::string QuadDist::getSymbol(std::string symbol,int atm,int strike,std::string callOrPut)
{
	std::string expiry=_strat->weeklyExpiry;
	std::string underlying=split(symbol,'_')[0];

	std::vector<string> result;
	std::string sym=underlying+'_'+expiry+'_'+std::to_string(strike/100)+'_'+callOrPut;
	return sym;
}

std::vector<std::pair<std::string,int>> QuadDist::getLotsDistribution(std::string symbol)
{
	auto inputs=maxMinLotsAtmAndExtDist(symbol); //return      maxLots,minLots,atm,putStrikeExtreme,callStrikeExtreme

	//get for put first
	_quadDis->getQuadEqnFromPoints(inputs[0],inputs[1],inputs[2],inputs[2]-inputs[3]);
	auto lotsPut=_quadDis->getLots(inputs[3],inputs[2],_strikesGap*100,_strat->getTotalLots()/2);

	//get Distribution for Call
	_quadDis->getQuadEqnFromPoints(inputs[0],inputs[1],inputs[2],inputs[4]-inputs[2]);
	auto lotsCall=_quadDis->getLots(inputs[2],inputs[4],_strikesGap*100,_strat->getTotalLots()/2);




	//here we need differnt quad eqn for call and put 

	std::vector<std::pair<std::string,int>> distribution;

	#ifdef __DEBUGPRINT__
	//	std::cout<<"PRINTING LOTS:"<<std::endl;
	#endif
//	for (auto combined : {lotsPut, lotsCall})
		for (auto lot : lotsPut)
		{
			auto sym = getSymbol(symbol, inputs[2], lot.first,"PE");
			distribution.push_back(std::make_pair(sym, lot.second));
		}
		for (auto lot : lotsCall)
		{
			auto sym = getSymbol(symbol, inputs[2], lot.first,"CE");
			distribution.push_back(std::make_pair(sym, lot.second));
		}
	return distribution;
}
bool QuadDist::buildQuadPosition(std::string symbol)
{
	
	auto lots=getLotsDistribution(symbol);

		std::map<std::string , int> desiredPos;
		//set default pos to 0 for all to exit strikes not got in current instance

		for(auto &sym:_strat->finalOptions)
			desiredPos[sym]=0;


		for(auto &pos: lots)
		{
			#ifdef __DEBUGPRINT__
					std::cout<<"IN BUILD POSITIONS-SYM:"<<pos.first<<"|LOTS:"<<pos.second<<std::endl;
				#endif
			if(_strat->finalOptions.find(pos.first)!=_strat->finalOptions.end())
			{
				
				desiredPos[pos.first]=-pos.second*_strat->stocks[pos.first]->lot;
			}
		}
		// desiredPos[callOptions]=-lots[0]*_strat->stocks[callOptions]->lot;
		// desiredPos[putOptions]=-lots[1]*_strat->stocks[putOptions]->lot;
		// updateCurrTradingOptions(callOptions,putOptions);
		// _currTradingCall=callOptions;
		// _currTradingPut=putOptions;

		buildPosition(desiredPos);
		return true;
	//return false;

}




bool QuadDist::checkForStratExit(std::string symbol)
{
	//checkSLTP
	return (_strat->checkSLTP(symbol) or  _strat->checkTrailingSL(symbol) or _strat->curTime>=_squareOffTime);

}

// std::vector<std::string> QuadDist::getCurrentTradingSymbols()
// {

//     return {_currTradingCall,_currTradingPut};
// }


std::string isCallOrPut(std::string symbol)
{
	auto info=split(symbol,'_');
	if(info.size()==4)return info[3];

	return "";
}
// void QuadDist::changeStrikes(std::vector<std::vector<std::string>> symbolInfo)
// {
// 	std::map<std::string ,int > desiredQty;
// 	auto lots=getLots();

// 	auto currSymbols=getCurrentTradingSymbols();
// 	std::string callSym=currSymbols[0],putSym=currSymbols[1];
// 	for(auto &info : symbolInfo)
// 	{
// 		if(isCallOrPut(info[0])==isCallOrPut(info[1]) and isCallOrPut(info[1])!="")
// 		{
// 			bool isCall=isCallOrPut(info[1])=="CE";
// 			desiredQty[info[0]]=0;
// 			desiredQty[info[1]]=isCallOrPut(info[0])=="CE" ? lots[0]:lots[1];

// 			isCall?callSym=info[1]:putSym=info[1];
// 		}
// 	}
// 	updateCurrTradingOptions(callSym,putSym);
// 	buildPosition(desiredQty);
	
// }
void QuadDist::checkSignal(std::string symbol)
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
		_positionTaken=buildQuadPosition(symbol);
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
	  _positionTaken=buildQuadPosition(symbol);

       
	}	
}
std::string QuadDist::getOptionAtPrice(std::string symbol,std::string callOrPut,double optionPremium)
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
		for(int i=options.size()-1;i>=0;i--)
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


void QuadDist::readStrategyConfig(std::string strategyConfigFilePath)
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
	else if(tempString=="strikesGap")
		_strikesGap=std::stoi(tempString2);
	else if(tempString=="strikesAggresiveness")
		_strikesAggresiveness=std::stod(tempString2);
	else if(tempString=="minMaxLotsRatio")
	_minMaxLotsRatio=std::stod(tempString2);

		
	
		

		
	}
}


void QuadDist::handleHedger(std::string underlying)
{
	//getCurrent call and put lots 
	
	//checkifHedgeAlready exist

	//EXIST-
	//check for moving close or away
	//
}
