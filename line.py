import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import linregress
import warnings
from matplotlib.font_manager import FontProperties

# ====================== 初始化配置 ======================
def init_plot_settings():
    """初始化绘图设置"""
    plt.rcParams['font.sans-serif'] = ['SimHei']  # 中文显示
    plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题
    plt.rcParams['figure.dpi'] = 300  # 高清输出
    plt.rcParams['font.size'] = 10
    warnings.filterwarnings("ignore")  # 忽略警告

# ====================== 理论复杂度函数 ======================
def theoretical_complexity(n, algo, capacity=None):
    """返回算法的理论时间复杂度"""
    n = np.array(n, dtype=np.float64)
    if algo == 'Brute Force':
        return 2 ** n
    elif algo == 'Dynamic Programming':
        return n * capacity if capacity else n * 1000  # 默认容量1000
    elif algo == 'Greedy':
        return n * np.log2(np.clip(n, 1, None))
    elif algo == 'Backtracking':
        return 2 ** n
    return np.zeros_like(n)

# ====================== 数据加载（GBK编码处理） ======================
def load_data():
    """加载并验证GBK编码的CSV数据"""
    try:
        # 指定GBK编码读取，并处理可能的BOM头
        df = pd.read_csv('knapsack_results.csv', encoding='gbk')
        
        # 检查列名是否含非法字符
        df.columns = df.columns.str.strip().str.replace('\ufeff', '')
        
        # 列名标准化（兼容中英文）
        col_mapping = {
            '算法': 'algorithm',
            '物品数': 'n',
            '容量': 'capacity',
            '总重量': 'total_weight',
            '总价值': 'total_value',
            '时间(ms)': 'time_ms',
            '时间（ms）': 'time_ms'  # 兼容全角括号
        }
        df = df.rename(columns=lambda x: col_mapping.get(x.strip(), x))
        
        # 数据清洗
        df = df.dropna(subset=['time_ms'])
        df['time_ms'] = pd.to_numeric(df['time_ms'], errors='coerce')
        df = df[df['time_ms'] > 0]
        
        # 算法名称标准化
        df['algorithm'] = df['algorithm'].str.strip().replace({
            'Brute Force': 'Brute Force',
            '暴力搜索': 'Brute Force',
            'Backtracking': 'Backtracking',
            '回溯法': 'Backtracking',
            'Dynamic Programming': 'Dynamic Programming',
            '动态规划': 'Dynamic Programming',
            'Greedy': 'Greedy',
            '贪心算法': 'Greedy'
        })
        return df
    except Exception as e:
        print(f"数据加载失败: {str(e)}\n请检查：")
        print("1. 文件是否存在 2. 编码是否为GBK 3. 列名是否匹配")
        return None

# ====================== 可视化分析 ======================
def plot_algorithm_groups(df, capacity):
    """分组绘制算法复杂度对比"""
    # 样式配置
    style_config = {
        'Brute Force': {'color': '#E63946', 'marker': 'o', 'linewidth': 2},
        'Backtracking': {'color': '#457B9D', 'marker': 's', 'linewidth': 2},
        'Dynamic Programming': {'color': '#2A9D8F', 'marker': '^', 'linewidth': 2},
        'Greedy': {'color': '#F4A261', 'marker': 'd', 'linewidth': 2}
    }
    
    # 创建画布
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 6))
    
    # ===== 第一组：指数级算法 =====
    exp_algorithms = ['Brute Force', 'Backtracking']
    for algo in exp_algorithms:
        subset = df[(df['algorithm'] == algo) & (df['capacity'] == capacity)]
        if len(subset) == 0:
            print(f"警告: 容量 {capacity} 下没有 {algo} 的数据")
            continue
            
        # 实际曲线
        ax1.plot(subset['n'], subset['time_ms'], 
                label=f'{algo} (实际)',
                **style_config[algo])
        
        # 理论曲线（缩放拟合）
        n = np.linspace(subset['n'].min(), subset['n'].max(), 100)
        theory = theoretical_complexity(n, algo, capacity)
        scale_factor = subset['time_ms'].median() / theoretical_complexity(
            subset['n'].median(), algo, capacity)
        ax1.plot(n, theory * scale_factor,
                label=f'{algo} (理论)',
                color=style_config[algo]['color'],
                linestyle='--',
                alpha=0.6,
                linewidth=2)
    
    ax1.set_title(f'指数级算法对比 (容量={capacity})', pad=15)
    ax1.set_xlabel('物品数量 (n)')
    ax1.set_ylabel('执行时间 (ms)')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    ax1.set_yscale('log')
    
    # ===== 第二组：多项式级算法 =====
    poly_algorithms = ['Dynamic Programming', 'Greedy']
    for algo in poly_algorithms:
        subset = df[(df['algorithm'] == algo) & (df['capacity'] == capacity)]
        if len(subset) == 0:
            print(f"警告: 容量 {capacity} 下没有 {algo} 的数据")
            continue
            
        # 实际曲线
        ax2.plot(subset['n'], subset['time_ms'], 
                label=f'{algo} (实际)',
                **style_config[algo])
        
        # 理论曲线
        n = np.linspace(subset['n'].min(), subset['n'].max(), 100)
        theory = theoretical_complexity(n, algo, capacity)
        scale_factor = subset['time_ms'].median() / theoretical_complexity(
            subset['n'].median(), algo, capacity)
        ax2.plot(n, theory * scale_factor,
                label=f'{algo} (理论)',
                color=style_config[algo]['color'],
                linestyle='--',
                alpha=0.6,
                linewidth=2)
    
    ax2.set_title(f'多项式级算法对比 (容量={capacity})', pad=15)
    ax2.set_xlabel('物品数量 (n)')
    ax2.set_ylabel('执行时间 (ms)')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # 自动决定是否使用对数坐标
    time_values = df[(df['capacity'] == capacity) & 
                    (df['algorithm'].isin(poly_algorithms))]['time_ms']
    if len(time_values) > 0 and time_values.max() / time_values.min() > 100:
        ax2.set_yscale('log')
    
    plt.tight_layout()
    plt.savefig(f'knapsack_capacity_{capacity}.png', dpi=300, bbox_inches='tight')
    plt.close()

# ====================== 主程序 ======================
def main():
    init_plot_settings()
    print("正在加载数据（GBK编码）...")
    df = load_data()
    if df is None:
        print("请确认：")
        print("1. 已运行C程序生成knapsack_results.csv")
        print("2. 文件放在同一目录下")
        print("3. 文件编码确实是GBK（可用记事本另存为确认）")
        return
    
    print("\n数据摘要:")
    print(df.groupby(['algorithm', 'capacity'])['time_ms'].describe())
    
    # 为每个容量生成对比图
    for capacity in sorted(df['capacity'].unique()):
        print(f"\n正在分析容量 {capacity}...")
        plot_algorithm_groups(df, capacity)
    
    print("\n分析完成！图表已保存为 knapsack_capacity_*.png")

if __name__ == '__main__':
    main()