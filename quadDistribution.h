//#include<math>
#include<vector>
#include <iostream>
class QuadDistribution
{
	public:
	QuadDistribution(double m, double c,int extremeDist,double atm);
	QuadDistribution();
	private:

	double _lm=0,_lc=0;
	double _qa=0,_qb=0,_qc=0;

	double get_D(){ return _qb*_qb-4*_qa*_qc;}

	double get_Y_at_X(double x) { return _qa*x*x + _qb*x + _qc;}
	
	public:
	std::vector<double> getQuadEqnFromLinear(double m,double c,double atm, int extremeDist); //get quadratic equation from linear Equation
	std::vector<double> getQuadEqnFromPoints(int maxLots,int minLots,int atm,int dist);
	double determinantOfMatrix(double mat[3][3]);
	std::vector<double> findSolution(std::vector<std::vector<double>> coeff);
	std::vector<std::pair<int,double>> getLots(double atm,int strikeLevel, int strikesGap,int totalLots);
	std::vector<std::pair<int,double>> getLots(int leftMost,int rightMost, int strikesGap,int totalLots);


};

