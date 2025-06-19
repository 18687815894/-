#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

FILE *result_file;  // 全局变量，用于写入CSV结果

typedef struct {
    int id;
    int weight;
    double value;
} Item;

typedef struct {
    int* selected;
    double total_value;
    int total_weight;
} Solution;

// 生成物品数据
void generate_items(Item* items, int n) {
    for (int i = 0; i < n; i++) {
        items[i].id = i + 1;
        items[i].weight = rand() % 100 + 1;
        items[i].value = (rand() % 900 + 100) + (rand() % 100) / 100.0;
    }
}

// 生成物品统计信息CSV文件
void generate_item_stats(Item* items, int n, const char* filename) {
    FILE *csv_file = fopen(filename, "w");
    if (!csv_file) {
        printf("无法创建CSV文件: %s\n", filename);
        return;
    }

    // 写入表头
    fprintf(csv_file, "物品编号,物品重量,物品价值\n");

    // 写入物品数据
    for (int i = 0; i < n; i++) {
        fprintf(csv_file, "%d,%d,%.2f\n", 
                items[i].id, items[i].weight, items[i].value);
    }

    fclose(csv_file);
}

// 打印解决方案到屏幕和CSV文件
void print_solution(Solution sol, Item* items, int n, int capacity, 
                   clock_t start, clock_t end, const char* algorithm) {
    // 计算执行时间(毫秒)
    double execution_time = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
    
    // 输出到屏幕
    printf("算法: %s, 物品数: %d, 容量: %d\n", algorithm, n, capacity);
    printf("总重量: %d, 总价值: %.2f, 执行时间: %.2f ms\n\n", 
           sol.total_weight, sol.total_value, execution_time);
    
    // 输出到CSV文件
    fprintf(result_file, "%s,%d,%d,%d,%.2f,%.2f\n", 
            algorithm, n, capacity, sol.total_weight, sol.total_value, execution_time);
}

// 蛮力法
Solution brute_force(Item* items, int n, int capacity) {
    Solution best = {NULL, 0, 0};
    best.selected = (int*)calloc(n, sizeof(int));
    
    long long max_iter = 1LL << n;
    for (long long mask = 0; mask < max_iter; mask++) {
        int current_weight = 0;
        double current_value = 0.0;
        
        for (int i = 0; i < n; i++) {
            if (mask & (1LL << i)) {
                current_weight += items[i].weight;
                current_value += items[i].value;
            }
        }
        
        if (current_weight <= capacity && current_value > best.total_value) {
            best.total_value = current_value;
            best.total_weight = current_weight;
            for (int i = 0; i < n; i++) {
                best.selected[i] = (mask & (1LL << i)) ? 1 : 0;
            }
        }
    }
    
    return best;
}

// 动态规划
Solution dynamic_programming(Item* items, int n, int capacity) {
    double** dp = (double**)malloc((n + 1) * sizeof(double*));
    for (int i = 0; i <= n; i++) {
        dp[i] = (double*)malloc((capacity + 1) * sizeof(double));
        for (int w = 0; w <= capacity; w++) {
            dp[i][w] = 0.0;
        }
    }
    
    for (int i = 1; i <= n; i++) {
        for (int w = 1; w <= capacity; w++) {
            if (items[i-1].weight <= w) {
                dp[i][w] = fmax(dp[i-1][w], dp[i-1][w-items[i-1].weight] + items[i-1].value);
            } else {
                dp[i][w] = dp[i-1][w];
            }
        }
    }
    
    Solution sol = {(int*)calloc(n, sizeof(int)), dp[n][capacity], 0};
    int w = capacity;
    for (int i = n; i > 0; i--) {
        if (dp[i][w] != dp[i-1][w]) {
            sol.selected[i-1] = 1;
            sol.total_weight += items[i-1].weight;
            w -= items[i-1].weight;
        }
    }
    
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    
    return sol;
}

// 贪心法（按价值/重量比排序）
int compare_value_per_weight(const void* a, const void* b) {
    Item* item_a = (Item*)a;
    Item* item_b = (Item*)b;
    double ratio_a = item_a->value / item_a->weight;
    double ratio_b = item_b->value / item_b->weight;
    if (ratio_a > ratio_b) return -1;
    if (ratio_a < ratio_b) return 1;
    return 0;
}

