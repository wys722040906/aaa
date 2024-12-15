from typing import Tuple, List, Optional
import random
from enum import Enum
from .agv import AGV

class TaskStatus(Enum):
    """任务状态枚举"""
    PENDING = "pending"          # 等待分配
    IN_PROGRESS = "in_progress"  # 正在执行
    COMPLETED = "completed"      # 已完成

class Task:
    def __init__(self, id: int, pickup: Tuple[int, int], delivery: Tuple[int, int]):
        """
        初始化任务
        :param id: 任务ID
        :param pickup: 取货点坐标
        :param delivery: 送货点坐标
        """
        self.id = id
        self.pickup = pickup
        self.delivery = delivery
        self.status = TaskStatus.PENDING
        self.assigned_agv: Optional[AGV] = None
        self.start_time: float = None
        self.complete_time: float = None
    
    def assign_agv(self, agv: AGV):
        """
        分配AGV给任务
        :param agv: 被分配的AGV
        """
        self.assigned_agv = agv
        self.status = TaskStatus.IN_PROGRESS
    
    def complete(self):
        """标记任务为完成状态"""
        self.status = TaskStatus.COMPLETED
        if self.assigned_agv:
            self.assigned_agv.status = "idle"
    
    def to_dict(self) -> dict:
        """
        将任务信息转换为字典（用于JSON序列化）
        :return: 包含任务信息的字典
        """
        return {
            "id": self.id,
            "pickup": self.pickup,
            "delivery": self.delivery,
            "status": self.status.value,
            "assigned_agv": self.assigned_agv.id if self.assigned_agv else None
        }

class TaskGenerator:
    def __init__(self, map_size: int):
        """
        初始化任务生成器
        :param map_size: 地图大小
        """
        self.map_size = map_size
        # 障碍物内部区域（不能作为任务点）
        self.obstacles = [
            # 第一个矩形障碍物内部区域
            [(x, y) for x in range(3, 6) for y in range(2, 4)],
            # 第二个矩形障碍物内部区域
            [(x, y) for x in range(5, 8) for y in range(6, 8)]
        ]
        # 障碍物边界点（可以作为任务点）
        self.boundary_points = {
            (2, 2), (2, 4), (6, 2), (6, 4),  # 第一个矩形的边界点
            (4, 6), (4, 8), (8, 6), (8, 8)   # 第二个矩形的边界点
        }
    
    def _is_valid_point(self, point: Tuple[int, int]) -> bool:
        """
        检查点是否是有效的任务点
        :param point: 待检查的点
        :return: 是否有效
        """
        x, y = point
        # 检查是否在地图范围内
        if not (0 <= x < self.map_size and 0 <= y < self.map_size):
            return False
        
        # 边界点是有效的
        if point in self.boundary_points:
            return True
        
        # 检查是否在障碍物内部
        for obstacle in self.obstacles:
            if point in obstacle:
                return False
        
        return True
    
    def generate_task(self, task_id: int) -> Task:
        """
        生成一个新任务
        :param task_id: 任务ID
        :return: 生成的任务
        """
        while True:
            # 随机生成取货点和送货点
            pickup = (random.randint(0, self.map_size-1), 
                     random.randint(0, self.map_size-1))
            delivery = (random.randint(0, self.map_size-1), 
                       random.randint(0, self.map_size-1))
            
            # 确保取货点和送货点都有效，且不相同
            if (self._is_valid_point(pickup) and 
                self._is_valid_point(delivery) and 
                pickup != delivery):
                return Task(task_id, pickup, delivery)