#include "strategy.h"
#include<fstream>
#include "blackScholes-orignal.cpp"
#include<bits/stdc++.h>

//#define __DEBUGPRINT__
using namespace std;


int Strategy::getSynthetic(std::string symbol,std::string expiry)
{
        int close = stocks[symbol]->close;

        int levels=1;
        int notfound=0;
        int synSum=0;
        std::string underlying=split(symbol,'_')[0];
        for(int i=-levels;i<=levels;i++)
        {
                int strike= close/(100*_strikesGap)*_strikesGap+i*_strikesGap;
                std::string optionCe=underlying+'_'+expiry+'_'+std::to_string(strike)+"_CE";
                std::string optionPe=underlying+'_'+expiry+'_'+std::to_string(strike)+"_PE";
	#ifdef __DEBUGPRINT__
                printf("Options for synthetic:%s,%s \n",optionCe.c_str(),optionPe.c_str());
	#endif
                if(finalOptions.find(optionCe)==finalOptions.end() or finalOptions.find(optionPe)==finalOptions.end())
                {
			#ifdef __DEBUGPRINT__
                        printf("options pair not found \n");
			#endif
                        notfound++;
                        continue;
                }

                int syn=strike*100 + stocks[optionCe]->close - stocks[optionPe]->close;
	#ifdef __DEBUGPRINT__
                printf("SYN SUM IS %d + %d + %d= %d \n",strike*100,stocks[optionCe]->close,stocks[optionPe]->close,syn);
	#endif
                synSum+=syn;
        }
        int finalSyn=0;
        if (2*levels+1 > notfound)
                finalSyn=synSum/(2*levels+1 - notfound);

        return finalSyn;
}

double Strategy::volMultiplier(std::string symbol)
{
	if(weeklyExpiry!=runDate)
		return 1;
	Stock * st =stocks[symbol];
        double volatility=1.0;
        double rate=0.0;
        double dividends=0.0;
        double exp_time=time_diff(getTrimmedDate(st->date),weeklyExpiry,curTime);
	
	return sqrt(exp_time*365);		
}
vector<std::string> Strategy::getTradingOptionsAtVol(std::string symbol,double strikesAggresiveness)
{
        double vol=stocks[symbol]->_volatility*volMultiplier(symbol);
        int underlyingClose=stocks[symbol]->close;
       double distanceOfStrikes= vol*strikesAggresiveness/16.0*(underlyingClose*1.0f);

       if(weeklyExpiry==monthlyExpiry)
       {

       }
        printf("Dist OF Strikes:%f Vol:%f close:%d strikesAggressiveness:%f \n",distanceOfStrikes,vol,underlyingClose,strikesAggresiveness);

        std::cout<<std::endl;
        int callStrike = underlyingClose + distanceOfStrikes;

        int putStrike = underlyingClose - distanceOfStrikes;

        int strikesGap=_strikesGap;
        int finalCallStrike = callStrike/(strikesGap*100)*(strikesGap*100);
        int finalPutStrike = putStrike/(strikesGap*100)*(strikesGap*100);


        double percDiffCall=(std::abs(callStrike-finalCallStrike))*1.0f/(_strikesGap*100.0);
        double percDiffPut=(std::abs(putStrike-finalPutStrike))*1.0f/(_strikesGap*100.0);

        if(percDiffCall>_percStrikeChange)
                finalCallStrike+=strikesGap*100;

        if(percDiffPut > _percStrikeChange)
                finalPutStrike+=strikesGap*100;

        printf("CALL STRIKE: %d PUT STRIKE %d \n",finalCallStrike,finalPutStrike);

        finalPutStrike/=100;
        finalCallStrike/=100;

        std::string underlying=split(symbol,'_')[0];
        std::string callOption=underlying+"_"+weeklyExpiry+"_"+std::to_string(finalCallStrike)+"_CE";
        std::string putOption=underlying+"_"+weeklyExpiry+"_"+std::to_string(finalPutStrike)+"_PE";

        printf("call options: %s put options: %s",callOption.c_str(),putOption.c_str());
        std::cout<<std::endl;
        return {callOption,putOption};
}

void Strategy::updateData(std::string symbol, std::string date,long long int time,int open,int high, int low, int tempClose, float volume,int OI, int lot, std::string expiry, std::string callput)
{
	int close=tempClose;
	if(_useSpot)
	{
		if(futSpotDiff.find(symbol)!=futSpotDiff.end())
		{
			close=close-futSpotDiff[symbol]*100;
	#ifdef __DEBUGPRINT__
			// std::cout<<"SpotFutDiff:"<<futSpotDiff[symbol]<<"|CLose:"<<close<<"|tempClose:"<<tempClose<<std::endl;
	#endif
		}

	}
    cout.precision(4);
#ifdef __DEBUGPRINT__
    cout.setf(ios::fixed,ios::floatfield);
     std::cout<<"UPDATE-DATA:"<<symbol<<" "<<date<<" "<<time<<" "<<open<<" "<<high<<" "<<low<<" "<<close<<" "<<volume<<" "<<OI<<" "<<lot<<" "<<expiry<<std::endl;
#endif

    if(!_totalLotsSet and isOption(symbol))
    {
	    if(_totalQuantity!=-1)
	    	_totalLots=_totalQuantity/lot;

	    _totalQuantity=lot*_totalLots;
	    _totalLotsSet=true;
	    #ifdef __DEBUGPRINT__
	    	cout<<"TOTAL LOTS:"<<_totalLots<<","<<_totalQuantity<<","<<lot<<std::endl;
	    #endif
    }
    runDate = date;
    curTime = time;
    if(stocks.find(symbol)==stocks.end())
        putSymbolinStockMap(symbol);
    if(monthlyExpiry=="")
    {
        monthlyExpiry=expiry;
    }
    if((weeklyExpiry=="") && !expiryWeek)
    {
        getWeeklyExpiry();
        getMonthlyExpiry();
        std::cout<<"Got weekly expiry"<<weeklyExpiry<<"monthly"<<monthlyExpiry<<std::endl;
        if(weeklyExpiry==monthlyExpiry)
        {
            expiryWeek=true;
            //weeklyExpiry="";
        }
    }
    stocks[symbol]->updateData(symbol,date,time,open, high, low, close, volume, OI, lot,expiry,callput);
        
    if ((callput=="CE") || (callput=="PE"))
    {
        if(finalOptions.find(symbol)==finalOptions.end())
        {
            finalOptions.insert(symbol);
        }
    }
    else if(isFutSymbol(symbol))
    {
        if(time>=_subscriptionTime)
        {
			/*
            if(stocks[symbol]->_optionsSubscribed==false)
            {
               stocks[symbol]->_optionsSubscribed=true;
            }
			*/
            subscribeToOptions(symbol,close);
        }
    }
}

void Strategy::updateVolatility(std::string symbol)
{
	std::string callOption=getOption(symbol,stocks[symbol]->close, 1, weeklyExpiry, "CE","OTM") ;
	std::string putOption=getOption(symbol,stocks[symbol]->close, 1, weeklyExpiry, "PE","OTM") ;
#ifdef __DEBUGPRINT__
	std::cout<<"Inside Vol calc:"<<callOption<<"|"<<putOption<<std::endl;
#endif
	if((finalOptions.find(callOption)!=finalOptions.end()) && (finalOptions.find(putOption)!=finalOptions.end()))
	{
		double volatility=calculateVolatility(callOption, putOption, stocks[symbol]->close);
#ifdef __DEBUGPRINT__
		printf("VolCalc|Symbol:%s|Vol:%f\n",symbol.c_str(), volatility);
#endif
		stocks[symbol]->_volatility=volatility;
		stocks[symbol]->_volReady=true;
	}
}

// std::vector<int> Strategy::getLots(double delta1, double delta2, int totalLots)
// {
//     int lot1, lot2;
//     double minDelta=INT_MAX;
//     int finalLots1, finalLots2;

//     int callLots=_callLegSquaredOff?0:totalLots/2;
//     int putLots=_putLegSquaredOff?0:((callLots==0)?totalLots/2:totalLots-callLots);

