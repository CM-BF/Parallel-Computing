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

#include<stdio.h>
#include<mpi.h>
#include<algorithm>
#include<vector>
using namespace std;

void output(int rank, vector<int> vec){
    printf("%d:  ", rank);
    for(auto it=vec.begin(); it != vec.end(); it++){
        printf("%d ", *it);
    }
    printf("\n");
}

vector<int> partition(vector<int> &arr, int key, vector<int>::iterator &last){
    auto iti = arr.begin() - 1;
    vector<int> segArr;
    for(auto itj = arr.begin(); itj != arr.end(); itj++){
        if(*itj <= key){
            iti++;
            int tmp = *iti;
            *iti = *itj;
            *itj = tmp;
            if (itj >= last)
                segArr.push_back(*iti);
        }
    }
    last = iti + 1;
    return segArr;
}

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);
    int pSize, rank;
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &pSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    FILE *fp;
    if((fp = fopen("data/PSRSinput.txt", "r")) == NULL) printf("Cannot open the input file.");
    vector<int> pArr;
    int A[50], numData;
    
    // 每个节点都读取输入文件中的数据
    fscanf(fp, "%d\n", &numData);

    for(int i=0; i<numData; i++){
        if(i != numData){
            fscanf(fp, "%d ", &A[i]);
        }else{
            fscanf(fp, "%d\n", &A[i]);
        }
    }

    // 取出属于自己的部分
    int blockSize = numData / pSize;
    int begin = rank * blockSize, end;
    if (rank != pSize - 1){
        end = (rank + 1) * blockSize;
    }else{
        end = numData;
    }

    for(int i = begin; i < end; i++){
        pArr.push_back(A[i]);
    }
    sort(pArr.begin(), pArr.end());
    //output(rank, pArr);

    // 阶段3：选取样本发送给0节点
    int stride = blockSize / pSize;
    MPI_Datatype mainElement;
    MPI_Type_vector(pSize, 1, stride, MPI_INT, &mainElement);
    MPI_Type_commit(&mainElement);
    int tmp[30];
    vector<int> P;
    vector<int> mainP;
    MPI_Gather(&(*pArr.begin()), 1, mainElement, &tmp, pSize, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0){
        for(int i=0; i<pSize*pSize; i++){
            P.push_back(tmp[i]);
        }
        sort(P.begin(), P.end());

        // 0节点选取主元
        for(int i=1; i<pSize; i++){
            mainP.push_back(P.at(i * stride));
        }
        
    }else{
        for(int i=1; i<pSize; i++){
            mainP.push_back(0);
        }
    }
    MPI_Bcast(&(*mainP.begin()), pSize - 1, MPI_INT, 0, MPI_COMM_WORLD);
    // 划分
    vector<int> segArr[4];
    auto last = pArr.begin();
    for(auto it=mainP.begin(); it != mainP.end(); it++){
        segArr[it-mainP.begin()] = partition(pArr, *it, last);
    }
    segArr[pSize - 1] = partition(pArr, 99999, last);

    
    // 全局交换
    // 发送
    for(int i=0; i<pSize; i++){
        if (i == rank)
            continue;
        int sendSize = segArr[i].size();
        MPI_Send(&sendSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        MPI_Send(&(*segArr[i].begin()), sendSize, MPI_INT, i, 2, MPI_COMM_WORLD);
    }
    // 接收
    vector<int> recArr[4];
    recArr[rank] = segArr[rank];
    for(int i=0; i<pSize; i++){
        if(i == rank)
            continue;
        int recSize;
        MPI_Recv(&recSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
        int tmp[10];
        MPI_Recv(&tmp, recSize, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
        for(int j=0; j<recSize; j++){
            recArr[i].push_back(tmp[j]);
        }
    }

    // 归并排序
    vector<int> lastArr;
    vector<int>::iterator it[4];
    int totalSize = 0;
    
    for(int i=0; i<pSize; i++){
        it[i] = recArr[i].begin();
        totalSize += recArr[i].size();
    }
    while(lastArr.size() < totalSize){
        int mini = -1, min = 999999;
        for(int i=0; i<pSize; i++){
            if(it[i] != recArr[i].end() && *it[i] <= min){
                min = *it[i];
                mini = i;
            }
        }
        lastArr.push_back(min);
        it[mini]++;
    }
    output(rank, lastArr);
    



    MPI_Finalize();
    return 0;
}