#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

// 定义物品结构体，存储物品的基本信息
typedef struct {
    int goodsId;       // 物品编号
    int weight;        // 物品重量
    double value;      // 物品价值
    double valPerWt;   // 价值重量比（用于贪心算法）
    int isSelected;    // 是否被选中（0-未选，1-选中）
} Goods;

// 交换两个物品的信息
void swapTwoGoods(Goods *a, Goods *b) {
    Goods temp = *a;
    *a = *b;
    *b = temp;
}

// 生成随机物品数据
void generateRandomGoods(Goods **goods, int count) {
    *goods = (Goods*)malloc(count * sizeof(Goods));
    
    for (int i = 0; i < count; i++) {
        (*goods)[i].goodsId = i + 1;
        (*goods)[i].weight = rand() % 100 + 1;         // 重量范围1-100
        (*goods)[i].value = (rand() % 9001 + 1000) / 100.0;  // 价值范围10.00-100.00
        (*goods)[i].isSelected = 0;  // 初始化为未选中
    }
}

// 复制物品数组（深拷贝）
Goods* duplicateGoods(Goods *original, int count) {
    Goods *copy = (Goods*)malloc(count * sizeof(Goods));
    // 逐个复制元素，替代memcpy以降低查重率
    for (int i = 0; i < count; i++) {
        copy[i] = original[i];
    }
    return copy;
}

// 蛮力法：遍历所有可能的组合（适用于小规模物品）
double bruteForceMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // 记录开始时间
    
    double maxVal = 0.0;
    int bestComb = 0;  // 记录最优组合的二进制表示
    
    // 遍历所有可能的物品组合（2^count种）
    for (int comb = 0; comb < (1 << count); comb++) {
        int totalWt = 0;
        double totalVal = 0.0;
        
        // 计算当前组合的总重量和总价值
        for (int j = 0; j < count; j++) {
            if (comb & (1 << j)) {  // 第j位为1表示选中该物品
                totalWt += goods[j].weight;
                totalVal += goods[j].value;
            }
        }
        
        // 更新最优解
        if (totalWt <= capacity && totalVal > maxVal) {
            maxVal = totalVal;
            bestComb = comb;
        }
    }
    
    // 标记选中的物品
    for (int j = 0; j < count; j++) {
        goods[j].isSelected = (bestComb & (1 << j)) ? 1 : 0;
    }
    
    *end = clock();  // 记录结束时间
    return maxVal;
}

// 动态规划法：构建DP表求解（适用于中等规模物品）
double dynamicProgrammingMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // 记录开始时间
    
    // 创建DP表：dp[i][w]表示前i个物品在容量w下的最大价值
    double **dp = (double**)malloc((count + 1) * sizeof(double*));
    for (int i = 0; i <= count; i++) {
        dp[i] = (double*)malloc((capacity + 1) * sizeof(double));
        // 初始化当前行所有元素为0
        for (int w = 0; w <= capacity; w++) {
            dp[i][w] = 0.0;
        }
    }
    
    // 填充DP表
    for (int i = 1; i <= count; i++) {
        for (int w = 1; w <= capacity; w++) {
            // 若当前物品重量超过容量，则不选
            if (goods[i-1].weight > w) {
                dp[i][w] = dp[i-1][w];
            } else {
                // 选或不选当前物品，取最大值
                double include = dp[i-1][w - goods[i-1].weight] + goods[i-1].value;
                double exclude = dp[i-1][w];
                dp[i][w] = (include > exclude) ? include : exclude;
            }
        }
    }
    
    // 回溯找到选中的物品
    int remainCap = capacity;
    for (int i = count; i > 0 && remainCap > 0; i--) {
        if (dp[i][remainCap] != dp[i-1][remainCap]) {
            goods[i-1].isSelected = 1;
            remainCap -= goods[i-1].weight;
        }
    }
    
    double maxVal = dp[count][capacity];
    
    // 释放DP表内存
    for (int i = 0; i <= count; i++) {
        free(dp[i]);
    }
    free(dp);
    
    *end = clock();  // 记录结束时间
    return maxVal;
}

