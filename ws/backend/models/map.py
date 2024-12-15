import numpy as np
from typing import List, Tuple, Set

class Map:
    def __init__(self):
        """初始化9x9的网格地图"""
        self.size = 9  # 9x9的网格
        self.grid = np.zeros((self.size, self.size), dtype=int)
        
        # 定义障碍物区域
        # 第一个障碍物：[2,2], [2,4], [6,2], [6,4]
        # 第二个障碍物：[4,6], [4,8], [8,6], [8,8]
        self.obstacles = [
            # 第一个矩形障碍物内部区域
            [(x, y) for x in range(3, 6) for y in range(2, 4)],
            # 第二个矩形障碍物内部区域
            [(x, y) for x in range(5, 8) for y in range(6, 8)]
        ]
        
        # 障碍物边界点（这些点是可以停留的）
        self.boundary_points = {
            (2, 2), (2, 4), (6, 2), (6, 4),  # 第一个矩形的边界点
            (4, 6), (4, 8), (8, 6), (8, 8)   # 第二个矩形的边界点
        }
        
        # 将障碍物标记在网格上
        self._mark_obstacles()
    
    def _mark_obstacles(self):
        """在网格上标记障碍物"""
        for obstacle_group in self.obstacles:
            for x, y in obstacle_group:
                self.grid[x][y] = 1
    
    def is_valid_position(self, x: int, y: int) -> bool:
        """
        检查位置是否有效
        - 在地图范围内
        - 不在障碍物内部
        - 或者是障碍物的边界点
        """
        # 检查是否在地图范围内
        if not (0 <= x < self.size and 0 <= y < self.size):
            return False
            
        # 检查是否是障碍物边界点
        if (x, y) in self.boundary_points:
            return True
            
        # 检查是否在障碍物内部
        return self.grid[x][y] == 0
    
    def get_neighbors(self, x: int, y: int) -> List[Tuple[int, int]]:
        """获取某个点的相邻可达节点"""
        neighbors = []
        # 四个方向：右、下、左、上
        directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]
        
        for dx, dy in directions:
            new_x, new_y = x + dx, y + dy
            if self.is_valid_position(new_x, new_y):
                neighbors.append((new_x, new_y))
        
        return neighbors
    
    def is_path_clear(self, start: Tuple[int, int], end: Tuple[int, int]) -> bool:
        """检查两点之间的直线路径是否可行（没有障碍物）"""
        x1, y1 = start
        x2, y2 = end
        
        # 如果是相邻点，直接检查终点的有效性
        if abs(x1 - x2) + abs(y1 - y2) == 1:
            return self.is_valid_position(x2, y2)
            
        return False  # 非相邻点需要通过A*算法寻路
    
    def get_valid_positions(self) -> List[Tuple[int, int]]:
        """获取所有有效的位置（用于随机生成AGV初始位置）"""
        valid_positions = []
        for x in range(self.size):
            for y in range(self.size):
                if self.is_valid_position(x, y):
                    valid_positions.append((x, y))
        return valid_positions