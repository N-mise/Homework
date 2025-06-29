#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

// ������Ʒ�ṹ�壬�洢��Ʒ�Ļ�����Ϣ
typedef struct {
    int goodsId;       // ��Ʒ���
    int weight;        // ��Ʒ����
    double value;      // ��Ʒ��ֵ
    double valPerWt;   // ��ֵ�����ȣ�����̰���㷨��
    int isSelected;    // �Ƿ�ѡ�У�0-δѡ��1-ѡ�У�
} Goods;

// ����������Ʒ����Ϣ
void swapTwoGoods(Goods *a, Goods *b) {
    Goods temp = *a;
    *a = *b;
    *b = temp;
}

// ���������Ʒ����
void generateRandomGoods(Goods **goods, int count) {
    *goods = (Goods*)malloc(count * sizeof(Goods));
    
    for (int i = 0; i < count; i++) {
        (*goods)[i].goodsId = i + 1;
        (*goods)[i].weight = rand() % 100 + 1;         // ������Χ1-100
        (*goods)[i].value = (rand() % 9001 + 1000) / 100.0;  // ��ֵ��Χ10.00-100.00
        (*goods)[i].isSelected = 0;  // ��ʼ��Ϊδѡ��
    }
}

// ������Ʒ���飨�����
Goods* duplicateGoods(Goods *original, int count) {
    Goods *copy = (Goods*)malloc(count * sizeof(Goods));
    // �������Ԫ�أ����memcpy�Խ��Ͳ�����
    for (int i = 0; i < count; i++) {
        copy[i] = original[i];
    }
    return copy;
}

// ���������������п��ܵ���ϣ�������С��ģ��Ʒ��
double bruteForceMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // ��¼��ʼʱ��
    
    double maxVal = 0.0;
    int bestComb = 0;  // ��¼������ϵĶ����Ʊ�ʾ
    
    // �������п��ܵ���Ʒ��ϣ�2^count�֣�
    for (int comb = 0; comb < (1 << count); comb++) {
        int totalWt = 0;
        double totalVal = 0.0;
        
        // ���㵱ǰ��ϵ����������ܼ�ֵ
        for (int j = 0; j < count; j++) {
            if (comb & (1 << j)) {  // ��jλΪ1��ʾѡ�и���Ʒ
                totalWt += goods[j].weight;
                totalVal += goods[j].value;
            }
        }
        
        // �������Ž�
        if (totalWt <= capacity && totalVal > maxVal) {
            maxVal = totalVal;
            bestComb = comb;
        }
    }
    
    // ���ѡ�е���Ʒ
    for (int j = 0; j < count; j++) {
        goods[j].isSelected = (bestComb & (1 << j)) ? 1 : 0;
    }
    
    *end = clock();  // ��¼����ʱ��
    return maxVal;
}

// ��̬�滮��������DP����⣨�������еȹ�ģ��Ʒ��
double dynamicProgrammingMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // ��¼��ʼʱ��
    
    // ����DP��dp[i][w]��ʾǰi����Ʒ������w�µ�����ֵ
    double **dp = (double**)malloc((count + 1) * sizeof(double*));
    for (int i = 0; i <= count; i++) {
        dp[i] = (double*)malloc((capacity + 1) * sizeof(double));
        // ��ʼ����ǰ������Ԫ��Ϊ0
        for (int w = 0; w <= capacity; w++) {
            dp[i][w] = 0.0;
        }
    }
    
    // ���DP��
    for (int i = 1; i <= count; i++) {
        for (int w = 1; w <= capacity; w++) {
            // ����ǰ��Ʒ����������������ѡ
            if (goods[i-1].weight > w) {
                dp[i][w] = dp[i-1][w];
            } else {
                // ѡ��ѡ��ǰ��Ʒ��ȡ���ֵ
                double include = dp[i-1][w - goods[i-1].weight] + goods[i-1].value;
                double exclude = dp[i-1][w];
                dp[i][w] = (include > exclude) ? include : exclude;
            }
        }
    }
    
    // �����ҵ�ѡ�е���Ʒ
    int remainCap = capacity;
    for (int i = count; i > 0 && remainCap > 0; i--) {
        if (dp[i][remainCap] != dp[i-1][remainCap]) {
            goods[i-1].isSelected = 1;
            remainCap -= goods[i-1].weight;
        }
    }
    
    double maxVal = dp[count][capacity];
    
    // �ͷ�DP���ڴ�
    for (int i = 0; i <= count; i++) {
        free(dp[i]);
    }
    free(dp);
    
    *end = clock();  // ��¼����ʱ��
    return maxVal;
}

