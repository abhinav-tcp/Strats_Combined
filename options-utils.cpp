#include "strategy.h"
std::string getOption(std::string symbol, int close,int strikeLevel, std::string finalExpiry, std::string callPut,std::string moneyness)
{
    std::string refSymbol = split(symbol,'_')[0];
    //std::string callPrice;
    //std::string putPrice;
    int callPrice;
    int putPrice;
    int finalPrice;
    if(moneyness=="OTM")
    {
        callPrice=int(close/10000)*100+strikeLevel*100;
        putPrice=int(close/10000)*100-(strikeLevel-1)*100;
    }
    else if(moneyness=="ITM")
    {
        callPrice=int(close/10000)*100-(strikeLevel-1)*100;
        putPrice=int(close/10000)*100+strikeLevel*100;
    }
    if(callPut=="CE")
        finalPrice=callPrice;
    else if(callPut=="PE")
        finalPrice=putPrice;
    else
        std::cout<<"Invalid value of callput passed"<<std::endl;
    std::string finalSymbol=refSymbol+"_"+finalExpiry+"_"+std::to_string(finalPrice)+"_"+callPut;
    return finalSymbol;
}
int minutesSinceMidnight(long long int time)
{
    int total_time = (int(time/10000))*60 + ((time/100)%100);
    return total_time;
}
vector<std::string> split(std::string str, char delimiter)
{
    vector<std::string> internal;
    stringstream ss(str); // Turn the string into a stream.
    std::string tok;
    while(getline(ss, tok, delimiter))
    {
        internal.push_back(tok);
    }
    return internal;
}
std::string getRefIndex(std::string option, std::string monthlyExpiry)
{
    if(ModeWanted==1)
        return split(option,'_')[0]+"_F1";
    else
        return split(option,'_')[0]+"_"+monthlyExpiry+"_FUT";
}

long double getPercDiff(long double avgTradePrice, int close)
{
    long double diff = ((long double)close - (long double)avgTradePrice) / std::abs((long double)avgTradePrice);
    return diff*100;
}

long double getPercDiff(long long maxPNL, long long currentPNL)
{
    long double diff = ((long double)currentPNL - (long double)maxPNL)/std::abs((long double)maxPNL);
    return diff*100;
}

bool isFutSymbol(std::string symbol)
{
        auto data=split(symbol,'_');
        if(data.size()==2 and ( data[1]=="F1" or data[1]=="F2" or data[1]=="F3"))
                return true;

        if(data.size()==3 and data[2]=="FUT")
                return true;
        return false;
}
bool isSpotSymbol(std::string symbol)
{
        auto data=split(symbol,'_');
        if(data.size()==2 and data[1]=="SPTIDX")
                return true;
        return false;
}
bool isOption(std::string symbol)
{


    auto data=split(symbol,'_');

    if(data.size()==4 and (data[3]=="CE" or data[3]=="PE"))
                    return true;

    return false;
}
std::string getTrimmedDate(std::string runDate)
{
    std::string finalDate = runDate;
    if(runDate.find("/") != std::string::npos)
    {
        std::vector<std::string> date = split(runDate, '/');
        std::string month = (date[1]);
        std::string day = (date[0]);
        std::string year = (date[2]);
        finalDate = year + month + day;
    }
    else if (runDate.find("-") != std::string::npos)
    {
        std::vector<std::string> date = split(runDate, '-');
        std::string month = (date[1]);
        std::string day = (date[0]);
        std::string year = (date[2]);
        finalDate = year + month + day;
    }
    return finalDate;
}
long long int time_diff(long long int time1, long long int time2)
{
    long long int hour1 = time1/10000;
    long long int hour2 = time2/10000;
    long long int min1 = (time1-hour1*10000)/100;
    long long int min2 = (time2-hour2*10000)/100;
    long long int diff = (hour1-hour2)*60+(min1-min2);
    //std::cout<<"time1 "<<time1<<", time2 "<<time2<<", diff "<<diff<<std::endl;
    return diff;

}
double time_diff(string date,string exp,long long s)
{
    cout.precision(4);
    cout.setf(ios::fixed,ios::floatfield);
    int t1=stoi(date),t2=stoi(exp);
    int a=t1%100,b=(t1%10000)/100,c=t2%100,d=(t2%10000)/100;
    int p;
    if(b==d)
    {
        if(c==a)
        {
            double td=((15.0*60*60+30*60.0-((s%10000)/100)*60.0-(s/10000)*60.0*60.0))/(24.0*60*60);
            td=td/365.0;
            return td;
        }
        p=c-a-1;
    }
    else
    {
        int arr[]={31,28,31,30,31,30,31,31,30,31,30,31};
        int x=(t1%100000000)/10000;
        if(!x%4)arr[1]++;
        p= (arr[b-1]-a)+c-1;
    }
    long long s1=(s%100)+((s%10000)/100)*60+((s%1000000)/10000)*60*60;
    s1=(24*60*60)-s1;
    double r=(double)s1/(24.0*60*60);
    r=r+p;
    r=r+(24*60*60.0-15*60.0*60-30*60.0)/(24.0*60*60.0);
    r=r/365.0;
    return r;
}
/*
void getExpiry(std::string fileName, std::string finalDate)
{
    //std::string finalDate = getFormattedDate();
    //finalDate is runDate
    std::ifstream file(fileName);
    std::string line;
    std::string monthlyExpiry;
    if(!file.is_open())
    {
        std::cout<<"Error in opening expiry file"<<fileName<<std::endl;
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
                if(std::stoll(expiry)>std::stoll(finalDate))
                {
                    std::cout<<"FoundMonthlyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
                    monthlyExpiry=expiry;
                    break;
                }
            }
            if(ModeWanted==2)
            {
                if((std::stoll(expiry)/86400)>(std::stoll(finalDate)/86400))
                {
                    std::cout<<"FoundMonthlyExpiry:"<<expiry<<"|RunDate:"<<finalDate<<std::endl;
                    monthlyExpiry=expiry;
                    break;
                }
            }
        }
    }
    file.close();
    return;
}
*/
// bool isFutSymbol(std::string symbol)
// {
// 	auto data=split(symbol,'_');
	
// 	if (data.size()==2 and (data[1]=="F1" or data[1]=="F2" or data[1]=="F3"))
// 		return true;
// 	if(data.size()==3 and data[2]=="FUT")
// 		return true;
// 	return false;
// }
