#include "/home/infra/InternalSimulatorUsers/Backtester/strategyConnector.h"
#include "strategy.h"

/*** validSymbols is a set which contains symbols passed in config  ***/
/*** OrderIDStartingNumber contains a positive number from where orderID numbers should start ***/
int stopLoss,volumeEnter,gapPercLong,gapPercShort;

Strategy* strategyObj;// = new strategyClass();
StrategyConnector::StrategyConnector(orderManager *tempOb,orderBook* tempObBook,dataManager *dmObject,string filepath)
{
	strategyObj=new Strategy(filepath);
	orderObj = tempOb;	// OM object pointer
	bookObj = tempObBook;	// OB object pointer
	dmObj = dmObject;	// DM object pointer
	strategyObj -> setStrategyConnector(this);
}

void StrategyConnector::readStrategyConfig(string path)
{
	ifstream myFile(path);
	if(!myFile.is_open())
    {
    	cout << "error opening strat config data file, path given :" << path << '\n';
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
        if(tempString == "StopLoss")
        	stopLoss = stoi(tempString2);
        else if(tempString == "VolumeEnter")
        	volumeEnter = stoi(tempString2);
        else if(tempString == "GapPercLong")
        	gapPercLong = stoi(tempString2);
        else if(tempString == "GapPercShort")
        	gapPercShort = stoi(tempString2);
	}
	cout << "Reading strategyConfig: " << stopLoss << " " << volumeEnter << " " << gapPercLong << " " << gapPercShort << "\n";
}

void StrategyConnector::send(char side, int qty, int px, string symbol, char orderType,long long oid,bool matchRemainingInNeXtBarOverRide,int fillQuantity,int disclosedQuantity,int ioc)
{
	OrderSt Order;                                                  // Creating Order and putting values in it
	Order.orderID = oid;
	Order.symbol = symbol;
	Order.side = side;
	Order.quantity = qty;
	Order.price = px;
	Order.orderType = orderType;
	Order.matchRemainingInNeXtBarOverRide = matchRemainingInNeXtBarOverRide;
	Order.stpc = 0;
	Order.iocflag = ioc;
	Order.openquantity=qty;
	Order.fillquantity=fillQuantity;

	#ifdef debugStrat
	cout << "Sending Ord from Strat: " << Order.symbol << " " << Order.orderID << " " << Order.side << " " << Order.quantity << " " << Order.price << " " << Order.orderType << " "  << ' ' << endl;
	#endif

	orderObj->recOrder(Order,symbol);	// sending order to OM
}

bool StrategyConnector::onMarketData(DataSt currentData, string smbl) 
{
	/*#ifdef debugStrat
	if(DataFreq=="TBT")
		cout << "\nStrategyConnector.cpp: " << smbl << " " << currentData.messageType << " " << currentData.time << " " << currentData.orderID << " " << currentData.side << " " << currentData.price << " " << currentData.quantity << " exp"<<currentData.expiry << " strike"<< currentData.strike << " l" << currentData.lot <<"\n";
	else if(DataFreq=="MINUTELY")
		cout << "\nStrategyConnector.cpp:" << smbl << " " << currentData.date <<" " << currentData.time << " " << currentData.open << " " << currentData.high << " " << currentData.low << " " << currentData.close << " " <<currentData.volume << " " << currentData.OI << " " << currentData.lot << " " << currentData.expiry << " " << currentData.callput;
	#endif*/

	if(DataFreq=="MINUTELY")
	{
		strategyObj->updateData(smbl,currentData.date,currentData.time,currentData.open,currentData.high, currentData.low,currentData.close,currentData.volume,currentData.OI,currentData.lot,currentData.expiry,currentData.callput);
	strategyObj->checkSignal(smbl);
	strategyObj->checkSL(smbl);
	}
	else if(DataFreq=="TBT")
		strategyObj->updateData(smbl,currentData.messageType,currentData.time,currentData.side, currentData.price,currentData.quantity,currentData.expiry,currentData.strike,currentData.lot,currentData.callput,currentData.booklvl, currentData.orderID, currentData.orderID2);
	#ifdef SimSnapshot
	else if(DataFreq=="SNAPSHOT")
        strategyObj->updateData(smbl,currentData.open,currentData.high,currentData.low,currentData.close,currentData.volumeSnapshot,currentData.LTP,currentData.LTQ,currentData.LTT,currentData.ATP,currentData.lot,currentData.expiry,currentData.bidPriceArray,currentData.bidQuantityArray,currentData.askPriceArray,currentData.askQuantityArray,/*currentData.buyDepth,currentData.sellDepth,*/currentData.time);
	#endif
	return false;
}