// ̰���㷨������ֵ����������ѡ�������ڴ��ģ��Ʒ��
double greedyAlgorithm(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // ��¼��ʼʱ��
    
    // ����ÿ����Ʒ�ļ�ֵ������
    for (int i = 0; i < count; i++) {
        goods[i].valPerWt = goods[i].value / goods[i].weight;
    }
    
    // ����ֵ�����ȴӸߵ�������ѡ������ʵ�֣�
    for (int i = 0; i < count - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (goods[j].valPerWt > goods[maxIdx].valPerWt) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            swapTwoGoods(&goods[i], &goods[maxIdx]);
        }
    }
    
    double totalVal = 0.0;
    int remainCap = capacity;
    
    // ��������ѡ����Ʒ
    for (int i = 0; i < count && remainCap > 0; i++) {
        if (goods[i].weight <= remainCap) {
            goods[i].isSelected = 1;
            totalVal += goods[i].value;
            remainCap -= goods[i].weight;
        }
    }
    
    // ��ԭ��Żָ�˳�򣨱��������
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (goods[i].goodsId > goods[j].goodsId) {
                swapTwoGoods(&goods[i], &goods[j]);
            }
        }
    }
    
    *end = clock();  // ��¼����ʱ��
    return totalVal;
}

// ���ݷ������������ݹ�̽�����п������
void backtrackHelper(int level, int currentWt, double currentVal, double *maxVal, 
                   Goods *goods, int count, int capacity, int *bestSel) {
    if (level == count) {  // ̽����������Ʒ
        if (currentVal > *maxVal) {
            *maxVal = currentVal;
            // ��¼��ǰ����ѡ��
            for (int i = 0; i < count; i++) {
                bestSel[i] = goods[i].isSelected;
            }
        }
        return;
    }
    
    // ��ѡ��ǰ��Ʒ
    backtrackHelper(level + 1, currentWt, currentVal, maxVal, goods, count, capacity, bestSel);
    
    // ѡ��ǰ��Ʒ���������أ�
    if (currentWt + goods[level].weight <= capacity) {
        goods[level].isSelected = 1;
        backtrackHelper(level + 1, currentWt + goods[level].weight,
                      currentVal + goods[level].value, maxVal,
                      goods, count, capacity, bestSel);
        goods[level].isSelected = 0;  // ���ݣ�ȡ��ѡ��
    }
}

// ���ݷ����ݹ�̽�����Ž⣨������С��ģ��Ʒ��
double backtrackMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // ��¼��ʼʱ��
    
    double maxVal = 0.0;
    int *bestSel = (int*)calloc(count, sizeof(int));  // ��¼����ѡ��
    
    // ���ø���������ʼ����
    backtrackHelper(0, 0, 0.0, &maxVal, goods, count, capacity, bestSel);
    
    // ����ѡ��״̬
    for (int i = 0; i < count; i++) {
        goods[i].isSelected = bestSel[i];
    }
    
    free(bestSel);
    *end = clock();  // ��¼����ʱ��
    return maxVal;
}

// �����㷨���ڴ�ռ�ã��ֽڣ�
int calculateMemoryUse(Goods *goods, int count, int capacity, const char *algName) {
    int mem = 0;
    
    if (strcmp(algName, "������") == 0 || strcmp(algName, "���ݷ�") == 0) {
        // ��Ʒ���� + ����ѡ������
        mem = count * sizeof(Goods) + count * sizeof(int);
    } else if (strcmp(algName, "��̬�滮��") == 0) {
        // DP�� + ��Ʒ����
        mem = (count + 1) * (capacity + 1) * sizeof(double) + count * sizeof(Goods);
    } else if (strcmp(algName, "̰���㷨") == 0) {
        // ����Ʒ����
        mem = count * sizeof(Goods);
    }
    
    return mem;
}

// ����㷨���������̨
void printResults(Goods *goods, int count, int capacity, double totalVal, 
                 clock_t start, clock_t end, const char *algName) {
    // ��������ʱ�䣨���룩
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    // ����ѡ����Ʒ��������
    int totalWt = 0;
    for (int i = 0; i < count; i++) {
        if (goods[i].isSelected) {
            totalWt += goods[i].weight;
        }
    }
    
    // �����ڴ�ռ��
    int memUse = calculateMemoryUse(goods, count, capacity, algName);
    
    // ������
    printf("�㷨��%s\n", algName);
    printf("����������%d\n", capacity);
    printf("��Ʒ������%d\n", count);
    printf("����ֵ��%.2f\n", totalVal);
    printf("��������%d\n", totalWt);
    printf("����ʱ�䣺%.2f ms\n", elapsed);
    printf("�ڴ�ռ�ã�%d �ֽ�\n", memUse);
    
    // ���ǰ10��ѡ�е���Ʒ
    printf("ѡ�е���Ʒ��ǰ10�������������CSV�ļ�����\n");
    printf("���\t����\t��ֵ\n");
    int showCount = 0;
    for (int i = 0; i < count && showCount < 10; i++) {
        if (goods[i].isSelected) {
            printf("%d\t%d\t%.2f\n", goods[i].goodsId, goods[i].weight, goods[i].value);
            showCount++;
        }
    }
    if (showCount == 0) {
        printf("��ѡ����Ʒ\n");
    }
    printf("\n");
}