//     return {callLots,putLots};
//     //return {totalLots/2,totalLots-totalLots/2}; // no delta hedge
// #ifndef absLots
//     int lotSum=totalLots*0.8;
// #endif
// #ifdef absLots
//     int lotSum=totalLots;
// #endif
//     for(;lotSum<=totalLots;lotSum++)
//     {
//         for(int i=0;i<=lotSum;i++)
//         {
//             int tempLots1=i;
//             int tempLots2=lotSum-i;
//             double netDelta=(long double)tempLots1*delta1+(long double)tempLots2*delta2;
//             if(std::abs(netDelta)<minDelta)
//             {
//                 minDelta=std::abs(netDelta);
//                 finalLots1=tempLots1;
//                 finalLots2=tempLots2;
//             }
//         }
//     }
// #ifdef __DEBUGPRINT__
//     std::cout<<"TotalLots:"<<totalLots<<"|MinDelta:"<<minDelta<<"|Delta1:"<<delta1<<"|Delta2:"<<delta2<<"|Lots1:"<<finalLots1<<"|Lots2:"<<finalLots2<<std::endl;
// #endif
//     return {finalLots1,finalLots2};
// }
double Strategy::calculateVolatility(std::string callOption, std::string putOption, int underlying_close)
{
	struct Stock *ce=stocks[callOption], *pe=stocks[putOption];
	double volatility=1.0;
	double rate=0.0;
	double dividends=0.0;
	double exp_time=time_diff(getTrimmedDate(ce->date),ce->expiry,ce->time);
	BlackScholes obj_ce(stocks[callOption]->getStrike(callOption),underlying_close,rate,dividends,volatility,exp_time);

#ifdef __DEBUGPRINT__
	printf("VolCalc|Option:%s|Strike:%d|Close:%d|Rate:%f|Dividend:%f|Volatility:%f|Exp_time:%f|Close:%d|Vol:%f\n",callOption.c_str(), stocks[callOption]->getStrike(callOption), underlying_close, rate, dividends, volatility, exp_time,ce->close,obj_ce.getImpliedVol_NewtonRaphson(ce->close,'c'));
#endif
	BlackScholes obj_pe(stocks[putOption]->getStrike(putOption),underlying_close,rate,dividends,volatility,exp_time);
#ifdef __DEBUGPRINT__
	printf("VolCalc|Option:%s|Strike:%d|Close:%d|Rate:%f|Dividend:%f|Volatility:%f|Exp_time:%f|Close:%d|Vol:%f\n",putOption.c_str(), stocks[putOption]->getStrike(putOption), underlying_close, rate, dividends, volatility, exp_time,pe->close,obj_pe.getImpliedVol_NewtonRaphson(pe->close,'p'));
#endif
	return (obj_ce.getImpliedVol_NewtonRaphson(ce->close,'c') + obj_pe.getImpliedVol_NewtonRaphson(pe->close,'p'))/2;
}
void Strategy::updateFutCloseWithSpread(std::string symbol)
{
	double spread=_spreadCalc->getSpread(symbol);

	stocks[symbol]->close=stocks[symbol]->close - spread;

#ifdef __DEBUGPRINT__
	std::cout<<"spread| close:"<<stocks[symbol]->close<<" "<<spread<<std::endl; 
#endif
}
void Strategy::printCurrentPosition()
{
    for (auto pos : positions)
    {
        if (pos.second->currentPosition != 0)
            std::cout << "CURR POS:" << pos.first << "|POS:" << pos.second->currentPosition <<"|LTP:"<<stocks[pos.first]->close<< std::endl;
    }
}
void Strategy::currMinuteBarsFinished()
{
    for (auto it = validSymbols.begin(); it != validSymbols.end(); it++)
    {
        std::string symbol = *it;
        if (isFutSymbol(symbol))
        {
            if (_useSynthetic)
            {
                int syn = getSynthetic(symbol, weeklyExpiry);
                if (syn != 0)
                    stocks[symbol]->close = syn;
            }
            else
            {
                updateFutCloseWithSpread(symbol);
            }

            updateVolatility(symbol);
            //_priceChase->currMinuteBarsFinished(symbol);
            _quadDist_strat->currMinuteBarsFinished(symbol);
            // checkSignalForIndex(symbol);
        }
    }
#ifdef __DEBUGPRINT__
    printCurrentPosition();
#endif
}
long long Strategy::getCurrentPNL(std::string index)
{
    long long int totalPNL=0;
    for(auto pair:positions)
    {
        std::string symbol=pair.first;
        if(symbol.size()>0)
        {
            if(split(symbol,'_')[0]==split(index,'_')[0])
            {
                long long pnl = positions[symbol]->totalSellValue-positions[symbol]->totalBuyValue+positions[symbol]->currentPosition*stocks[symbol]->close;
                totalPNL+=pnl;
            }
        }
    }
    /*if(printData)
     *     std::cout<<index<<":"<<totalPNL<<std::endl;*/
    return totalPNL;
}


bool Strategy:: checkTrailingSL(std::string index)
{
    long long currPNL = getCurrentPNL(index);
    #ifdef __DEBUGPRINT__
    	  std::cout<<"CurrPnl:"<<currPNL<<"|_trailingSLThreshold*_totalQuantity:"<<_trailingSLThreshold*_totalQuantity<<"|SLTHRESHLD:"<<stocks[index]->_thresholdHit<<std::endl;
   #endif

    if(currPNL > _trailingSLThreshold*_totalQuantity and stocks[index]->_thresholdHit==false)
    {
        stocks[index]->_thresholdHit=true;
        stocks[index]->_maxPNL=currPNL;

        #ifdef __DEBUGPRINT__

        std::cout<<"TRAILING SL THRESHOLD REACHED , ACTIVATING TRAIL SL"<<std::endl;

        #endif
    }

    if( stocks[index]->_thresholdHit==true)
    {
        stocks[index]->_maxPNL = std::max(stocks[index]->_maxPNL,currPNL);
        long double perc = getPercDiff(stocks[index]->_maxPNL,currPNL);
     #ifdef __DEBUGPRINT__
         printf("Symbol:%s|CurrPNL:%lld|TrailingSLThreshold:%Lf|maxPNL:%lld|Perc:%Lf \n",index.c_str(), currPNL, _trailingSLThreshold,stocks[index]->_maxPNL,perc);
    #endif
    if( perc < -_trailingSLPerc )
    {
        return true;
    }
    return false;


    }
    return false;
}
bool Strategy::checkSLTP(std::string index)
{
    long long currPNL = getCurrentPNL(index);
    //if ((currPNL < -sl*_totalLots) || (currPNL > tp*_totalLots))
    if ((currPNL < -sl*_totalQuantity) || (currPNL > tp*_totalQuantity))
    {
	#ifdef __DEBUGPRINT__
        printf("SLTPHIT|Symbol:%s|CurrentPNL:%lld|SL:%lld|TP:%lld|TotalLots:%d|TotalQuantity:%lld\n",index.c_str(), currPNL,sl,tp,_totalLots,_totalQuantity);
        #endif
	return true;
    }
    return false;
}

// bool Strategy::checkReEntry(std::string index)
// {
//     if(stocks[index]->_squareOffDone==true)
//     {
// 	#ifdef __DEBUGPRINT__
//         std::cout<<curTime<<" checking ReEntry "<<index<<" "<<stocks[index]->_squareOffDone<<" "<<stocks[index]->_squareOffTime<<" "<<curTime<<" "<<time_diff(stocks[index]->_squareOffTime,curTime)<<std::endl;
//         #endif
// 	if(time_diff(curTime,stocks[index]->_squareOffTime)>_reEntryCoolOff)
//         {
//             long long lastRunPNL = getPNLForUnderlying(index);
//             std::cout<<"LastRunPNL"<<lastRunPNL<<" "<<index<<" "<<slReEntry<<" "<<tpReEntry<<std::endl;
//             if ( ((lastRunPNL < 0) && slReEntry) || ((lastRunPNL > 0) && tpReEntry) )
//             {
//                 std::cout<<"Setting re entry"<<std::endl;
//                 stocks[index]->_squareOffDone=false;
//                 stocks[index]->_optionSLTPHit=false;
//                 stocks[index]->_tradesTaken=false;
//                 std::set<std::string> symbolsToResetPositionFor;
//                 for(auto pair:positions)
//                 {
//                     std::string symbol=pair.first;
//                     struct Position* position=pair.second;
//                     if(symbol.size()>0)
//                     {
//                         if(split(symbol,'_')[0]==split(index,'_')[0])
//                         {
//                             position->resetPosition();
//                         }
//                     }
//                 }
//                 return true;
//             }
//         }
//     }
//     return false;
// }

void Strategy::checkSignal(std::string symbol)
{
}
// void Strategy::checkSignalForIndex(std::string symbol)
// {
//     if(isOption(symbol))
//         return;
//     if(!stocks[symbol]->_volReady)
//         return;
//     if(curTime>=_tradeTime)
//     {
//         double underlying_close=stocks[symbol]->close;
//         auto values=getTradingOptions(symbol);

// 	if((values.size()>1) and (finalOptions.find(values[0])==finalOptions.end() or finalOptions.find(values[1])==finalOptions.end()))
// 	{
// 		subscribeToOption(values[0]);
// 		subscribeToOption(values[1]);