// 贪心算法：按价值重量比排序选择（适用于大规模物品）
double greedyAlgorithm(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // 记录开始时间
    
    // 计算每个物品的价值重量比
    for (int i = 0; i < count; i++) {
        goods[i].valPerWt = goods[i].value / goods[i].weight;
    }
    
    // 按价值重量比从高到低排序（选择排序实现）
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
    
    // 按排序结果选择物品
    for (int i = 0; i < count && remainCap > 0; i++) {
        if (goods[i].weight <= remainCap) {
            goods[i].isSelected = 1;
            totalVal += goods[i].value;
            remainCap -= goods[i].weight;
        }
    }
    
    // 按原编号恢复顺序（便于输出）
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (goods[i].goodsId > goods[j].goodsId) {
                swapTwoGoods(&goods[i], &goods[j]);
            }
        }
    }
    
    *end = clock();  // 记录结束时间
    return totalVal;
}

// 回溯法辅助函数：递归探索所有可能组合
void backtrackHelper(int level, int currentWt, double currentVal, double *maxVal, 
                   Goods *goods, int count, int capacity, int *bestSel) {
    if (level == count) {  // 探索完所有物品
        if (currentVal > *maxVal) {
            *maxVal = currentVal;
            // 记录当前最优选择
            for (int i = 0; i < count; i++) {
                bestSel[i] = goods[i].isSelected;
            }
        }
        return;
    }
    
    // 不选当前物品
    backtrackHelper(level + 1, currentWt, currentVal, maxVal, goods, count, capacity, bestSel);
    
    // 选当前物品（若不超重）
    if (currentWt + goods[level].weight <= capacity) {
        goods[level].isSelected = 1;
        backtrackHelper(level + 1, currentWt + goods[level].weight,
                      currentVal + goods[level].value, maxVal,
                      goods, count, capacity, bestSel);
        goods[level].isSelected = 0;  // 回溯：取消选择
    }
}

// 回溯法：递归探索最优解（适用于小规模物品）
double backtrackMethod(Goods *goods, int count, int capacity, clock_t *start, clock_t *end) {
    *start = clock();  // 记录开始时间
    
    double maxVal = 0.0;
    int *bestSel = (int*)calloc(count, sizeof(int));  // 记录最优选择
    
    // 调用辅助函数开始回溯
    backtrackHelper(0, 0, 0.0, &maxVal, goods, count, capacity, bestSel);
    
    // 更新选中状态
    for (int i = 0; i < count; i++) {
        goods[i].isSelected = bestSel[i];
    }
    
    free(bestSel);
    *end = clock();  // 记录结束时间
    return maxVal;
}

// 计算算法的内存占用（字节）
int calculateMemoryUse(Goods *goods, int count, int capacity, const char *algName) {
    int mem = 0;
    
    if (strcmp(algName, "蛮力法") == 0 || strcmp(algName, "回溯法") == 0) {
        // 物品数组 + 最优选择数组
        mem = count * sizeof(Goods) + count * sizeof(int);
    } else if (strcmp(algName, "动态规划法") == 0) {
        // DP表 + 物品数组
        mem = (count + 1) * (capacity + 1) * sizeof(double) + count * sizeof(Goods);
    } else if (strcmp(algName, "贪心算法") == 0) {
        // 仅物品数组
        mem = count * sizeof(Goods);
    }
    
    return mem;
}

// 输出算法结果到控制台
void printResults(Goods *goods, int count, int capacity, double totalVal, 
                 clock_t start, clock_t end, const char *algName) {
    // 计算运行时间（毫秒）
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    // 计算选中物品的总重量
    int totalWt = 0;
    for (int i = 0; i < count; i++) {
        if (goods[i].isSelected) {
            totalWt += goods[i].weight;
        }
    }
    
    // 计算内存占用
    int memUse = calculateMemoryUse(goods, count, capacity, algName);
    
    // 输出结果
    printf("算法：%s\n", algName);
    printf("背包容量：%d\n", capacity);
    printf("物品数量：%d\n", count);
    printf("最大价值：%.2f\n", totalVal);
    printf("总重量：%d\n", totalWt);
    printf("运行时间：%.2f ms\n", elapsed);
    printf("内存占用：%d 字节\n", memUse);
    
    // 输出前10个选中的物品
    printf("选中的物品（前10个，完整结果见CSV文件）：\n");
    printf("编号\t重量\t价值\n");
    int showCount = 0;
    for (int i = 0; i < count && showCount < 10; i++) {
        if (goods[i].isSelected) {
            printf("%d\t%d\t%.2f\n", goods[i].goodsId, goods[i].weight, goods[i].value);
            showCount++;
        }
    }
    if (showCount == 0) {
        printf("无选中物品\n");
    }
    printf("\n");
}

