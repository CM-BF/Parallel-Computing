/*划分方法
n个元素A[1..n]分成p组，每组A[(i-1)n/p+1..in/p]，i=1~p
示例：MIMD-SM模型上的PSRS排序
     begin
        (1)均匀划分：将n个元素A[1..n]均匀划分成p段，每个pi处理
                              A[(i-1)n/p+1..in/p]
        (2)局部排序：pi调用串行排序算法对A[(i-1)n/p+1..in/p]排序
        (3)选取样本：pi从其有序子序列A[(i-1)n/p+1..in/p]中选取p个样本元素
        (4)样本排序：用一台处理器对p2个样本元素进行串行排序
        (5)选择主元：用一台处理器从排好序的样本序列中选取p-1个主元，并
                             播送给其他pi
        (6)主元划分：pi按主元将有序段A[(i-1)n/p+1..in/p]划分成p段
        (7)全局交换：各处理器将其有序段按段号交换到对应的处理器中
        (8)归并排序：各处理器对接收到的元素进行归并排序
*/

#include<omp.h>
#include<stdio.h>
#include<algorithm>
#include<functional>

#define MAX_NUM 100000

int A[30], B[30], numData, numThread = 3;

// #define DEBUG
#ifdef DEBUG
#define print(format, args...) printf(format, ##args)
void Aoutput(int *arr, int b, int e){
    for(int i=b; i<e; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}
#else
#define print(format, args...)  
void Aoutput(int *arr, int b, int e){}
#endif

void Routput(int *arr, int b, int e){
    for(int i=b; i<e; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int partition(int *A, int b, int e, int key)
{
    for(int i=b; i<=e; i++){
        if(A[i] > key)
            return i;
    }
}

int main()
{
    omp_set_num_threads(numThread);
    FILE *fp;
    if((fp = fopen("data/PSRSinput.txt", "r")) == NULL) printf("Cannot open the input file!");
    fscanf(fp, "%d\n", &numData);

    for(int i=1; i<=numData; i++){
        if(i != numData){
            fscanf(fp, "%d ", &A[i]);
        }else{
            fscanf(fp, "%d\n", &A[i]);
        }
    }

    // 第1 2阶段: 局部排序
    int blockSize = numData / numThread;
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int begin = 0, end = 0;
        if(id != omp_get_num_threads() - 1){
            begin = id * blockSize + 1;
            end = (id + 1) * blockSize;
        }else{
            begin = id * blockSize + 1;
            end = numData;
        }    
        std::sort(A + begin, A + end + 1);

        // 阶段3：选取样本 B数组 
        int interval = blockSize / numThread;
        for(int i=0; i<numThread; i++){
            B[id * numThread + i] = A[begin + i * interval];
        }
    }
    // 4:排序
    std::sort(B, B + numThread * numThread);
    Aoutput(B, 0, numThread * numThread);

    // 5:选取主元
    int Main[10];
    for(int i=0; i<numThread - 1; i++){
        Main[i] = B[(i+1)*numThread];
    }
    Aoutput(Main, 0, 2);

    // 6:划分
    int par[10][20];
    int numPar[20]={0};
    int countPar[10][20];
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int begin = 0, end = 0;
        if(id != omp_get_num_threads() - 1){
            begin = id * blockSize + 1;
            end = (id + 1) * blockSize;
        }else{
            begin = id * blockSize + 1;
            end = numData;
        } 
        par[id][0] = id * blockSize + 1;
        for(int i=0; i<numThread - 1; i++){
            par[id][i+1] = partition(A, begin, end, Main[i]); 
            countPar[id][i] = par[id][i+1] - par[id][i];
            #pragma omp critical
            numPar[i] += par[id][i+1] - par[id][i];
        }
    }
    for(int i=0; i<numThread - 1; i++){
        numPar[numThread - 1] += par[i+1][0] - par[i][numThread - 1];
        countPar[i][numThread - 1] = par[i+1][0] - par[i][numThread - 1];
    }
    numPar[numThread - 1] += numData + 1 - par[numThread - 1][numThread - 1];
    countPar[numThread - 1][numThread - 1] = numData + 1 - par[numThread - 1][numThread - 1];
    
    for(int i=0; i<numThread; i++){
        for(int j=0; j<numThread; j++){
            print("%d: %d;", par[i][j], countPar[i][j]);
        }
    }
    print("\n");
    for(int i=0; i<numThread; i++){
        print("%d ", numPar[i]);
    }
    print("\n");
    Aoutput(A, 1, numData + 1);

    // 最后阶段，交换归并
    int C[30];
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int begin = 1, end = 0;
        for(int i=0; i<=id; i++){
            if(i != id)
                begin += numPar[i];
            end += numPar[i];
        }
        if(id == 0)
            print("%d %d\n", begin, end);
        int point[5]={0};
        for(int i=begin; i<=end; i++){
            C[i] = MAX_NUM;
            int min = -1;
            for(int k=0; k<numThread; k++){
                if(id == 0)
                    print("%d ", A[par[k][id] + point[k]]);
                if(point[k] < countPar[k][id] && A[par[k][id] + point[k]] < C[i]){
                    C[i] = A[par[k][id] + point[k]];
                    min = k;
                }
            }
            point[min]++;
        }
    }
    print("\n");
    Aoutput(C, 1, numData + 1);
    for(int i=1; i<=numData; i++)
        A[i] = C[i];
    Routput(A, 1, numData + 1);
    return 0;
}