// 		return;
// 	}
//         if((stocks[symbol]->_tradesTaken==false) && (values.size()>0))
//         {
//             stocks[symbol]->_tradesTaken=true;
//             std::string callOption=values[0];
//             std::string putOption=values[1];
//             stocks[callOption]->calcGreeks(underlying_close, stocks[symbol]->_volatility);
//             stocks[putOption]->calcGreeks(underlying_close, stocks[symbol]->_volatility);
//             // _entryDeltaOfOption[callOption]=stocks[callOption]->delta;
//             // _entryDeltaOfOption[putOption]=stocks[putOption]->delta;
//             _entryPriceOfOption[callOption]=stocks[callOption]->close;
//             _entryPriceOfOption[putOption]=stocks[putOption]->close;


//             stocks[symbol]->entryPrice=stocks[symbol]->close;
//             auto lots = getLots(stocks[callOption]->delta, stocks[putOption]->delta,_totalLots);
//             std::cout<<lots[0]<<" "<<lots[1]<<std::endl;
//             lastDistribution={lots[0],lots[1]};
//            // lastSkew=getSkewness(lots[0],lots[1]);
//             send('S',lots[0]*stocks[callOption]->lot,100,callOption,'N',getOrderId(),false);
//             send('S',lots[1]*stocks[putOption]->lot,100,putOption,'N',getOrderId(),false);
//             _prevPositions[callOption]=lots[0]*stocks[callOption]->lot;
//             _prevPositions[putOption]=lots[1]*stocks[putOption]->lot;
//             _currTradingPairs={callOption,putOption};
//             return;
//         }
//     }
//     if(stocks[symbol]->_tradesTaken)
//     {
//         if(curTime>=_squareOffTime or checkSLTP(symbol))
//         {
//             if(stocks[symbol]->_squareOffDone==false)
//             {
//                 squareOffPositions(symbol);
//                 stocks[symbol]->_squareOffDone=true;
//                 stocks[symbol]->_squareOffTime=curTime;
//             }
//         }
//         else if(stocks[symbol]->_squareOffDone==false)
//         {
           

//             auto strikeChangeInfo=getChangeStrikeOrNot(symbol);
//            // std::cout<<"DEBUG STRIKE CHANGE INFO:"<<strikeChangeInfo[0]<<" | "<<strikeChangeInfo[1]<<std::endl;
// 	   //
// 	#ifdef __DEBUGPRINT__
// 	   for(auto &info : strikeChangeInfo)
// 	   {
// 		 std::cout<<"DEBUG Strike Change:TIME"<<curTime<<"|"<<info[0]<<"|"<<info[1]<<std::endl;
// 	   }
// 	 #endif
//             if(strikeChangeInfo.size()!=0)
//             {
//                 changeStrikes(symbol,strikeChangeInfo);
//             }
//         }
//     }
// }

// std::vector<std::string> Strategy::getCurrentTradingSymbols(std::string underlying)
// {
//     std::string call= _currTradingPairs.first;
//     std::string put= _currTradingPairs.second;

//     if(call.size()==0 or put.size()==0 or !isOption(call) or !isOption(put))
//         return {"",""};
    
//     call=split(call,'_')[3]=="CE"?call:put;
//     put=split(put,'_')[3]=="PE"?put:call;


//     return {call,put};
// }

// std::string Strategy::getNewStrike(std::vector<std::string> strikeChangeInfo)
// {

//     bool moveInwards = false;
//     std::string optionsSymbol = strikeChangeInfo[1];

//    // string optionsIsCorP = split(optionsSymbol, '_')[3] == "CE" ? "CE" : "PE";
//     if (strikeChangeInfo[0] == "PASS")
//         moveInwards = true;
//     #ifdef __DEBUGPRINT__
//     	std::cout<<"DEBUG CHANGE STRIKE:"<<optionsSymbol<<"|MoveIn:"<<moveInwards<<std::endl;
//     #endif
//     std::string newOption = getNextStrikeCallOrPutOption(optionsSymbol, _strikesGap, moveInwards);

//     if(finalOptions.find(newOption)!=finalOptions.end() and stocks[newOption]->close<_minPremium*100)
//     {
// 	  //if aggressive strike change and next strike has price less than 5 rs 
// 	  //then wait for aggressive price chase by x percent more (above already set aggressive price chase)
// 	   if(strikeChangeInfo[0]=="AGGR")
// 	   {
// 		double priceChange=getPriceChange(optionsSymbol);
// 		if(priceChange<=_exitLegThreshold)
// 		{
// 			 #ifdef __DEBUGPRINT__
// 			        std::cout<<"Next premium is less than:"<<_minPremium<<"| curr priceChange:"<<priceChange<<std::endl;
//    			 #endif
// 				//return the same old option
// 				return optionsSymbol;
// 		}
// 		else if(curTime >= _exitLegAfter and runDate==weeklyExpiry)
// 		{
// 			//exit the leg 
// 			//
// 			auto callOrPut=split(optionsSymbol,'_')[3];
// 			if(callOrPut=="CE")
// 			{
// 				_callLegSquaredOff=true;
// 			}
// 			else if(callOrPut=="PE")
// 			{
// 				_putLegSquaredOff=true;
// 			}	
			

// 			#ifdef __DEBUGPRINT__
// 		        	std::cout<<"SQUARING OFF THIS LEG:"<<optionsSymbol<<"|CurTime:"<<curTime<<std::endl;
// 			#endif
// 			return optionsSymbol;

// 		}
// 	   }


// 	    std::string underlying=split(newOption,'_')[0]+"_F1";
// 	#ifdef __DEBUGPRINT__
// 	    printf("new option for strike change has premium less than the minimum options: %s | premium %d | min %f \n",newOption.c_str(),stocks[newOption]->close,_minPremium*100);
// 	#endif 
// 	    return getOptionAtPrice(underlying,split(newOption,'_')[3],_minPremium*100); 
//     }

//     else if(finalOptions.find(newOption)!=finalOptions.end() and stocks[newOption]->close>_maxPremium*100)
//     {
// 	std::string underlying=split(newOption,'_')[0]+"_F1";
// 	#ifdef __DEBUGPRINT__
// 	printf("new option for strike change has premium more than the maximum options: %s | premium %d | max %f \n",newOption.c_str(),stocks[newOption]->close,_maxPremium*100);
// 	#endif
// 	return getOptionAtPrice(underlying,split(newOption,'_')[3],_maxPremium*100);
//     }
//      return newOption;     
// }
// void Strategy::changeStrikes(std::string symbol, std::vector<std::vector<std::string>> strikeChangeInfo)
// {
//     auto currTradingSymbols = getCurrentTradingSymbols(symbol);

//     std::string callOption=currTradingSymbols[0],putOption=currTradingSymbols[1], callOptionNew=currTradingSymbols[0],putOptionNew=currTradingSymbols[1];

//     for(auto &info:strikeChangeInfo)
//     {
// 	    if(info.size()==2)
// 	    {
// 		bool isCall=split(info[1],'_')[3]=="CE"?true:false;

// 		std::string nextOption = getNewStrike(info);
// 		if(!isStrikeLevelBreached(nextOption,info[0]))
// 		{
// 			if(isCall)
// 				callOptionNew=nextOption;
// 			else
// 				putOptionNew=nextOption;
// 		}

// 	    }
//     }
//     if (stocks.find(callOptionNew) == stocks.end() ) // stocks.find(putOptionNew) == stocks.end())
//     {
// 	#ifdef __DEBUGPRINT__
//         std::cout << "NOT CHANGING CALL STRIKE| SYM NOT FOUND:"<<callOptionNew<<"|old:"<<callOption<<std::endl;
//         #endif
// 	    callOptionNew=callOption;
// 	//return;
    
//     }
	
//     if (stocks.find(putOptionNew) == stocks.end() ) // stocks.find(putOptionNew) == stocks.end())
//     {
// 	#ifdef __DEBUGPRINT__
//         std::cout << "NOT CHANGING PUT STRIKE| SYM NOT FOUND:"<<putOptionNew<<"|old:"<<putOption<<std::endl;
//         #endif
// 	    putOptionNew=putOption;
// 	//return;
    
//     }

//     double underlying_close = stocks[symbol]->close;
//     for(auto &option:{callOption,putOption,callOptionNew,putOptionNew})
//     {
// 	if(stocks.find(option)!=stocks.end())
//     		stocks[option]->calcGreeks(underlying_close, stocks[symbol]->_volatility);
//     }

//     if(callOptionNew==callOption and putOption==putOptionNew)
//     {
// 	#ifdef __DEBUGPRINT__
// 	     std::cout << "NOT CHANGING STRIKES| SYMBOLS ARE SAME:"<<callOptionNew<<"|"<<putOptionNew<<std::endl;
// 	 #endif 
// 	     //return;
//     }

//     std::unordered_map<std::string,double > _entryPriceOfOption;

//     if(callOptionNew!=callOption)       //this confirms we are going to change strike
//          _entryPriceOfOption[callOptionNew]=stocks[callOptionNew]->close;

//     if(putOptionNew!=putOption)
// 	  _entryPriceOfOption[putOptionNew]=stocks[putOptionNew]->close;

    