void StrategyConnector::currMinuteBarsFinished()
{
	strategyObj->currMinuteBarsFinished();
}

// MINUTELY & SNAPSHOT
void StrategyConnector::onOrderResponse(char responseType,int16_t errorCode,long long int orderID,std::string symbol, char side, int quantity, int price,int openquantity)		// response received from  OM
{
	#ifdef debugStrat
	cout << "StrategyConnector Response : " << responseType << " " << errorCode << " " << orderID << " " << symbol << " " << 
	side << " " << quantity << " " << price <<" "<<openquantity << endl;
	#endif

	if(responseType == 'T' || responseType == 'P') strategyObj->onResponse(responseType,errorCode,orderID,symbol,side,openquantity,quantity,price, responseType);
	else			strategyObj->onResponse(responseType,errorCode, orderID,symbol,side,quantity,openquantity,price, responseType);
}

// TBT
void StrategyConnector::onOrderResponse(char responseType,int16_t errorCode,char orderType,long long int orderID,std::string symbol, char side, int openQuantity,int fillQuantity, int price)
{
	#ifdef debugStrat
	cout << "Response TBT: " << responseType << " " << errorCode << " " << orderType << " " << orderID << " " <<symbol << " " << 
	side << " " << openQuantity << " " << fillQuantity << " " << price << endl;
	#endif

	strategyObj->onResponse(responseType,errorCode, orderID,symbol,side,openQuantity,fillQuantity,price, orderType);
}

orderBook* StrategyConnector::getOrderBookObject()
{
	return bookObj; 
}

void StrategyConnector::subscribeToSymbol(string symbol)
{
	dmObj->subscribeToSymbol(symbol);
}

void StrategyConnector::onSignal1() {strategyObj->onSignal1();}
void StrategyConnector::onSignal2() {strategyObj->onSignal2();}
void StrategyConnector::onSignal3() {strategyObj->onSignal3();}
void StrategyConnector::onSignal4() {strategyObj->onSignal4();}
void StrategyConnector::TopOfBook(std::string symbol,BookLevel *booklvl) {strategyObj->TopOfBook(symbol,booklvl);}
void StrategyConnector::getTopFromBook(std::string symbol,int numOfLevels,std::vector<pair<long long,long long>> &topBuy,std::vector<pair<long long,long long>> &topSell)
{
        bookObj->getTopFromBook(symbol,numOfLevels,topBuy,topSell);
}
void StrategyConnector::timerEvent() {strategyObj->timerEvent();}

// By Som
void StrategyConnector::getTopOfBook(std::string symbol)
{
    BookLevel bookLevels[10];

    for(int i=5; i<10; i++)
        bookLevels[i]._buySellFlag = 1;

    strategyObj->processTopOfBook(symbol, bookLevels);
}

long long int StrategyConnector::getOrderId()
{
    return OrderIDStartingNumber++;
}

long long int StrategyConnector::getOriginalOrderId(long long int orderId)
{
    return orderId;
}

long long int StrategyConnector::getLatestOrderId(long long int orderId)
{
    return orderId;
}
void StrategyConnector::send(TCP_ORDER_MULTILEG Order) {
     orderObj->recOrder(Order);
 }

 void StrategyConnector::onOrderResponse(TCP_ORDER_MULTILEG order) {
     strategyObj->onResponse(order);
 }
