from typing import Tuple, List
from enum import Enum

class AGVStatus(Enum):
    """AGV状态枚举"""
    IDLE = "idle"                    # 空闲状态
    MOVING_TO_PICKUP = "to_pickup"   # 正在前往取货点
    MOVING_TO_DELIVERY = "to_delivery"  # 正在前往送货点

class AGV:
    def __init__(self, id: int, position: Tuple[int, int]):
        """
        初始化AGV
        :param id: AGV唯一标识
        :param position: 初始位置坐标 (x, y)
        """
        self.id = id
        self.position = position
        self.status = AGVStatus.IDLE
        
        # 当前任务的路径
        self.current_path: List[Tuple[int, int]] = []
        
        # 当前任务的目标点（取货点或送货点）
        self.target_position: Tuple[int, int] = None
        
        # 记录已经走过的路径点（用于前端显示）
        self.path_history: List[Tuple[int, int]] = []
    
    def set_path(self, path: List[Tuple[int, int]], target: Tuple[int, int]):
        """
        设置AGV的运动路径
        :param path: 路径点列表
        :param target: 目标点（取货点或送货点）
        """
        self.current_path = path
        self.target_position = target
        self.path_history = [self.position]  # 记录起始点
    
    def move_next(self) -> bool:
        """
        移动到路径中的下一个位置
        :return: 是否成功移动
        """
        if not self.current_path:
            return False
        
        # 获取下一个位置
        next_position = self.current_path.pop(0)
        
        # 验证移动的合法性（相邻点之间移动）
        if not self._is_valid_move(self.position, next_position):
            return False
        
        # 更新位置
        self.position = next_position
        self.path_history.append(next_position)
        
        # 检查是否到达目标点
        if self.position == self.target_position:
            if self.status == AGVStatus.MOVING_TO_PICKUP:
                self.status = AGVStatus.MOVING_TO_DELIVERY
            elif self.status == AGVStatus.MOVING_TO_DELIVERY:
                self.status = AGVStatus.IDLE
                self.target_position = None
        
        return True
    
    def _is_valid_move(self, current: Tuple[int, int], next_pos: Tuple[int, int]) -> bool:
        """
        验证移动是否合法（只能在相邻点之间移动）
        :param current: 当前位置
        :param next_pos: 下一个位置
        :return: 移动是否合法
        """
        x1, y1 = current
        x2, y2 = next_pos
        return abs(x1 - x2) + abs(y1 - y2) == 1
    
    def get_distance_to(self, target: Tuple[int, int]) -> int:
        """
        计算到目标点的曼哈顿距离
        :param target: 目标位置
        :return: 距离值
        """
        return abs(self.position[0] - target[0]) + abs(self.position[1] - target[1])
    
    def is_idle(self) -> bool:
        """检查AGV是否空闲"""
        return self.status == AGVStatus.IDLE
    
    def to_dict(self) -> dict:
        """
        将AGV信息转换为字典（用于JSON序列化）
        :return: 包含AGV信息的字典
        """
        return {
            "id": self.id,
            "position": self.position,
            "status": self.status.value,
            "current_path": self.current_path,
            "target_position": self.target_position,
            "path_history": self.path_history
        }