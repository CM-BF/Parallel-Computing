#include<omp.h>
#include<stdio.h>



int main()
{ 
    int numThreads = 4;
    int numStep = 1000000;
    double step = 1.0 / (double)numStep;
	double pi=0.0, sum[10]={0};

    omp_set_num_threads(numThreads);
    #pragma omp parallel
    {
        double x;
        int id;
        id = omp_get_thread_num();
        sum[id]=0.0;
        for (int i=id; i<numStep; i=i+numThreads){
            x = (i+0.5)*step;
            sum[id] += 4.0/(1.0+x*x);
        }
    }

	for(int i=0; i<numThreads; i++)  
        pi += sum[i] * step;
	printf("%lf\n",pi);
    return 0;
 }