// ��ѡ����Ʒ��Ϣд��CSV�ļ������1000����
void saveToCSV(const char *filename, Goods *goods, int count, const char *algName) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("�޷����ļ� %s\n", filename);
        return;
    }
    
    int saved = 0;
    // д��ѡ�е���Ʒ������1000����
    for (int i = 0; i < count && saved < 1000; i++) {
        if (goods[i].isSelected) {
            fprintf(file, "%s,%d,%d,%.2f\n", algName, goods[i].goodsId, goods[i].weight, goods[i].value);
            saved++;
        }
    }
    
    fclose(file);
}

int main() {
    srand((unsigned int)time(NULL));  // ��ʼ�����������
    
    // ����CSV�ļ���д���ͷ
    FILE *csvFile = fopen("knapsack_results.csv", "w");
    if (csvFile) {
        fprintf(csvFile, "�㷨,��Ʒ���,��Ʒ����,��Ʒ��ֵ\n");
        fclose(csvFile);
    }
    
    const int TEST_TIMES = 1;  // ÿ���㷨���Դ���
    const int BAG_CAPACITY = 8000;  // �̶���������Ϊ8000
    
    // ��������������Ʒ����20��
    printf("=== ���������� ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int BRUTE_FORCE_ITEMS = 19;  // ��������Ʒ����
        Goods *goods;
        generateRandomGoods(&goods, BRUTE_FORCE_ITEMS);
        
        printf("���� %d����Ʒ���� = %d���������� = %d\n", i+1, BRUTE_FORCE_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, BRUTE_FORCE_ITEMS);
        clock_t startTime, endTime;
        double maxVal = bruteForceMethod(goodsCopy, BRUTE_FORCE_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, BRUTE_FORCE_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "������");
        saveToCSV("knapsack_results.csv", goodsCopy, BRUTE_FORCE_ITEMS, "������");
        
        free(goodsCopy);
        free(goods);
    }
    
    // ���Զ�̬�滮������Ʒ����4000��
    printf("=== ��̬�滮������ ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int DP_ITEMS = 4000;  // ��̬�滮����Ʒ����
        Goods *goods;
        generateRandomGoods(&goods, DP_ITEMS);
        
        printf("���� %d����Ʒ���� = %d���������� = %d\n", i+1, DP_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, DP_ITEMS);
        clock_t startTime, endTime;
        double maxVal = dynamicProgrammingMethod(goodsCopy, DP_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, DP_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "��̬�滮��");
        saveToCSV("knapsack_results.csv", goodsCopy, DP_ITEMS, "��̬�滮��");
        
        free(goodsCopy);
        free(goods);
    }
    
    // ����̰���㷨����Ʒ����5000��
    printf("=== ̰���㷨���� ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int GREEDY_ITEMS = 5000;  // ̰���㷨��Ʒ����
        Goods *goods;
        generateRandomGoods(&goods, GREEDY_ITEMS);
        
        printf("���� %d����Ʒ���� = %d���������� = %d\n", i+1, GREEDY_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, GREEDY_ITEMS);
        clock_t startTime, endTime;
        double maxVal = greedyAlgorithm(goodsCopy, GREEDY_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, GREEDY_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "̰���㷨");
        saveToCSV("knapsack_results.csv", goodsCopy, GREEDY_ITEMS, "̰���㷨");
        
        free(goodsCopy);
        free(goods);
    }
    
    // ���Ի��ݷ�����Ʒ����25��
    printf("=== ���ݷ����� ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int BACKTRACK_ITEMS = 25;  // ���ݷ���Ʒ����
        Goods *goods;
        generateRandomGoods(&goods, BACKTRACK_ITEMS);
        
        printf("���� %d����Ʒ���� = %d���������� = %d\n", i+1, BACKTRACK_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, BACKTRACK_ITEMS);
        clock_t startTime, endTime;
        double maxVal = backtrackMethod(goodsCopy, BACKTRACK_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, BACKTRACK_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "���ݷ�");
        saveToCSV("knapsack_results.csv", goodsCopy, BACKTRACK_ITEMS, "���ݷ�");
        
        free(goodsCopy);
        free(goods);
    }
    
    return 0;
}
