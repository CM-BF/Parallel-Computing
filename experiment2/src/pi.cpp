#include<stdio.h>
#include<mpi.h>

#define N 10000000

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);
    MPI_Status status;
    int pSize, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &pSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // 各节点并行计算结果
    double pSum=0, x, step=1.0/N;
    for(int i=rank; i < N; i += pSize){
        x = (i + 0.5) * step;
        pSum += 4.0 / (1 + x*x);
    }

    // 串行合并节点为0
    if(rank == 0){
        double sum=0;
        sum = pSum;
        double revSum;
        for(int i=1; i<pSize; i++){
            MPI_Recv(&revSum, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &status);
            sum += revSum;
        }
        sum *= step;
        printf("%.12f\n", sum);
    }else{
        MPI_Send(&pSum, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}