//     auto lots = getLots(stocks[callOptionNew]->delta, stocks[putOptionNew]->delta, _totalLots);
// 	#ifdef __DEBUGPRINT__
//     	  std::cout << "LOTS StrikeChange:" << callOptionNew << "|" << putOptionNew << "|" << lots[0] << "|" << lots[1] << "|" << stocks[callOptionNew]->delta << "|" << stocks[putOptionNew]->delta << std::endl;
// 	 #endif
//     std::map<std::string,int> desiredQuantities;
//     desiredQuantities[callOption]=0;
//     desiredQuantities[putOption]=0;
//     desiredQuantities[callOptionNew] = -lots[0] * stocks[callOptionNew]->lot;
//     desiredQuantities[putOptionNew] = -lots[1] * stocks[putOptionNew]->lot;
//     _optionPairs[callOptionNew] = putOptionNew;

//     takeTrades(desiredQuantities);
// }
bool Strategy::canSL()
{
    int curTimeInMins = minutesSinceMidnight(curTime);
    int exitTimeInMins = minutesSinceMidnight(_squareOffTime);
    if(curTimeInMins > exitTimeInMins - _earlySquareOff)
    {
        return false;
    }
    return true;
}



// double Strategy::getPriceChange(std::string symbol)
// {
//     std::string underlying=split(symbol,'_')[0]+"_F1";
//     double underlying_close= stocks[underlying]->close;

//     //stocks[symbol]->calcGreeks(underlying_close, stocks[underlying]->_volatility);

//     double currPrice=stocks[symbol]->close;
//     double previous=_entryPriceOfOption[symbol];

// #ifdef __DEBUGPRINT__
//     std::cout<<"SYM:"<<symbol<<"|Curr:"<<currPrice<<"|Prev:"<<previous<<"|CurTime:"<<curTime<<"|PNL:"<<getCurrentPNL(underlying)/100.0<<std::endl;
// #endif
//     return (currPrice-previous)/previous;
// }
// std::vector<std::vector<std::string>> Strategy::getChangeStrikeOrNot(std::string underlying)   //return vector of string ["aggr/pass","callOrPut option symbol"] if it need to change strike else return empty strings
// {
//     auto tradingSymbols = getCurrentTradingSymbols(underlying); //this should return call first and then put option
//     double priceChangeCall=0,priceChangePut=0;
//     if(!_callLegSquaredOff)
//     	priceChangeCall = getPriceChange(tradingSymbols[0]);
//     if(!_putLegSquaredOff)
//     	priceChangePut = getPriceChange(tradingSymbols[1]);
   
//     // double absDeltaCall=std::abs(stocks[tradingSymbols[0]]->delta);
//     // double absDeltaPut=std::abs(stocks[tradingSymbols[1]]->delta);

//     //double currentDelta =stocks[symbol]->delta

//     //somehow don't go passive if  currDelta is already more than some specific value or probaby 0.5 (!!its already in ITM then)

//     //go for aggressive first
//     //
//     std::vector<std::vector<std::string>> result; 
// #ifdef __DEBUGPRINT__
//     std::cout<<"PriceChange call:"<<priceChangeCall<<"|put"<<priceChangePut<<std::endl;
//  #endif
//    if(priceChangeCall !=0 and priceChangeCall >=_aggressivePriceChange or -priceChangeCall >_passivePriceChange)
//    {
// 		std::string aggOrPass=priceChangeCall>0?"AGGR":"PASS";
// 		result.push_back({aggOrPass,tradingSymbols[0]});
//    }
//    if(priceChangePut !=0 and priceChangePut >=_aggressivePriceChange or -priceChangePut >_passivePriceChange)
//    {
// 		std::string aggOrPass=priceChangePut>0?"AGGR":"PASS";
// 		result.push_back({aggOrPass,tradingSymbols[1]});
//    }

//     return result;

// /**
//     if (priceChangeCall > 0 or priceChangePut > 0)
//     {
//         if (priceChangeCall > _aggressivePriceChange or priceChangePut > _aggressivePriceChange)
//         {
//             std::string callOrPut=priceChangeCall > _aggressivePriceChange ? tradingSymbols[0] : tradingSymbols[1] ;   //remeber edge case when both delta rise | is this even possible  ?
// 		result.push_back({"AGGR",callOrPut});
//               //return {"AGGR",callOrPut};
//         }
          
//     }
//     //now checkForPassi
//     if (priceChangeCall < 0 or priceChangePut < 0)
//     {
//         if ((-1*priceChangeCall > _passivePriceChange ) or (-1*priceChangePut > _passivePriceChange))
//            {
//              std::string callOrPut=-priceChangeCall > _passivePriceChange ? tradingSymbols[0] : tradingSymbols[1] ;   //remeber edge case when both delta rise | is this even possible  ?
//              result.push_back({"PASS",callOrPut}); 
// 	     //return {"PASS",callOrPut};
//            }
//     }
// **/
//     //return {"",""};
// }
// bool Strategy::isSL(std::string symbol)
// {
//     long double valToCheck;
//     if(_slType=="SUM_DELTA")
//     {
//         valToCheck=getTotalDelta(symbol);
// 	#ifdef __DEBUGPRINT__
// 		printf("Delta:%lld:%s:%Lf\n",curTime,symbol.c_str(),valToCheck);
//    	 #endif
//     }
//     else if(_slType=="INDIVIDUAL_SL")
//     {
//         valToCheck=getIndividualSLCheck(symbol);
//     }
//     else if(_slType=="INDEX_MOVEMENT")
//     {
//         valToCheck=getIndexMovement(symbol);
//     }
//     if((valToCheck<-_SLValue)||(valToCheck>_SLValue))
//     {
//         return true;
//     }
//     return false;
// }
// long double Strategy::getIndexMovement(std::string symbol)
// {
//     double underlying_close=stocks[symbol]->close;
//     double entryPrice=stocks[symbol]->entryPrice;
//     return (underlying_close-entryPrice)/(underlying_close*0.02);
// }
// long double Strategy::getIndividualSLCheck(std::string index)
// {
//     double underlying_close=stocks[index]->close;
//     long double total_delta=0;
//     long double total_positions=0;
//     long double minChange=INT_MAX;
//     for(const auto &pair:positions)
// 	{
// 		std::string symbol=pair.first;
// 		if(split(index,'_')[0]!=split(symbol,'_')[0])
// 			continue;
// 		struct Position* position=pair.second;
// 		struct Stock* stock=stocks[symbol];
// 		if(position->getTotalPosition()!=0)
// 		{
// 			minChange=std::min(minChange,position->getPercChange(stock->close));
// 			std::cout<<symbol<<" "<<stock->close<<" "<<minChange<<std::endl;
// 		}
// 	}
//     return minChange;
// }
// long double Strategy::getTotalDelta(std::string index)
// {
//     double underlying_close=stocks[index]->close;
//     long double total_delta=0;
//     long double total_positions=0;
//     for(const auto &pair:positions)
//     {
//         std::string symbol=pair.first;
// 		if(split(index,'_')[0]!=split(symbol,'_')[0])
// 			continue;
//         struct Position* position=pair.second;
//         struct Stock* stock=stocks[symbol];
//         if(position->getTotalPosition()!=0)
//         {
//             stock->calcGreeks(underlying_close, stocks[index]->_volatility);
//             total_delta+=stock->delta*position->getTotalPosition();
//             total_positions+=position->getTotalPosition();
//         }
//     }
//     return total_delta/std::abs(total_positions);
// }
// void Strategy::hedgeWithFutures(std::string index)
// {
//     double underlying_close=stocks[index]->close;
//     int lots=stocks[index]->lot;
//     long double total_delta=0;
//     long double total_positions=0;
//     for(const auto &pair:positions)
//     {
//         std::string symbol=pair.first;
// 		if(split(index,'_')[0]!=split(symbol,'_')[0])
// 			continue;
//         struct Position* position=pair.second;
//         struct Stock* stock=stocks[symbol];
//         if(position->getTotalPosition()!=0)
//         {
// 			lots=stock->lot;
//             stock->calcGreeks(underlying_close, stocks[index]->_volatility);
//             total_delta+=stock->delta*position->getTotalPosition();
//         }
//     }
// #ifdef __DEBUGPRINT__
//     std::cout<<"Delta:"<<total_delta<<"|Lots:"<<lots<<std::endl;
//   #endif 
//     int quantity=-int(total_delta/lots)*lots;
//     char side=quantity>0?'B':'S';
//     if(quantity!=0)
//     send(side, std::abs(quantity),100,index,'N',getOrderId(),false);
// }
// void Strategy::exitOptionPosition(std::string index)
// {
//     double underlying_close=stocks[index]->close;
//     int lots=stocks[index]->lot;
//     long double total_delta=0;
//     long double total_positions=0;
//     std::string posDeltaSymbol, negDeltaSymbol;
//     for(const auto &pair:positions)
//     {
//         std::string symbol=pair.first;
// 		if(split(index,'_')[0]!=split(symbol,'_')[0])
// 			continue;
//         struct Position* position=pair.second;
//         struct Stock* stock=stocks[symbol];
//         if(position->getTotalPosition()!=0)
//         {
// 			lots=stock->lot;
//             //stock->calcGreeks(underlying_close);
//             total_delta+=stock->delta*position->getTotalPosition();
//             if(stock->delta*position->getTotalPosition()>0)
//             {
//                 posDeltaSymbol=symbol;
//             }
//             if(stock->delta*position->getTotalPosition()<0)
//             {
//                 negDeltaSymbol=symbol;
//             }
//             std::cout<<symbol<<" "<<stock->delta<<" "<<position->getTotalPosition()<<std::endl;
//         }
//     }
//     std::string tradeSymbol;
//     if(total_delta>0)
//         tradeSymbol=posDeltaSymbol;
//     else
//         tradeSymbol=negDeltaSymbol;
//     int quantity=-positions[tradeSymbol]->getTotalPosition();
//     quantity=(quantity/lots)*lots;
//     char side=quantity>0?'B':'S';
//     if(quantity!=0)
//     {
//         send(side, std::abs(quantity),100,tradeSymbol,'N',getOrderId(),false);
//         _positionExit=true;
// 		stocks[index]->_slHit=true;
//     }
// }
// std::string Strategy::getOtherOption(std::string symbol)
// {
//     for(auto pair:_optionPairs)
//     {
//         std::string symbol1=pair.first;
//         std::string symbol2=pair.second;
//         if(symbol==symbol1)
//         {
//             return symbol2;
//         }
//         else if(symbol==symbol2)
//         {
//             return symbol1;
//         }
//     }
//     return "";
// }

