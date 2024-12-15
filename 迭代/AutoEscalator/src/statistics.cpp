#include "statistics.h"
#include <iostream>
#include <iomanip>

Statistics::Statistics() {
    floorUsage.resize(15, 0); // 0号索引不使用，1-14层
}

void Statistics::recordPassenger(int fromFloor, int toFloor) {
    // 记录出发楼层和目标楼层的使用次数
    floorUsage[fromFloor]++;
    floorUsage[toFloor]++;
}

void Statistics::reset() {
    std::fill(floorUsage.begin(), floorUsage.end(), 0);
}

void Statistics::displayChart() const {
    std::cout << "\n=== 楼层使用统计 ===" << std::endl;
    
    // 找出最大使用次数，用于计算比例
    int maxUsage = *std::max_element(floorUsage.begin(), floorUsage.end());
    const int chartWidth = 50; // 图表最大宽度
    
    // 显示柱状图
    for (int floor = 14; floor >= 1; --floor) {
        std::cout << std::setw(2) << floor << " |";
        
        // 计算显示的字符数量
        int barLength = maxUsage > 0 
            ? (floorUsage[floor] * chartWidth / maxUsage) 
            : 0;
            
        // 显示柱状图条
        std::cout << std::string(barLength, '#')
                  << " " << floorUsage[floor] << std::endl;
    }
    
    // 显示底部分隔线
    std::cout << "   " << std::string(chartWidth + 10, '-') << std::endl;
}

const std::vector<int>& Statistics::getFloorUsage() const {
    return floorUsage;
} 