from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware
import json
import random
from typing import List, Dict
import asyncio

from models.map import Map
from models.agv import AGV, AGVStatus
from models.task import Task, TaskGenerator, TaskStatus
from algorithms.scheduler import Scheduler

# 创建FastAPI应用
app = FastAPI()

# 允许跨域请求
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 全局变量
map_instance = Map()
scheduler = Scheduler(map_instance)
task_generator = TaskGenerator(map_instance.size)
current_task_id = 0
tasks: List[Task] = []
active_connections: List[WebSocket] = []

def initialize_agvs():
    """初始化15辆AGV到随机有效位置"""
    valid_positions = map_instance.get_valid_positions()
    used_positions = set()
    
    for i in range(15):
        while True:
            pos = random.choice(valid_positions)
            if pos not in used_positions:
                agv = AGV(i, pos)
                scheduler.add_agv(agv)
                used_positions.add(pos)
                break

# 初始化AGV
initialize_agvs()

async def broadcast_state():
    """广播当前状态到所有连接的客户端"""
    if not active_connections:
        return
    
    state = {
        "agvs": [
            {
                "id": agv.id,
                "position": agv.position,
                "status": agv.status.value,
                "path": agv.current_path,
                "path_history": agv.path_history
            }
            for agv in scheduler.agvs
        ],
        "tasks": [
            {
                "id": task.id,
                "pickup": task.pickup,
                "delivery": task.delivery,
                "status": task.status.value,
                "assigned_agv": task.assigned_agv.id if task.assigned_agv else None
            }
            for task in tasks
        ]
    }
    
    for connection in active_connections:
        try:
            await connection.send_json(state)
        except:
            active_connections.remove(connection)

async def update_system():
    """更新系统状态"""
    while True:
        if scheduler.update():
            await broadcast_state()
        await asyncio.sleep(1.0)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket连接处理"""
    await websocket.accept()
    active_connections.append(websocket)
    
    try:
        while True:
            data = await websocket.receive_text()
            if data == "start_task":
                # 生成新任务
                global current_task_id
                new_task = task_generator.generate_task(current_task_id)
                current_task_id += 1
                tasks.append(new_task)
                
                # 分配任务给AGV
                scheduler.assign_task(new_task)
                
                # 广播更新
                await broadcast_state()
    except:
        active_connections.remove(websocket)

@app.on_event("startup")
async def startup_event():
    """启动时开始系统更新循环"""
    asyncio.create_task(update_system())

@app.get("/")
async def root():
    """测试接口"""
    return {"message": "AGV调度系统后端服务正在运行"}

@app.get("/map")
async def get_map():
    """获取地图信息"""
    return {
        "size": map_instance.size,
        "obstacles": map_instance.obstacles,
        "boundary_points": list(map_instance.boundary_points)
    }

@app.get("/agvs")
async def get_agvs():
    """获取所有AGV信息"""
    return [agv.to_dict() for agv in scheduler.agvs]

@app.get("/tasks")
async def get_tasks():
    """获取所有任务信息"""
    return [task.to_dict() for task in tasks]

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000) 