// int Strategy::getSkewness(int callLot,int putLot)
// {
//    // int total=(callLot+putLot)/2;
//    std::cout<<"SKW|CALLLOT:"<<callLot<<"|PUTLOT:"<<putLot<<std::endl;
//     int skew=std::abs(_totalLots/2-callLot)+std::abs(_totalLots/2-putLot);

//     return skew;

// }


// //std::string getOptions(std::string)
// bool Strategy::isStrikeLevelBreached(std::string symbol,std::string aggOrPass)
// {
//     //std::cout<<"INSIDE ISSTRIKE CHANGED"<<std::endl;

//     if(!isOption(symbol))return false;

//     std::string callOrPut=split(symbol,'_')[3];

//     int strikeSymbol=std::stoi(split(symbol,'_')[2]);
//     int close=stocks[split(symbol,'_')[0]+"_F1"]->close;

//     int atm=close/(100*_strikesGapMin)*_strikesGapMin;

//     int maxCallStrike=atm + _maxOTMStrike*_strikesGapMin;

//     int minPutStrike=atm - _maxOTMStrike*_strikesGapMin;

// #ifdef __DEBUGPRINT__
//     std::cout<<"SYMBOL:"<<symbol<<"|CLOSE"<<close<<"|maxCallStrike:"<<maxCallStrike<<"|minPutStrike:"<<minPutStrike<<"|ATM:"<<atm<<std::endl;
//  #endif
//     if(aggOrPass=="PASS")
//     {
//         if(callOrPut=="CE" and strikeSymbol<atm+_strikesGapMin)
//             return true;
//         else if(callOrPut=="PE" and strikeSymbol>atm)
//             return true;
//     }
//     /*else if(aggOrPass=="AGGR")
//     {
//         if(callOrPut=="CE" and strikeSymbol>maxCallStrike)
//             return true;
//         else if(callOrPut=="PE" and strikeSymbol<minPutStrike)
//             return true;
//     }
//     */

//     return false;
   
// }

// std::vector<std::string> Strategy::getStrikeChangeTradingOptions(std::string callOption,std::string putOption)
// {
//     auto vals = getLots(stocks[callOption]->delta, stocks[putOption]->delta,_totalLots);

//     int currSkew=getSkewness(vals[0],vals[1]);
// #ifdef __DEBUGPRINT__
//     std::cout<<"CURR SKW:"<<currSkew<<"|lastSkw:"<<lastSkew<<std::endl;
// #endif
//     if(currSkew<lastSkew and ((lastDistribution.first<=lastDistribution.second and vals[0]<=vals[1]) || (lastDistribution.first>=lastDistribution.second and vals[0]>=vals[1]) ))
//     {
//         return {callOption,putOption};
//     }

//     std::vector<int> lots=vals;

// 	std::string callorPut=lots[0]<lots[1]?callOption:putOption;
// 	int i=lots[0]<lots[1]?0:1;
    
// 	//while(lots[i]*stocks[callorPut]->lot<=(long double)_totalLots*stocks[callorPut]->lot*_optionsChangeVal)
	
//         float val=0.00001f;
// 		if(stocks[callOption]->delta<=val and stocks[putOption]->delta*(-1)<=0.00001f)
// 		{
			
// 			return {callOption,putOption};
// 		    //break;
// 		}
// 	#ifdef __DEBUGPRINT__
// 		else 
// 			std::cout<<"CALL-DELTA:"<<stocks[callOption]->delta<<"|PUT-DELTA:"<<stocks[putOption]->delta*(-1)<<std::endl;
// 	 #endif
// 		if(!i)//i==0 means call
//         	{
// 			#ifdef __DEBUGPRINT__
//             		std::cout<<"OLD CALL OPTION:"<<callOption<<"|";
//            		#endif          
// 		 if(!isStrikeLevelBreached(callOption,"AGGR"))
//                     {
//                              callOption=getNextStrikeCallOrPutOption(callOption,_strikesGap,false);
// 		     #ifdef __DEBUGPRINT__
//             		    std::cout<<"NEW CALL OPTION:"<<callOption<<std::endl;
//                      #endif
// 		    }
// 		   #ifdef __DEBUGPRINT__
//                     else
//                     {
//                         cout<<"CALL LEVEL BREACHED:"<<callOption<<std::endl;
//                     }
//           	   #endif   		
//        		}
//         	else
//        		{
// 			#ifdef __DEBUGPRINT__
//             		std::cout<<"OLD PUT OPTION:"<<putOption<<"|";
//           		 #endif 
// 	      		if(!isStrikeLevelBreached(putOption,"AGGR"))
//                      {
//             		    putOption=getNextStrikeCallOrPutOption(putOption,_strikesGap,false);
//             		    std::cout<<"NEW PUT OPTION:"<<putOption<<std::endl;
//                      }
//                       else
//                     {
//                         cout<<"PUT LEVEL BREACHED:"<<putOption<<std::endl;
//                     }
//         	}
//         // if(stocks.find(callOption)==stocks.end() || stocks.find(putOption)==stocks.end())
//         // {
//         //     std::cout<<"NOT REBALNCNG| SYM NOT FOUND";
//         //     return {"",""};
//         // }
//         return {callOption,putOption};

// 	//	stocks[callOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
// 		//stocks[putOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
//         //        lots = getLots(stocks[callOption]->delta, stocks[putOption]->delta,_totalLots);
		


// }
// void Strategy::hedgeWithOptionsRebalance(std::string index)
// {
//     double underlying_close=stocks[index]->close;
//     std::string callOption,putOption;
//     for(const auto &pair:positions)
//     {
//         std::string symbol=pair.first;
// 		if(split(index,'_')[0]!=split(symbol,'_')[0])
// 			continue;
//         struct Position* position=pair.second;
//         struct Stock* stock=stocks[symbol];
//         if(position->getTotalPosition()!=0)
//         {
//             if(stock->callput=="CE")
//                 callOption=symbol;
//             else if(stock->callput=="PE")
//                 putOption=symbol;
//             //stock->calcGreeks(underlying_close, stocks[index]->_volatility);
//         }
//     }
// 	if((callOption=="") && (putOption==""))
// 	{
// 		return;
// 	}
// 	if(callOption=="")
// 	{
// 		callOption=getOtherOption(putOption);
// 	}
// 	if(putOption=="")
// 	{
// 		putOption=getOtherOption(callOption);
// 	}
// 	stocks[callOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
// 	stocks[putOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
// 	auto vals = getLots(stocks[callOption]->delta, stocks[putOption]->delta,_totalLots);
//     std::cout<<"LOTS:"<<callOption<<"|"<<putOption<<"|"<<vals[0]<<"|"<<vals[1]<<"|"<<stocks[callOption]->delta<<"|"<<stocks[putOption]->delta<<std::endl;
// 	std::map<std::string,int> desiredQuantities;
//     desiredQuantities[callOption]=-vals[0]*stocks[callOption]->lot;
//     desiredQuantities[putOption]=-vals[1]*stocks[putOption]->lot;
// #ifdef lotsChange
//     if((std::abs(desiredQuantities[callOption])<=(long double)_totalLots*stocks[callOption]->lot*_optionsChangeVal) || (std::abs(desiredQuantities[putOption])<=(long double)_totalLots*stocks[callOption]->lot*_optionsChangeVal))
// #endif
// #ifndef lotsChange
//     if((desiredQuantities[callOption]==0) || (desiredQuantities[putOption]==0))
// #endif
//     {
// 	#ifdef __DEBUGPRINT__