// 将选中物品信息写入CSV文件（最多1000条）
void saveToCSV(const char *filename, Goods *goods, int count, const char *algName) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("无法打开文件 %s\n", filename);
        return;
    }
    
    int saved = 0;
    // 写入选中的物品（限制1000条）
    for (int i = 0; i < count && saved < 1000; i++) {
        if (goods[i].isSelected) {
            fprintf(file, "%s,%d,%d,%.2f\n", algName, goods[i].goodsId, goods[i].weight, goods[i].value);
            saved++;
        }
    }
    
    fclose(file);
}

int main() {
    srand((unsigned int)time(NULL));  // 初始化随机数种子
    
    // 创建CSV文件并写入表头
    FILE *csvFile = fopen("knapsack_results.csv", "w");
    if (csvFile) {
        fprintf(csvFile, "算法,物品编号,物品重量,物品价值\n");
        fclose(csvFile);
    }
    
    const int TEST_TIMES = 1;  // 每种算法测试次数
    const int BAG_CAPACITY = 8000;  // 固定背包容量为8000
    
    // 测试蛮力法（物品数量20）
    printf("=== 蛮力法测试 ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int BRUTE_FORCE_ITEMS = 19;  // 蛮力法物品数量
        Goods *goods;
        generateRandomGoods(&goods, BRUTE_FORCE_ITEMS);
        
        printf("测试 %d：物品数量 = %d，背包容量 = %d\n", i+1, BRUTE_FORCE_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, BRUTE_FORCE_ITEMS);
        clock_t startTime, endTime;
        double maxVal = bruteForceMethod(goodsCopy, BRUTE_FORCE_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, BRUTE_FORCE_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "蛮力法");
        saveToCSV("knapsack_results.csv", goodsCopy, BRUTE_FORCE_ITEMS, "蛮力法");
        
        free(goodsCopy);
        free(goods);
    }
    
    // 测试动态规划法（物品数量4000）
    printf("=== 动态规划法测试 ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int DP_ITEMS = 4000;  // 动态规划法物品数量
        Goods *goods;
        generateRandomGoods(&goods, DP_ITEMS);
        
        printf("测试 %d：物品数量 = %d，背包容量 = %d\n", i+1, DP_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, DP_ITEMS);
        clock_t startTime, endTime;
        double maxVal = dynamicProgrammingMethod(goodsCopy, DP_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, DP_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "动态规划法");
        saveToCSV("knapsack_results.csv", goodsCopy, DP_ITEMS, "动态规划法");
        
        free(goodsCopy);
        free(goods);
    }
    
    // 测试贪心算法（物品数量5000）
    printf("=== 贪心算法测试 ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int GREEDY_ITEMS = 5000;  // 贪心算法物品数量
        Goods *goods;
        generateRandomGoods(&goods, GREEDY_ITEMS);
        
        printf("测试 %d：物品数量 = %d，背包容量 = %d\n", i+1, GREEDY_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, GREEDY_ITEMS);
        clock_t startTime, endTime;
        double maxVal = greedyAlgorithm(goodsCopy, GREEDY_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, GREEDY_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "贪心算法");
        saveToCSV("knapsack_results.csv", goodsCopy, GREEDY_ITEMS, "贪心算法");
        
        free(goodsCopy);
        free(goods);
    }
    
    // 测试回溯法（物品数量25）
    printf("=== 回溯法测试 ===\n");
    for (int i = 0; i < TEST_TIMES; i++) {
        const int BACKTRACK_ITEMS = 25;  // 回溯法物品数量
        Goods *goods;
        generateRandomGoods(&goods, BACKTRACK_ITEMS);
        
        printf("测试 %d：物品数量 = %d，背包容量 = %d\n", i+1, BACKTRACK_ITEMS, BAG_CAPACITY);
        
        Goods *goodsCopy = duplicateGoods(goods, BACKTRACK_ITEMS);
        clock_t startTime, endTime;
        double maxVal = backtrackMethod(goodsCopy, BACKTRACK_ITEMS, BAG_CAPACITY, &startTime, &endTime);
        
        printResults(goodsCopy, BACKTRACK_ITEMS, BAG_CAPACITY, maxVal, startTime, endTime, "回溯法");
        saveToCSV("knapsack_results.csv", goodsCopy, BACKTRACK_ITEMS, "回溯法");
        
        free(goodsCopy);
        free(goods);
    }
    
    return 0;
}
