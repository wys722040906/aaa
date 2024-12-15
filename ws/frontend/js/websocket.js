class WSClient {
    constructor() {
        this.ws = null;
        this.onStateUpdate = null;
    }

    connect() {
        this.ws = new WebSocket('ws://localhost:8000/ws');
        
        this.ws.onopen = () => {
            console.log('WebSocket连接已建立');
        };
        
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            if (this.onStateUpdate) {
                this.onStateUpdate(data);
            }
        };
        
        this.ws.onclose = () => {
            console.log('WebSocket连接已关闭');
            setTimeout(() => this.connect(), 1000);
        };
    }

    startTask() {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send('start_task');
        }
    }
}

const wsClient = new WSClient(); 