//         std::cout<<"DESIRED CALL:"<<desiredQuantities[callOption]<<"|total:"<<_totalLots<<"|optionsChangeVal:"<<_optionsChangeVal<<"|currVal:"<<(long double)_totalLots*stocks[callOption]->lot*_optionsChangeVal<<std::endl;
//         std::cout<<"DESIRED PUT:"<<desiredQuantities[putOption]<<"|total:"<<_totalLots<<"|optionsChangeVal:"<<_optionsChangeVal<<"currVal:"<<(long double)_totalLots*stocks[callOption]->lot*_optionsChangeVal<<std::endl;
// 	 #endif
//         desiredQuantities[callOption]=0;
//         desiredQuantities[putOption]=0;
//         auto newOptions=getStrikeChangeTradingOptions(callOption,putOption);

//         if(stocks.find(newOptions[0])==stocks.end() || stocks.find(newOptions[0])==stocks.end())
//         {
// 	#ifdef __DEBUGPRINT__
//             std::cout<<"NOT REBALNCNG| SYM NOT FOUND";
//           #endif
// 	    return;
//         }
//         callOption=newOptions[0];
//         putOption=newOptions[1];
//         stocks[callOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
// 		stocks[putOption]->calcGreeks(underlying_close, stocks[index]->_volatility);
        
//         auto lots = getLots(stocks[callOption]->delta, stocks[putOption]->delta,_totalLots);
//         lastSkew=getSkewness(lots[0],lots[1]);
//         lastDistribution={lots[0],lots[1]};
// 	#ifdef __DEBUGPRINT__
//          std::cout<<"LOTS StrikeChange:"<<callOption<<"|"<<putOption<<"|"<<lots[0]<<"|"<<lots[1]<<"|"<<stocks[callOption]->delta<<"|"<<stocks[putOption]->delta<<std::endl;
// 	 #endif
//         desiredQuantities[callOption]=-lots[0]*stocks[callOption]->lot;
//         desiredQuantities[putOption]=-lots[1]*stocks[putOption]->lot;
// 		_optionPairs[callOption]=putOption;
//     }
// 	takeTrades(desiredQuantities);
// }

void Strategy::takeTrades(std::map<std::string,int> desiredQuantities)
{
    for(auto pair:desiredQuantities)
    {
        std::string tradeSymbol=pair.first;
        int desiredQuantity=pair.second;
        int quantity= desiredQuantity - positions[tradeSymbol]->getTotalPosition();
        char side=quantity>0?'B':'S';
        if(quantity!=0)
            send(side, std::abs(quantity),100,tradeSymbol,'N',getOrderId(),false);
    }
}

void Strategy::squareOffPositions(std::string index)
{
    stocks[index]->_squareOffDone=true;		
    for(const auto &pair:positions)
    {
        std::string symbol=pair.first;
		if(split(index,'_')[0]!=split(symbol,'_')[0])
			continue;
        struct Position* position=pair.second;
        int quantity=position->getTotalPosition();
        char side=quantity>0?'S':'B';
        quantity=std::abs(quantity);
        if(quantity!=0)
        {
            send(side,quantity,100,symbol,'N',OrderIDStartingNumber++,false);
        }
    }
}
// std::string Strategy::getNextStrikeCallOrPutOption(std::string symbol,int strikeGap,bool moveInwards)
// {
    
    
//     auto symSplit=split(symbol,'_');
//     int multiplier_towards=moveInwards?-1:1;
//     int multiplier=symSplit[3]=="CE"?1*multiplier_towards:-1*multiplier_towards;


//     int nextStrikePrice=std::stoi(symSplit[2])+multiplier*strikeGap;
//     //std::cout<<"DEBUG IN GET NEXT STRIKE:"<<symbol<<"|MOVE IN:"<<moveInwards<<"|nextStrike:"<<nextStrikePrice<<std::endl;
//     std::string nextStrike=symSplit[0]+'_'+symSplit[1]+'_'+std::to_string(nextStrikePrice)+'_'+symSplit[3];

//     return nextStrike;

// }
// std::string Strategy::getNextStrikeCallOrPutOption(std::string symbol,int strikeGap)
// {
    

//     auto symSplit=split(symbol,'_');

//     int multiplier=symSplit[3]=="CE"?1:-1;

//     int nextStrikePrice=std::stoi(symSplit[2])+multiplier*strikeGap;

//     std::string nextStrike=symSplit[0]+'_'+symSplit[1]+'_'+std::to_string(nextStrikePrice)+'_'+symSplit[3];

//     return nextStrike;

// }
std::string Strategy::getSymbolAtStrike(std::string symbol,int strikeLevel,std::string callorPut)
{
    int close=stocks[symbol]->close;
    std::string price;
    std::string refSymbol = split(symbol,'_')[0];

    strikeLevel=split(symbol,'_')[3]=="CE"?strikeLevel:-(strikeLevel-1);
    price = std::to_string(((int(close/10000))*100)+strikeLevel*100);
   // putPrice = std::to_string(((int(close/10000))*100)-(strikeLevel-1)*100); 

    std::string finalSymbol= refSymbol+"_"+weeklyExpiry+"_"+price+"_"+callorPut;
    // std::string putOption= refSymbol+"_"+weeklyExpiry+"_"+putPrice+"_PE";
    return finalSymbol;
}


// std::string Strategy::getOptionAtPrice(std::string symbol,std::string callOrPut,double optionPremium)
// {
// 	int close=stocks[symbol]->close;

// 	int atmStrike=close/(100*_strikesGap)*_strikesGap; //strike in rupees

// 	//int multiplier=callOrPut=="CE" ?1:-1;

// 	std::string underlying=split(symbol,'_')[0];

// 	int strikesGap=_strikesGap; //get options at strike gap of strikesGap only not strikeGapMin

// 	std::vector<std::string> options; //contains list of all options with premium near to give premium



// 	for(int i=0;i<20;i++)   //assumption that our required strike won't be far away from 10th strike
// 	{

// 		int otmLevel=callOrPut=="CE" ? i+1: -1*i;
// 		int currLevelStrike=atmStrike+otmLevel*_strikesGap;

// 		std::string currOption=underlying+"_"+weeklyExpiry+"_"+std::to_string(currLevelStrike)+"_"+callOrPut;

// 		if(finalOptions.find(currOption)!=finalOptions.end() and stocks.find(currOption)!=stocks.end())
// 		{
// 			options.push_back(currOption);
// 		}

// 	}

// 	double closest=999999999;

// 	std::string closestPriceOption;

// 	for(auto option: options)
// 	{
// 		int optionLtp=stocks[option]->close;
// 		//std::cout<<"DEBUG "<<option<<" close:"<<optionLtp<<std::endl;

// 		int diff=std::abs(optionLtp-optionPremium);
// 		if(diff<closest)
// 		{
// 			closestPriceOption=option;
// 			closest=diff;
// 		}


// 	}
	
// #ifdef __DEBUGPRINT__
// 	std::cout<<"closestPriceOption:"<<closestPriceOption<<"|price:"<<stocks[closestPriceOption]->close<<std::endl;
// #endif
// 	return closestPriceOption;

	


// }

// vector<std::string> Strategy::getTradingOptionsAtVol(std::string symbol)
// {
// 	double vol=stocks[symbol]->_volatility;
// 	int underlyingClose=stocks[symbol]->close;
// 	double timeToExpInSec=time_diff(getTrimmedDate(stocks[symbol]->date),weeklyExpiry,stocks[symbol]->time)*(365*24*60*60);
// 	double multiplier=getTrimmedDate(stocks[symbol]->date)==weeklyExpiry ? sqrt(timeToExpInSec/(3600.0*24.0)):1;
// 	#ifdef __DEBUGPRINT__
	
// 	printf("VolMultiplier| Date:%s | expiryDate: %s | time %lld | initialVol: %f | multiplier:%f | timeToExpInSec %f  \n",stocks[symbol]->date.c_str(),weeklyExpiry.c_str(),stocks[symbol]->time,vol,multiplier,timeToExpInSec);	
// 	 #endif
// 	vol*=multiplier;
// 	double distanceOfStrikes= vol*_strikesAggresiveness/16.0*(underlyingClose*1.0f);
// 	#ifdef __DEBUGPRINT__
// 	printf("Dist OF Strikes:%f Vol:%f close:%d strikesAggressiveness:%f \n",distanceOfStrikes,vol,underlyingClose,_strikesAggresiveness);
// 	 #endif
// 	int callStrike = underlyingClose + distanceOfStrikes;

