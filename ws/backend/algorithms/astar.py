from typing import List, Tuple, Dict, Set
import heapq
from ..models.map import Map

class AStar:
    def __init__(self, map_instance: Map):
        """
        初始化A*算法
        :param map_instance: 地图实例
        """
        self.map = map_instance
    
    def heuristic(self, a: Tuple[int, int], b: Tuple[int, int]) -> int:
        """
        启发式函数：使用曼哈顿距离
        :param a: 起点坐标
        :param b: 终点坐标
        :return: 估计距离
        """
        return abs(a[0] - b[0]) + abs(a[1] - b[1])
    
    def get_path_cost(self, path: List[Tuple[int, int]]) -> int:
        """
        计算路径总成本
        :param path: 路径点列表
        :return: 路径总长度
        """
        if not path:
            return float('inf')
        return len(path) - 1  # 路径长度等于点数减1
    
    def find_path(self, start: Tuple[int, int], goal: Tuple[int, int]) -> List[Tuple[int, int]]:
        """
        A*寻路算法实现
        :param start: 起点坐标
        :param goal: 终点坐标
        :return: 路径点列表，如果没有找到路径则返回空列表
        """
        # 验证起点和终点的有效性
        if not (self.map.is_valid_position(*start) and self.map.is_valid_position(*goal)):
            return []
        
        # 初始化开放列表和关闭列表
        frontier = []  # 优先队列，存储(优先级, 坐标)
        heapq.heappush(frontier, (0, start))
        
        came_from: Dict[Tuple[int, int], Tuple[int, int]] = {start: None}  # 记录路径
        cost_so_far: Dict[Tuple[int, int], int] = {start: 0}  # 记录到达每个点的实际成本
        
        while frontier:
            current = heapq.heappop(frontier)[1]
            
            # 到达目标点
            if current == goal:
                break
            
            # 遍历相邻节点
            for next_pos in self.map.get_neighbors(*current):
                # 计算新路径的成本
                new_cost = cost_so_far[current] + 1  # 相邻点之间距离为1
                
                # 如果找到更优路径或是新节点
                if next_pos not in cost_so_far or new_cost < cost_so_far[next_pos]:
                    cost_so_far[next_pos] = new_cost
                    # 计算优先级（f = g + h）
                    priority = new_cost + self.heuristic(goal, next_pos)
                    heapq.heappush(frontier, (priority, next_pos))
                    came_from[next_pos] = current
        
        # 如果没有找到路径
        if goal not in came_from:
            return []
        
        # 重建路径
        path = []
        current = goal
        while current is not None:
            path.append(current)
            current = came_from.get(current)
        
        # 返回反转后的路径（从起点到终点）
        return path[::-1]
    
    def find_path_avoiding_agvs(self, start: Tuple[int, int], goal: Tuple[int, int], 
                               agv_positions: Set[Tuple[int, int]]) -> List[Tuple[int, int]]:
        """
        考虑其他AGV位置的路径规划
        :param start: 起点坐标
        :param goal: 终点坐标
        :param agv_positions: 其他AGV的当前位置集合
        :return: 路径点列表
        """
        # 创建临时地图副本
        temp_map = Map()
        temp_map.size = self.map.size
        temp_map.grid = self.map.grid.copy()
        
        # 将其他AGV的位置标记为临时障碍
        for pos in agv_positions:
            if pos != start and pos != goal:  # 不标记起点和终点
                x, y = pos
                temp_map.grid[x][y] = 1
        
        # 使用临时地图进行路径规划
        original_map = self.map
        self.map = temp_map
        path = self.find_path(start, goal)
        self.map = original_map
        
        return path