Solution greedy(Item* items, int n, int capacity) {
    Item* sorted_items = (Item*)malloc(n * sizeof(Item));
    memcpy(sorted_items, items, n * sizeof(Item));
    qsort(sorted_items, n, sizeof(Item), compare_value_per_weight);
    
    Solution sol = {(int*)calloc(n, sizeof(int)), 0.0, 0};
    
    for (int i = 0; i < n; i++) {
        if (sol.total_weight + sorted_items[i].weight <= capacity) {
            sol.total_weight += sorted_items[i].weight;
            sol.total_value += sorted_items[i].value;
            sol.selected[sorted_items[i].id - 1] = 1;
        }
    }
    
    free(sorted_items);
    return sol;
}

// 回溯法
void backtrack(Item* items, int n, int capacity, int index, 
               int current_weight, double current_value,
               int* current_selection, Solution* best) {
    if (index == n || current_weight == capacity) {
        if (current_value > best->total_value) {
            best->total_value = current_value;
            best->total_weight = current_weight;
            memcpy(best->selected, current_selection, n * sizeof(int));
        }
        return;
    }
    
    if (current_weight + items[index].weight <= capacity) {
        current_selection[index] = 1;
        backtrack(items, n, capacity, index + 1, 
                 current_weight + items[index].weight, 
                 current_value + items[index].value,
                 current_selection, best);
        current_selection[index] = 0;
    }
    
    backtrack(items, n, capacity, index + 1, 
              current_weight, current_value,
              current_selection, best);
}

Solution backtracking(Item* items, int n, int capacity) {
    Solution best = {(int*)calloc(n, sizeof(int)), 0.0, 0};
    int* current_selection = (int*)calloc(n, sizeof(int));
    
    backtrack(items, n, capacity, 0, 0, 0.0, current_selection, &best);
    
    free(current_selection);
    return best;
}

int main() {
    // 打开CSV结果文件
    result_file = fopen("knapsack_results.csv", "w");
    if (!result_file) {
        printf("无法创建结果文件!\n");
        return 1;
    }
    
    // 写入CSV表头
    fprintf(result_file, "算法,物品数,容量,总重量,总价值,时间(ms)\n");
    
    srand(time(NULL));
    
    int item_counts[] = {10,15,20,25,30,1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 
                        10000, 20000, 40000, 80000, 160000, 320000};
    int capacities[] = {1000,2000,5000,10000, 20000,30000,50000,100000};
    int num_counts = sizeof(item_counts) / sizeof(item_counts[0]);
    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);
    
    for (int i = 0; i < num_counts; i++) {
        int n = item_counts[i];
        Item* items = (Item*)malloc(n * sizeof(Item));
        generate_items(items, n);
        
        // 当物品数量为1000时，生成物品统计CSV
        if (n == 1000) {
            generate_item_stats(items, n, "20231120137-刘福涛-数据.csv");
        }
        
        for (int j = 0; j < num_capacities; j++) {
            int capacity = capacities[j];
            
            printf("物品数量=%d, 容量=%d\n", n, capacity);
            
            // 蛮力法（仅适用于小规模）
            if (n <= 30&&capacity<=30000) {
                clock_t start = clock();
                Solution sol = brute_force(items, n, capacity);
                clock_t end = clock();
                print_solution(sol, items, n, capacity, start, end, "Brute Force");
                free(sol.selected);
            }
            
            // 动态规划（适用于中等规模）
            if (n <= 10000) {
                clock_t start = clock();
                Solution sol = dynamic_programming(items, n, capacity);
                clock_t end = clock();
                print_solution(sol, items, n, capacity, start, end, "Dynamic Programming");
                free(sol.selected);
            }
            
            // 贪心法（适用于所有规模）
            clock_t start = clock();
            Solution sol = greedy(items, n, capacity);
            clock_t end = clock();
            print_solution(sol, items, n, capacity, start, end, "Greedy");
            free(sol.selected);
            
            // 回溯法（适用于小规模）
            if (n <= 100&&capacity<20000) {
                clock_t start = clock();
                Solution sol = backtracking(items, n, capacity);
                clock_t end = clock();
                print_solution(sol, items, n, capacity, start, end, "Backtracking");
                free(sol.selected);
            }
        }
        
        free(items);
    }
    
    fclose(result_file);
    printf("结果已保存到 knapsack_results.csv\n");
    return 0;
}