// 	int putStrike = underlyingClose - distanceOfStrikes;
	
// 	int strikesGap=_strikesGap;
// 	int finalCallStrike = callStrike/(strikesGap*100)*(strikesGap*100);
// 	int finalPutStrike = putStrike/(strikesGap*100)*(strikesGap*100);


//     	double percDiffCall=(std::abs(callStrike-finalCallStrike))*1.0f/(_strikesGap*100.0);
//     	double percDiffPut=(std::abs(putStrike-finalPutStrike))*1.0f/(_strikesGap*100.0);

// 	if(percDiffCall>_percStrikeChange)
// 		finalCallStrike+=strikesGap*100;

// 	if(percDiffPut > _percStrikeChange)
// 		finalPutStrike+=strikesGap*100;
// 	#ifdef __DEBUGPRINT__
// 	printf("CALL STRIKE: %d PUT STRIKE %d \n",finalCallStrike,finalPutStrike);
//  	#endif
// 	finalPutStrike/=100;
// 	finalCallStrike/=100;

// 	std::string underlying=split(symbol,'_')[0];
// 	std::string callOption=underlying+"_"+weeklyExpiry+"_"+std::to_string(finalCallStrike)+"_CE";
// 	std::string putOption=underlying+"_"+weeklyExpiry+"_"+std::to_string(finalPutStrike)+"_PE";
// 	#ifdef __DEBUGPRINT__
// 		printf("call options: %s put options: %s \n",callOption.c_str(),putOption.c_str());
// 	 #endif
// 	return {callOption,putOption};
// }
// vector<std::string> Strategy::getTradingOptions(std::string symbol)
// {
//     int close=stocks[symbol]->close;
//     std::string callPrice;
//     std::string putPrice;
//     double minPremium=_minPremium*100; //user gives premium in rupees
//     double maxPremium=_maxPremium*100;
//     if(stocks[symbol]->_volReady and stocks[symbol]->_tradesTaken==false and tradeOnPrice==false)
// 	  {
		
// 		 auto values=getTradingOptionsAtVol(symbol);

// 		 if(values.size()>0 and finalOptions.find(values[0])!=finalOptions.end() and isOption(values[0]))
// 		 {
// 			 if(stocks[values[0]]->close <minPremium)
// 				{
// 				#ifdef __DEBUGPRINT__
// 					printf("Options current premium less than specified: current %f specified: %f \n",double(stocks[values[0]]->close)/100.0,minPremium);
// 				 #endif
// 					values[0]=getOptionAtPrice(symbol,split(values[0],'_')[3],minPremium);
// 				}
// 			 if(stocks[values[0]]->close >maxPremium)
// 				{
// 				#ifdef __DEBUGPRINT__
// 					printf("Options current premium more than specified: current %f specified: %f \n",double(stocks[values[0]]->close)/100.0,maxPremium);
// 				 #endif
// 					values[0]=getOptionAtPrice(symbol,split(values[0],'_')[3],maxPremium);
// 				}

// 		 }
// 		 if(values.size()>1 and finalOptions.find(values[1])!=finalOptions.end() and isOption(values[1]))
// 		 {
// 			 if(stocks[values[1]]->close <minPremium)
// 			 {
// 				#ifdef __DEBUGPRINT__
// 				 printf("Options current premium less than specified: current %f specified: %f \n",double(stocks[values[1]]->close)/100.0,minPremium);
// 				 #endif
// 				 values[1]=getOptionAtPrice(symbol,split(values[1],'_')[3],minPremium);
// 			 }
// 			 if(stocks[values[1]]->close >maxPremium)
// 				{
// 				#ifdef __DEBUGPRINT__
// 					printf("Options current premium more than specified: current %f specified: %f \n",double(stocks[values[1]]->close)/100.0,maxPremium);
// 				 #endif
// 					values[1]=getOptionAtPrice(symbol,split(values[1],'_')[3],maxPremium);
// 				}
// 		 }

// 		 return values;
// 	  }
//     //int strikeLevel=_strikeLevel;
//     int strikePrice;
//     std::string refSymbol=split(symbol,'_')[0];



//     std::string callOption=getOptionAtPrice(symbol,"CE",minPremium);
//     std::string putOption=getOptionAtPrice(symbol,"PE",minPremium);
// /*
//     int atm=int(close/(100*_strikesGap))*_strikesGap*100;

//     double percDiff=(std::abs(close-atm))*1.0f/(_strikesGap*100.0);


//     int strikeCall=strikeLevel,strikePut=strikeLevel;
//     std::cout<<"PERC_DIFF:"<<percDiff<<std::endl;
//     if(percDiff<=_percStrikeChange)
//     {
// 	    strikePut+=1;
// 	    std::cout<<"DEC PUT"<<std::endl;
//     }
//     else if((1-percDiff)<=_percStrikeChange)
//     {
// 	    strikeCall+=1;
// 	    std::cout<<"INC CALL"<<std::endl;

//     }

// #ifdef __SIMMODE__
//     callPrice = std::to_string(((int(close/(100*_strikesGap)))*_strikesGap)+strikeCall*_strikesGap);
//     putPrice =  std::to_string(((int(close/(100*_strikesGap)))2_strikesGap)-(strikePut-1)*_strikesGap);
// #ifdef percOTM
// 	strikePrice = (close)*(1.0f+(long double)_optionPerc/100);
// 	strikePrice = round((float)strikePrice/10000.0f)*100;
// 	callPrice=std::to_string(strikePrice);
// 	strikePrice = (close)*(1.0f-(long double)_optionPerc/100);
// 	strikePrice = round((float)strikePrice/10000.0f)*100;
// 	putPrice=std::to_string(strikePrice);
// #endif
// #endif
// #ifdef __LIVEMODE__
//     callPrice = std::to_string((((int(close/10000))*100)+strikeLevel*100)*100);
//     putPrice = std::to_string((((int(close/10000))*100)-(strikeLevel-1)*100)*100);
// #endif
//     std::string callOption= refSymbol+"_"+weeklyExpiry+"_"+callPrice+"_CE";
//     std::string putOption= refSymbol+"_"+weeklyExpiry+"_"+putPrice+"_PE";
// 	//std::cout<<"Choosing Options"<<callOption<<" "<<putOption<<std::endl;
// 	*/
//     if((finalOptions.find(callOption)!=finalOptions.end()) && 
//             (finalOptions.find(putOption)!=finalOptions.end()))
//     {
//         return {callOption,putOption,callPrice,putPrice};
//     }
//     else
// 	{
// 		std::cout<<"NoBarsReceivedFo"<<callOption<<":"<<putOption<<std::endl;
// 		return {};
// 	}
// }

void Strategy::checkSL(std::string symbol)
{
}

//void Strategy::onResponse(char responseType, long long int orderID, std::string symbol, char side, int quantity,int price, int16_t errorCode, char orderType, int openQuantity)
void Strategy::onResponse(char responseType, int16_t errorCode, long long int orderID, std::string symbol, char side, int openQuantity,int fillQuantity,int price, char orderType)
//void Strategy::onResponse(char responseType, long long int orderID, std::string symbol, char side, int quantity,int price)
{
    positions[symbol]->onOrderResponse(side,fillQuantity,price,responseType);
  #ifdef __DEBUGPRINT__
    std::cout<<"Got reponse "<<curTime<<" "<<responseType<<" " <<orderID<<" "<<symbol<<" "<<side<<" "<<fillQuantity<<" "<<price<<std::endl;
   #endif
    //std::cout<<"Got reponse "<<curTime<<" "<<responseType<<" " <<orderID<<" "<<symbol<<" "<<side<<" "<<fillQuantity<<" "<<openQuantity<<" "<<price<<std::endl;
    //std::cout<<"<<curTime<<" resType:"<<responseType<<" OID:" <<orderID<<" "<<symbol<<" side:"<<side<<" fillQty:"<<fillQuantity<<" openQty:"<<openQuantity<<" price:"<<price<<std::endl;

}
void Strategy::setStrategyConnector(StrategyConnector *stc)
{
    connector = stc;
}

