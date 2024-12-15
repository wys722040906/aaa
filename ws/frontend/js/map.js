class MapRenderer {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.gridSize = 50;  // 每个格子的大小
        this.padding = 20;   // 画布边距
        
        // 设置画布大小
        this.canvas.width = this.gridSize * 9 + this.padding * 2;
        this.canvas.height = this.gridSize * 9 + this.padding * 2;
        
        // AGV和任务数据
        this.agvs = [];
        this.currentTask = null;
        
        // 颜色配置
        this.colors = {
            grid: '#000',        // 网格线颜色
            obstacle: '#666',    // 障碍物颜色
            agv: '#4CAF50',     // AGV颜色
            pickup: '#2196F3',   // 取货点颜色
            delivery: '#F44336', // 送货点颜色
            path: '#4CAF50'      // 路径颜色
        };
    }
    
    // 坐标转换：网格坐标转画布坐标
    gridToCanvas(x, y) {
        return {
            x: x * this.gridSize + this.padding,
            y: y * this.gridSize + this.padding
        };
    }
    
    // 清空画布
    clear() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    }
    
    // 绘制网格
    drawGrid() {
        this.ctx.strokeStyle = this.colors.grid;
        this.ctx.lineWidth = 1;
        
        // 绘制垂直线
        for (let i = 0; i <= 9; i++) {
            const x = i * this.gridSize + this.padding;
            this.ctx.beginPath();
            this.ctx.moveTo(x, this.padding);
            this.ctx.lineTo(x, this.canvas.height - this.padding);
            this.ctx.stroke();
        }
        
        // 绘制水平线
        for (let i = 0; i <= 9; i++) {
            const y = i * this.gridSize + this.padding;
            this.ctx.beginPath();
            this.ctx.moveTo(this.padding, y);
            this.ctx.lineTo(this.canvas.width - this.padding, y);
            this.ctx.stroke();
        }
    }
    
    // 绘制障碍物
    drawObstacles() {
        this.ctx.fillStyle = this.colors.obstacle;
        
        // 绘制第一个障碍物 [2,2], [2,4], [6,2], [6,4]
        this.drawPoint(2, 2);
        this.drawPoint(2, 4);
        this.drawPoint(6, 2);
        this.drawPoint(6, 4);
        this.fillRect(3, 2, 3, 2);
        
        // 绘制第二个障碍物 [4,6], [4,8], [8,6], [8,8]
        this.drawPoint(4, 6);
        this.drawPoint(4, 8);
        this.drawPoint(8, 6);
        this.drawPoint(8, 8);
        this.fillRect(5, 6, 3, 2);
    }
    
    // 绘制AGV
    drawAGVs() {
        this.ctx.fillStyle = this.colors.agv;
        
        for (const agv of this.agvs) {
            const pos = this.gridToCanvas(agv.position[0], agv.position[1]);
            
            // 绘制AGV圆点
            this.ctx.beginPath();
            this.ctx.arc(pos.x, pos.y, this.gridSize/3, 0, Math.PI * 2);
            this.ctx.fill();
            
            // 绘制AGV编号
            this.ctx.fillStyle = '#fff';
            this.ctx.textAlign = 'center';
            this.ctx.textBaseline = 'middle';
            this.ctx.font = `${this.gridSize/3}px Arial`;
            this.ctx.fillText(agv.id.toString(), pos.x, pos.y);
            
            // 恢复AGV颜色
            this.ctx.fillStyle = this.colors.agv;
            
            // 绘制路径
            if (agv.path && agv.path.length > 0) {
                this.drawPath(agv.path, this.colors.path);
            }
            
            // 绘制历史路径
            if (agv.path_history && agv.path_history.length > 1) {
                this.drawPath(agv.path_history, '#90CAF9', 0.5);
            }
        }
    }
    
    // 绘制任务点
    drawTaskPoints() {
        if (!this.currentTask) return;
        
        // 绘制取货点
        this.ctx.fillStyle = this.colors.pickup;
        const pickup = this.gridToCanvas(
            this.currentTask.pickup[0], 
            this.currentTask.pickup[1]
        );
        this.ctx.beginPath();
        this.ctx.arc(pickup.x, pickup.y, this.gridSize/4, 0, Math.PI * 2);
        this.ctx.fill();
        
        // 绘制送货点
        this.ctx.fillStyle = this.colors.delivery;
        const delivery = this.gridToCanvas(
            this.currentTask.delivery[0], 
            this.currentTask.delivery[1]
        );
        this.ctx.beginPath();
        this.ctx.arc(delivery.x, delivery.y, this.gridSize/4, 0, Math.PI * 2);
        this.ctx.fill();
    }
    
    // 绘制路径
    drawPath(path, color, alpha = 1) {
        if (!path || path.length < 2) return;
        
        this.ctx.strokeStyle = color;
        this.ctx.lineWidth = 2;
        this.ctx.globalAlpha = alpha;
        
        this.ctx.beginPath();
        const start = this.gridToCanvas(path[0][0], path[0][1]);
        this.ctx.moveTo(start.x, start.y);
        
        for (let i = 1; i < path.length; i++) {
            const point = this.gridToCanvas(path[i][0], path[i][1]);
            this.ctx.lineTo(point.x, point.y);
        }
        
        this.ctx.stroke();
        this.ctx.globalAlpha = 1;
    }
    
    // 绘制点
    drawPoint(x, y) {
        const pos = this.gridToCanvas(x, y);
        this.ctx.beginPath();
        this.ctx.arc(pos.x, pos.y, this.gridSize/4, 0, Math.PI * 2);
        this.ctx.fill();
    }
    
    // 填充矩形
    fillRect(x, y, width, height) {
        const pos = this.gridToCanvas(x, y);
        this.ctx.fillRect(
            pos.x, 
            pos.y, 
            width * this.gridSize, 
            height * this.gridSize
        );
    }
    
    // 更新AGV数据
    updateAGVs(agvs) {
        this.agvs = agvs;
    }
    
    // 更新任务数据
    updateTask(task) {
        this.currentTask = task;
    }
    
    // 渲染整个地图
    render() {
        this.clear();
        this.drawGrid();
        this.drawObstacles();
        this.drawTaskPoints();
        this.drawAGVs();
    }
}

export default MapRenderer; 