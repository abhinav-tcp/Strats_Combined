#include "quadDistribution.h"

QuadDistribution::QuadDistribution(double m , double c,int extremeDist,double atm)
{
	_lm=m;
	_lc=c;
//	_q_X_coordinate=atm;

}
QuadDistribution::QuadDistribution()
{
}





//std::vector<std::pair<int,double>> QuadDistribution::getLots(double atm,int strikeLevel, int strikesGap,int totalLots) //extremeDist = strikesGap* strikeLevel
std::vector<std::pair<int,double>> QuadDistribution::getLots(int leftMost,int rightMost, int strikesGap,int totalLots) //extremeDist = strikesGap* strikeLevel
{
	std::vector<std::pair<int,double>> ratio;
    #ifdef __DEBUGPRINT__

   // std::cout<<"LEFT:"<<leftMost<<"|RIGHTMOST:"<<rightMost<<"|SG:"<<strikesGap<<"|TOTALLOTS:"<<totalLots<<std::endl;
    #endif
	for(int i=leftMost;i<=rightMost;i+=strikesGap)
	{
		ratio.push_back({i,get_Y_at_X(i)});
	}

	double totalSum=0;
	for(auto &x: ratio)totalSum+=x.second;

	std::vector<std::pair<int,double>> lots;
    if(totalSum==0 and leftMost==rightMost)
    {
        ratio={std::make_pair(leftMost,totalLots)};
        totalSum=totalLots;
    }
    if(totalSum==0)totalSum=1; // incase of all 0 edge case handle
    
	for(auto &x : ratio)
	{
		int lot=x.second*totalLots/totalSum;
		std::cout<<x.first<<","<<x.second<<std::endl;		
		lots.push_back(std::make_pair(x.first,lot));
	}
	

	// std::cout<<"Lots:";
	// for(auto &lot:lots)
	// 	{
	// 		std::cout<<lot.first<<":"<<lot.second<<" ";
	// 	}
	std::cout<<std::endl;
	return lots;
}


std::vector<double> QuadDistribution::getQuadEqnFromPoints(int maxLots,int minLots,int atm,int dist)
{
	std::vector<double> _vertexPoint={double(atm),double(minLots)};

        std::vector<double> _extremeRight={double(atm+dist),double(maxLots)};
        std::vector<double> _extremeLeft= {double(atm-dist), double(maxLots) };

	std::vector<std::vector<double>> matrix;
    std::vector<std::vector<double>> points={_vertexPoint,_extremeRight,_extremeLeft};
        for(auto &point : points)
        {
                double x=point[0],y=point[1];
                std::vector<double> row={x*x,x,1,y};                                                                                                                                                                             std::cout<<"POINTS:"<<x<<":"<<y<<std::endl;
                matrix.push_back(row);
        }


        auto sol= findSolution(matrix);

        _qa=sol[0];_qb=sol[1];_qc=sol[2];

        return sol;
}
// std::vector<double> QuadDistribution::getQuadEqnFromLinear(double m, double c,double atm, int extremeDistCall,int extremeDistPut)
// {

// 	//get three points through which the line will pass
// 	std::vector<double> _vertexPoint={atm,m*atm+c};

// 	std::vector<double> _extremeRight={atm+extremeDistCall, m*(atm+extremeDistCall)+c};
// 	std::vector<double> _extremeLeft= {atm-extremeDistPut, m* (atm+extremeDistPut)+c };


// 	std::vector<std::vector<double>> points={_vertexPoint,_extremeRight,_extremeLeft};

// 	//gte matrix
// 	//
	
// 	std::vector<std::vector<double>> matrix;

// 	for(auto &point : points)
// 	{
// 		double x=point[0],y=point[1];
// 		std::vector<double> row={x*x,x,1,y};
// 		std::cout<<"POINTS:"<<x<<":"<<y<<std::endl;
// 		matrix.push_back(row);
// 	}


// 	auto sol= findSolution(matrix); 

// 	_qa=sol[0];_qb=sol[1];_qc=sol[2];

// 	return sol;






// }












// This functions finds the determinant of Matrix
double QuadDistribution::determinantOfMatrix(double mat[3][3])
{
    double ans;
    ans = mat[0][0] * (mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2])
          - mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0])
          + mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
    return ans;
}

// This function finds the solution of system of
// linear equations using cramer's rule 
//
std::vector<double> QuadDistribution::findSolution(std::vector<std::vector<double>> coeff)
{
    // Matrix d using coeff as given in cramer's rule
    double d[3][3] = {
        { coeff[0][0], coeff[0][1], coeff[0][2] },
        { coeff[1][0], coeff[1][1], coeff[1][2] },
        { coeff[2][0], coeff[2][1], coeff[2][2] },
    };
    // Matrix d1 using coeff as given in cramer's rule
    double d1[3][3] = {
        { coeff[0][3], coeff[0][1], coeff[0][2] },
        { coeff[1][3], coeff[1][1], coeff[1][2] },
        { coeff[2][3], coeff[2][1], coeff[2][2] },
    };
    // Matrix d2 using coeff as given in cramer's rule
    double d2[3][3] = {
        { coeff[0][0], coeff[0][3], coeff[0][2] },
        { coeff[1][0], coeff[1][3], coeff[1][2] },
        { coeff[2][0], coeff[2][3], coeff[2][2] },
    };
    // Matrix d3 using coeff as given in cramer's rule
    double d3[3][3] = {
        { coeff[0][0], coeff[0][1], coeff[0][3] },
        { coeff[1][0], coeff[1][1], coeff[1][3] },
        { coeff[2][0], coeff[2][1], coeff[2][3] },
    };

    // Calculating Determinant of Matrices d, d1, d2, d3
    double D = determinantOfMatrix(d);
    double D1 = determinantOfMatrix(d1);
    double D2 = determinantOfMatrix(d2);
    double D3 = determinantOfMatrix(d3);
    printf("D is : %lf \n", D);
    printf("D1 is : %lf \n", D1);
    printf("D2 is : %lf \n", D2);
    printf("D3 is : %lf \n", D3);

    // Case 1
    if (D != 0) {
        // Coeff have a unique solution. Apply Cramer's Rule
        double x = D1 / D;
        double y = D2 / D;
        double z = D3 / D; // calculating z using cramer's rule
        printf("Value of x is : %lf\n", x);
        printf("Value of y is : %lf\n", y);
        printf("Value of z is : %lf\n", z);

	return {x,y,z};
    }
    // Case 2
    else {
        if (D1 == 0 && D2 == 0 && D3 == 0)
            printf("Infinite solutions\n");
        else if (D1 != 0 || D2 != 0 || D3 != 0)
            printf("No solutions\n");

	return {0,0,0};
    }
    return {0,0,0};
}

// Driver Code
/*int main()
{

    // storing coefficients of linear equations in coeff matrix
    double coeff[3][4] = {
        { 2, -1, 3, 9 },
        { 1, 1, 1, 6 },
        { 1, -1, 1, 2 },
    };

    findSolution(coeff);
    return 0;
}
*/