void Strategy::send(char side, int qty, int px, string symbol, char orderType,long long oid,bool matchRemainingInNeXtBarOverRide,int fillQuantity,int disclosedQuantity,int ioc)
//void Strategy::send(char side, int qty, int px, string symbol, char orderType,long long orderID, bool matchRemainingInNeXtBarOverRide)
{
    positions[symbol]->addOrder(side, std::abs(qty));
	#ifdef __DEBUGPRINT__
    std::cout<<"Request|"<<curTime<<" "<<qty<<" "<<px<<" "<<side<<" "<<symbol<<" "<<orderType<<std::endl;
	 #endif	
    //int fillQuantity=0;
	//int iocFlag=0;
	connector->send(side,qty,px,symbol,orderType,oid,matchRemainingInNeXtBarOverRide,fillQuantity,disclosedQuantity,ioc);
    //connector->sendOrder(side,qty,px,symbol,orderType,orderID,matchRemainingInNeXtBarOverRide);
}
Strategy::Strategy(std::string strategyConfigFilePath)
{
    
    readStrategyConfig(strategyConfigFilePath);
    _configPath=strategyConfigFilePath;
    putSymbolinStockMap();
    _spreadCalc=std::make_shared<SpreadCalc>(this);
  // _priceChase=std::make_shared<PriceChase>(this); 
    _quadDist_strat=std::make_shared<QuadDist>(this); 

}
void Strategy::putSymbolinStockMap()
{
    for(auto it=validSymbols.begin();it!=validSymbols.end();it++)
    {
        putSymbolinStockMap(*it);
    }
}
void Strategy::putSymbolinStockMap(std::string symbol)
{
    struct Stock *stock = new struct Stock();
    stocks[symbol] = stock;
    struct Position *position = new struct Position(symbol,stock);
    positions[symbol] = position;
}
void Strategy::subscribeToOptions(std::string symbol,int close)
{
    //std::cout<<"symbol:"<<symbol<<"|close:"<<close<<std::endl;
    std::string refSymbol = split(symbol,'_')[0];
    for(int i=-40;i<=40;i++)
    {
        int strikeLevel=i;
        std::string price;
#ifdef __SIMMODE__
        price = std::to_string(((int(close/(_strikesGapMin*100)))*_strikesGapMin)+strikeLevel*_strikesGapMin);
#endif
#ifdef __LIVEMODE__
        price = std::to_string((((int(close/10000))*100)+strikeLevel*100)*100);
#endif
        for (std::string moneyness:{"CE","PE"})
        {
            std::string tempOption= refSymbol+"_"+weeklyExpiry+"_"+price+"_"+moneyness;
			if(tempOptions.find(tempOption)==tempOptions.end())
			{
			#ifdef __DEBUGPRINT__
                		std::cout<<"SUBSCRIBING SYMBOOL:"<<tempOption<<std::endl;
			#endif
				tempOptions.insert(tempOption);
				connector->subscribeToSymbol(tempOption);
			}
        }
    }
    if(tempOptions.find(_spotSymbol)==tempOptions.end())
    {
	    tempOptions.insert(_spotSymbol);
            connector->subscribeToSymbol(_spotSymbol);
    }
}

void Strategy::subscribeToOption(std::string symbol)
{
	if(tempOptions.find(symbol)==tempOptions.end())
		connector->subscribeToSymbol(symbol);
}
void Strategy::getWeeklyExpiry()
{
    std::string finalDate = getFormattedDate();
    std::string fileName=_weeklyExpiryFile;
    std::ifstream file(fileName);
    std::string line;
    if(!file.is_open())
    {
        //std::cout<<"Weekly expiry file not found"<<std::endl;
        return;
    }
    while(!file.eof())
    {
        std::getline(file,line);
        auto expiries = split(line,',');
        for(auto expiry:expiries)
        {
            if(ModeWanted==1)
            {
                if(std::stoll(expiry)>=std::stoll(finalDate))
                {
	#ifdef __DEBUGPRINT__
                    std::cout<<"FoundWeeklyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
           #endif     
	       	    weeklyExpiry=expiry;
                    break;
                }
            }
            if(ModeWanted==2)
            {
                if((std::stoll(expiry)/86400)>=(std::stoll(finalDate)))
                {
		#ifdef __DEBUGPRINT__
                    std::cout<<"FoundWeeklyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
           	#endif 
		    weeklyExpiry=expiry;
                    break;
                }
            }
        }
    }
    file.close();
    return;
}
void Strategy::getMonthlyExpiry()
{
    std::string finalDate = getFormattedDate();
    std::string fileName=_monthlyExpiryFile;
    std::ifstream file(fileName);
    std::string line;
    if(!file.is_open())
    {
        //std::cout<<"Weekly expiry file not found"<<std::endl;
        return;
    }
    while(!file.eof())
    {
        std::getline(file,line);
        auto expiries = split(line,',');
        for(auto expiry:expiries)
        {
            if(ModeWanted==1)
            {
                if(std::stoll(expiry)>=std::stoll(finalDate))
                {
		#ifdef __DEBUGPRINT__
                    std::cout<<"FoundMonthlyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
                  #endif 
		    monthlyExpiry=expiry;
                    break;
                }
            }
            if(ModeWanted==2)
            {
                if((std::stoll(expiry)/86400)>=(std::stoll(finalDate)/86400))
                {	
		#ifdef __DEBUGPRINT__
                    std::cout<<"FoundMonthlyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
                   #endif  
		    monthlyExpiry=expiry;
                    break;
                }
            }
        }
    }
    file.close();
    return;
}
void Strategy::readStrategyConfig(std::string strategyConfigFilePath)
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

        if (tempString == "subscriptionTime")
            _subscriptionTime = stoll(tempString2);

        else if(tempString == "totalLots")
            _totalLots= stoll(tempString2);
        else if(tempString == "totalQuantity")
            _totalQuantity= stoll(tempString2);
        else if(tempString == "weeklyExpiryFile")
            _weeklyExpiryFile= tempString2;
        else if(tempString == "monthlyExpiryFile")
            _monthlyExpiryFile= tempString2;
       
		else if(tempString=="useSpot")
		{
			if(tempString2=="TRUE")
			{
				_useSpot=true;

			}
		}
	else if(tempString=="strikesGap")
	{
		_strikesGap=std::stoi(tempString2);
	
	}
	else if(tempString=="strikesGapMin")
	{
		_strikesGapMin=std::stoi(tempString2);
	
	}
	else if(tempString=="percStrikeChange")
	{
		_percStrikeChange=std::stold(tempString2);
	}
	else if(tempString=="strikeLevel")
	{
		_strikeLevel=std::stoi(tempString2);
	}
    
	
	
    
        
       
         else if(tempString == "trailingSLThreshold")
                _trailingSLThreshold= stold(tempString2);
            else if(tempString == "trailingSLPerc")
                _trailingSLPerc = stold(tempString2);
		else if(tempString == "sl")
			sl = stoll(tempString2);
		else if(tempString == "tp")
			tp = stoll(tempString2);
        else if(tempString == "reEntryCoolOff")
            _reEntryCoolOff= std::stoi(tempString2);
		else if(tempString == "optionPerc")
		{
			_optionPerc = std::stof(tempString2);
		}
	else if(tempString=="useSynthetic")
	{
		_useSynthetic=tempString2=="TRUE"?true:false;
	}
	else if(tempString == "spotSymbol")
	{
		_spotSymbol=tempString2;
	}
    }
    _exitLegThreshold+=_aggressivePriceChange; //exit thresholdShouldAlwaysBe gretaer than aggressivePriceChange
}
std::string Strategy::getFormattedDate()
{
    return getTrimmedDate(runDate);
}
long long Strategy::getPNLForUnderlying(std::string index)
{
    long long int totalPNL=0;
    for(auto pair:positions)
    {
        std::string symbol=pair.first;
        if(symbol.size()>0)
        {
            if(split(symbol,'_')[0]==split(index,'_')[0])
            {
                long long pnl = getIndividualPNL(symbol);
                totalPNL+=pnl;
            }
        }
    }
    return totalPNL;
}
long long Strategy::getIndividualPNL(std::string symbol)
{
    long long pnl = positions[symbol]->totalSellValue-positions[symbol]->totalBuyValue+(long long)positions[symbol]->currentPosition*(long long)stocks[symbol]->close;
    if(pnl!=0)
    {
        /*
    if(printData)
        std::cout<<"IndividualPNL:"<<symbol<<":"<<pnl<<":"<<positions[symbol]->totalSellValue<<":"<<positions[symbol]->totalBuyValue<<":"<<positions[symbol]->currentPosition<<":"<<stocks[symbol]->close<<std::endl;
        */
    }
    return pnl;
}
long long int Strategy::getOrderId()
{
    return connector->getOrderId();
}

void Strategy::processTopOfBook(std::string symbol, BookLevel booklvl[10])
{
    cout<<"Top Of The Book\n";
    for(int i=0; i<10; i++)
    {
        cout<<i+1<<" "<<booklvl[i]._price<<" "<<booklvl[i]._quantity<<"\n";
    }
}



void Strategy::buildPosition(std::map<std::string ,int > desiredQty)
{
	takeTrades(desiredQty);
}
void Strategy::buildPosition(std::string symbol,int qty)
{
	std::map<std::string,int> desiredQuantities;
	desiredQuantities[symbol]=qty;
	takeTrades(desiredQuantities);
}
