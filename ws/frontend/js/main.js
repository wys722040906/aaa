import MapRenderer from './map.js';
import WebSocketManager from './websocket.js';

class AGVSystem {
    constructor() {
        // 初始化画布
        this.canvas = document.getElementById('mapCanvas');
        if (!this.canvas) {
            throw new Error('找不到地图画布元素');
        }
        
        // 初始化渲染器
        this.mapRenderer = new MapRenderer(this.canvas);
        
        // 初始化WebSocket管理器
        this.wsManager = new WebSocketManager(this.mapRenderer);
        
        // 绑定事件处理
        this.bindEvents();
        
        // 初始状态
        this.taskCount = 0;
        this.maxTasks = 20;
        
        // 初始渲染
        this.mapRenderer.render();
    }
    
    bindEvents() {
        // 开始任务按钮
        const startButton = document.getElementById('startTaskBtn');
        if (startButton) {
            startButton.addEventListener('click', () => this.handleStartTask());
        }
        
        // 窗口大小改变时重新渲染
        window.addEventListener('resize', () => {
            this.adjustCanvasSize();
            this.mapRenderer.render();
        });
        
        // 初始调整画布大小
        this.adjustCanvasSize();
    }
    
    handleStartTask() {
        // 检查任务数量限制
        if (this.taskCount >= this.maxTasks) {
            alert('已完成所有20个任务！');
            return;
        }
        
        // 检查WebSocket连接状态
        if (!this.wsManager.isConnected) {
            alert('系统未连接，请稍后重试');
            return;
        }
        
        // 发送开始任务命令
        this.wsManager.startTask();
        this.taskCount++;
        
        // 更新按钮状态
        this.updateStartButton();
    }
    
    updateStartButton() {
        const startButton = document.getElementById('startTaskBtn');
        if (startButton) {
            startButton.disabled = this.taskCount >= this.maxTasks;
            startButton.textContent = this.taskCount >= this.maxTasks ? 
                '任务已完成' : '开始任务';
        }
    }
    
    adjustCanvasSize() {
        // 获取容器大小
        const container = this.canvas.parentElement;
        if (!container) return;
        
        const containerWidth = container.clientWidth;
        const containerHeight = container.clientHeight;
        
        // 计算合适的网格大小
        const gridCount = 9;  // 9x9的网格
        const padding = 40;   // 两边的padding
        
        // 计算可用空间
        const availableWidth = containerWidth - padding * 2;
        const availableHeight = containerHeight - padding * 2;
        
        // 计算网格大小
        const gridSize = Math.min(
            Math.floor(availableWidth / gridCount),
            Math.floor(availableHeight / gridCount)
        );
        
        // 更新画布大小
        this.canvas.width = gridSize * gridCount + padding * 2;
        this.canvas.height = gridSize * gridCount + padding * 2;
        
        // 更新渲染器的网格大小
        this.mapRenderer.gridSize = gridSize;
        this.mapRenderer.padding = padding;
    }
    
    // 显示错误信息
    showError(message) {
        const errorElement = document.getElementById('errorMessage');
        if (errorElement) {
            errorElement.textContent = message;
            errorElement.style.display = 'block';
            setTimeout(() => {
                errorElement.style.display = 'none';
            }, 3000);
        } else {
            console.error(message);
        }
    }
}

// 当页面加载完成时初始化系统
document.addEventListener('DOMContentLoaded', () => {
    try {
        window.agvSystem = new AGVSystem();
    } catch (error) {
        console.error('系统初始化失败:', error);
        alert('系统初始化失败，请刷新页面重试');
    }
});

// 导出模块
export default AGVSystem; 