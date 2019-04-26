#include<omp.h>
#include<stdio.h>


int main()
{ 
    int numThreads = 4;
    int numStep = 1000000;
    double step;
	double x, pi=0.0;

	step = 1.0 / (double)numStep;

    
    double sum=0.0;
    #pragma omp parallel for reduction(+:sum) private(x)
    for (int i=0; i<numStep; i++){
        x = (i+0.5)*step;
        sum += 4.0/(1.0+x*x);
    }
    pi = sum * step;
	printf("%lf\n",pi);
    return 0;
 }