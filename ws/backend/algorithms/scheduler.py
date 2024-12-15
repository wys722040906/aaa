from typing import List, Optional, Set, Tuple
from ..models.agv import AGV, AGVStatus
from ..models.task import Task
from ..models.map import Map
from .astar import AStar
import asyncio

class Scheduler:
    def __init__(self, map_instance: Map):
        """
        初始化调度器
        :param map_instance: 地图实例
        """
        self.map = map_instance
        self.astar = AStar(map_instance)
        self.agvs: List[AGV] = []
    
    def add_agv(self, agv: AGV):
        """添加AGV到调度系统"""
        self.agvs.append(agv)
    
    def get_idle_agvs(self) -> List[AGV]:
        """获取所有空闲的AGV"""
        return [agv for agv in self.agvs if agv.is_idle()]
    
    def get_agv_positions(self) -> Set[Tuple[int, int]]:
        """获取所有AGV的当前位置"""
        return {agv.position for agv in self.agvs}
    
    def find_best_agv(self, task: Task) -> Optional[AGV]:
        """
        为任务找到最优的AGV
        :param task: 待分配的任务
        :return: 最优的AGV或None
        """
        best_agv = None
        min_total_cost = float('inf')
        
        # 获取所有空闲的AGV
        idle_agvs = self.get_idle_agvs()
        if not idle_agvs:
            return None
        
        # 获取其他AGV的位置（用于避障）
        other_positions = self.get_agv_positions()
        
        for agv in idle_agvs:
            # 计算到取货点的路径
            path_to_pickup = self.astar.find_path_avoiding_agvs(
                agv.position, task.pickup, other_positions)
            if not path_to_pickup:
                continue
            
            # 计算从取货点到送货点的路径
            path_to_delivery = self.astar.find_path_avoiding_agvs(
                task.pickup, task.delivery, other_positions)
            if not path_to_delivery:
                continue
            
            # 计算总成本
            total_cost = (self.astar.get_path_cost(path_to_pickup) + 
                         self.astar.get_path_cost(path_to_delivery))
            
            # 更新最优AGV
            if total_cost < min_total_cost:
                min_total_cost = total_cost
                best_agv = agv
        
        return best_agv
    
    def assign_task(self, task: Task) -> bool:
        """
        分配任务给最优的AGV
        :param task: 待分配的任务
        :return: 是否成功分配
        """
        # 找到最优AGV
        best_agv = self.find_best_agv(task)
        if not best_agv:
            return False
        
        # 获取其他AGV的位置
        other_positions = self.get_agv_positions()
        
        # 计算完整路径
        path_to_pickup = self.astar.find_path_avoiding_agvs(
            best_agv.position, task.pickup, other_positions)
        path_to_delivery = self.astar.find_path_avoiding_agvs(
            task.pickup, task.delivery, other_positions)
        
        if not path_to_pickup or not path_to_delivery:
            return False
        
        # 设置AGV路径和状态
        complete_path = path_to_pickup + path_to_delivery[1:]  # 避免重复pickup点
        best_agv.set_path(complete_path, task.pickup)  # 先设置取货点为目标
        best_agv.status = AGVStatus.MOVING_TO_PICKUP
        
        # 更新任务状态
        task.assign_agv(best_agv)
        
        return True
    
    def update(self):
        """
        更新所有AGV的状态
        :return: 是否有AGV状态发生变化
        """
        changed = False
        for agv in self.agvs:
            if agv.current_path:
                if agv.move_next():
                    changed = True
        return changed

async def update_agv_positions():
    """更新AGV位置并广播状态"""
    while True:
        for agv in scheduler.agvs:
            if agv.current_path:
                agv.move_next()
        
        await broadcast_state()
        await asyncio.sleep(1.0)  # 确保精确的1秒间隔