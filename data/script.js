// P-touch ESP32 Web Interface JavaScript
class PtouchInterface {
    constructor() {
        this.websocket = null;
        this.printerConnected = false;
        this.currentTool = 'text';
        this.isDrawing = false;
        this.printQueue = [];
        this.canvasHistory = [];
        
        this.initializeWebSocket();
        this.initializeEventListeners();
        this.initializeCanvas();
        this.refreshStatus();
    }
    
    // WebSocket Management
    initializeWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        this.websocket = new WebSocket(wsUrl);
        
        this.websocket.onopen = () => {
            this.showToast('Connected to printer server', 'success');
            this.updateConnectionStatus('Connected');
        };
        
        this.websocket.onmessage = (event) => {
            const data = JSON.parse(event.data);
            this.handleWebSocketMessage(data);
        };
        
        this.websocket.onclose = () => {
            this.showToast('Connection to server lost', 'error');
            this.updateConnectionStatus('Disconnected');
            // Attempt to reconnect after 5 seconds
            setTimeout(() => this.initializeWebSocket(), 5000);
        };
        
        this.websocket.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.showToast('Connection error', 'error');
        };
    }
    
    handleWebSocketMessage(data) {
        switch (data.type) {
            case 'printerStatus':
                this.updatePrinterStatus(data);
                break;
            case 'printResult':
                this.handlePrintResult(data);
                break;
            default:
                console.log('Unknown message type:', data.type);
        }
    }
    
    sendWebSocketMessage(message) {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.websocket.send(JSON.stringify(message));
        }
    }
    
    // UI Event Listeners
    initializeEventListeners() {
        // Tab switching
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                this.switchTab(e.target.dataset.tab);
            });
        });
        
        // Text input and preview
        document.getElementById('textInput').addEventListener('input', () => {
            this.updateTextPreview();
        });
        
        document.getElementById('fontSize').addEventListener('change', () => {
            this.updateTextPreview();
        });
        
        // Print buttons
        document.getElementById('printTextBtn').addEventListener('click', () => {
            this.printText();
        });
        
        document.getElementById('printDesignBtn').addEventListener('click', () => {
            this.printDesign();
        });
        
        // Status and control buttons
        document.getElementById('reconnectBtn').addEventListener('click', () => {
            this.reconnectPrinter();
        });
        
        document.getElementById('refreshBtn').addEventListener('click', () => {
            this.refreshStatus();
        });
        
        // Design tools
        document.querySelectorAll('.tool-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                this.selectTool(e.target.dataset.tool);
            });
        });
        
        // Canvas size controls
        document.getElementById('canvasWidth').addEventListener('change', () => {
            this.updateCanvasSize();
        });
        
        document.getElementById('canvasHeight').addEventListener('change', () => {
            this.updateCanvasSize();
        });
        
        // Canvas controls
        document.getElementById('clearCanvas').addEventListener('click', () => {
            this.clearCanvas();
        });
        
        document.getElementById('undoBtn').addEventListener('click', () => {
            this.undoCanvas();
        });
        
        // Queue controls
        document.getElementById('clearQueueBtn').addEventListener('click', () => {
            this.clearQueue();
        });
        
        document.getElementById('printQueueBtn').addEventListener('click', () => {
            this.printQueue();
        });
    }
    
    // Canvas Management
    initializeCanvas() {
        this.textPreviewCanvas = document.getElementById('textPreview');
        this.textPreviewCtx = this.textPreviewCanvas.getContext('2d');
        
        this.designCanvas = document.getElementById('designCanvas');
        this.designCtx = this.designCanvas.getContext('2d');
        
        this.setupCanvasEvents();
        this.updateTextPreview();
    }
    
    setupCanvasEvents() {
        this.designCanvas.addEventListener('mousedown', (e) => {
            this.startDrawing(e);
        });
        
        this.designCanvas.addEventListener('mousemove', (e) => {
            this.draw(e);
        });
        
        this.designCanvas.addEventListener('mouseup', () => {
            this.stopDrawing();
        });
        
        this.designCanvas.addEventListener('mouseleave', () => {
            this.stopDrawing();
        });
        
        // Touch events for mobile
        this.designCanvas.addEventListener('touchstart', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            const mouseEvent = new MouseEvent('mousedown', {
                clientX: touch.clientX,
                clientY: touch.clientY
            });
            this.designCanvas.dispatchEvent(mouseEvent);
        });
        
        this.designCanvas.addEventListener('touchmove', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            const mouseEvent = new MouseEvent('mousemove', {
                clientX: touch.clientX,
                clientY: touch.clientY
            });
            this.designCanvas.dispatchEvent(mouseEvent);
        });
        
        this.designCanvas.addEventListener('touchend', (e) => {
            e.preventDefault();
            const mouseEvent = new MouseEvent('mouseup', {});
            this.designCanvas.dispatchEvent(mouseEvent);
        });
    }
    
    getCanvasCoordinates(e) {
        const rect = this.designCanvas.getBoundingClientRect();
        return {
            x: e.clientX - rect.left,
            y: e.clientY - rect.top
        };
    }
    
    saveCanvasState() {
        this.canvasHistory.push(this.designCanvas.toDataURL());
        if (this.canvasHistory.length > 10) {
            this.canvasHistory.shift();
        }
    }
    
    startDrawing(e) {
        if (this.currentTool === 'text') {
            this.addTextToCanvas(e);
            return;
        }
        
        this.isDrawing = true;
        this.saveCanvasState();
        
        const coords = this.getCanvasCoordinates(e);
        this.lastX = coords.x;
        this.lastY = coords.y;
        
        this.designCtx.beginPath();
        this.designCtx.moveTo(coords.x, coords.y);
    }
    
    draw(e) {
        if (!this.isDrawing) return;
        
        const coords = this.getCanvasCoordinates(e);
        this.designCtx.lineWidth = 2;
        this.designCtx.lineCap = 'round';
        this.designCtx.strokeStyle = '#000';
        
        switch (this.currentTool) {
            case 'line':
                this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
                this.restoreCanvasState();
                this.designCtx.beginPath();
                this.designCtx.moveTo(this.lastX, this.lastY);
                this.designCtx.lineTo(coords.x, coords.y);
                this.designCtx.stroke();
                break;
            case 'rect':
                this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
                this.restoreCanvasState();
                this.designCtx.beginPath();
                this.designCtx.rect(this.lastX, this.lastY, coords.x - this.lastX, coords.y - this.lastY);
                this.designCtx.stroke();
                break;
            case 'circle':
                this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
                this.restoreCanvasState();
                const radius = Math.sqrt(Math.pow(coords.x - this.lastX, 2) + Math.pow(coords.y - this.lastY, 2));
                this.designCtx.beginPath();
                this.designCtx.arc(this.lastX, this.lastY, radius, 0, 2 * Math.PI);
                this.designCtx.stroke();
                break;
        }
    }
    
    stopDrawing() {
        this.isDrawing = false;
    }
    
    addTextToCanvas(e) {
        const text = prompt('Enter text to add:');
        if (text) {
            const coords = this.getCanvasCoordinates(e);
            this.saveCanvasState();
            this.designCtx.font = '16px Arial';
            this.designCtx.fillStyle = '#000';
            this.designCtx.fillText(text, coords.x, coords.y);
        }
    }
    
    restoreCanvasState() {
        if (this.canvasHistory.length > 0) {
            const img = new Image();
            img.onload = () => {
                this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
                this.designCtx.drawImage(img, 0, 0);
            };
            img.src = this.canvasHistory[this.canvasHistory.length - 1];
        }
    }
    
    // UI Control Functions
    switchTab(tabName) {
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.classList.remove('active');
        });
        document.querySelectorAll('.tab-content').forEach(content => {
            content.classList.remove('active');
        });
        
        document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
        document.getElementById(`${tabName}Tab`).classList.add('active');
    }
    
    selectTool(tool) {
        this.currentTool = tool;
        document.querySelectorAll('.tool-btn').forEach(btn => {
            btn.classList.remove('active');
        });
        document.querySelector(`[data-tool="${tool}"]`).classList.add('active');
    }
    
    updateTextPreview() {
        const text = document.getElementById('textInput').value;
        const fontSize = document.getElementById('fontSize').value;
        
        this.textPreviewCtx.clearRect(0, 0, this.textPreviewCanvas.width, this.textPreviewCanvas.height);
        
        if (text) {
            this.textPreviewCtx.font = `${fontSize}px Arial`;
            this.textPreviewCtx.fillStyle = '#000';
            this.textPreviewCtx.textAlign = 'center';
            this.textPreviewCtx.fillText(text, this.textPreviewCanvas.width / 2, this.textPreviewCanvas.height / 2);
        }
    }
    
    updateCanvasSize() {
        const width = parseInt(document.getElementById('canvasWidth').value);
        const height = parseInt(document.getElementById('canvasHeight').value);
        
        this.designCanvas.width = width;
        this.designCanvas.height = height;
        this.designCtx.fillStyle = '#fff';
        this.designCtx.fillRect(0, 0, width, height);
    }
    
    clearCanvas() {
        this.saveCanvasState();
        this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
        this.designCtx.fillStyle = '#fff';
        this.designCtx.fillRect(0, 0, this.designCanvas.width, this.designCanvas.height);
    }
    
    undoCanvas() {
        if (this.canvasHistory.length > 0) {
            const previousState = this.canvasHistory.pop();
            const img = new Image();
            img.onload = () => {
                this.designCtx.clearRect(0, 0, this.designCanvas.width, this.designCanvas.height);
                this.designCtx.drawImage(img, 0, 0);
            };
            img.src = previousState;
        }
    }
    
    // Printer Functions
    printText() {
        const text = document.getElementById('textInput').value.trim();
        if (!text) {
            this.showToast('Please enter text to print', 'warning');
            return;
        }
        
        if (!this.printerConnected) {
            this.showToast('Printer not connected', 'error');
            return;
        }
        
        this.showLoading(true);
        
        fetch('/api/print/text', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: `text=${encodeURIComponent(text)}`
        })
        .then(response => response.text())
        .then(data => {
            this.showLoading(false);
            if (data.includes('successfully')) {
                this.showToast('Print job sent successfully', 'success');
                this.addToQueue('text', text);
            } else {
                this.showToast('Print job failed: ' + data, 'error');
            }
        })
        .catch(error => {
            this.showLoading(false);
            this.showToast('Error: ' + error.message, 'error');
        });
    }
    
    printDesign() {
        if (!this.printerConnected) {
            this.showToast('Printer not connected', 'error');
            return;
        }
        
        // Convert canvas to image data
        const imageData = this.designCanvas.toDataURL('image/png');
        
        this.showLoading(true);
        
        fetch('/api/print/image', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: `image=${encodeURIComponent(imageData)}&width=${this.designCanvas.width}&height=${this.designCanvas.height}`
        })
        .then(response => response.text())
        .then(data => {
            this.showLoading(false);
            if (data.includes('successfully')) {
                this.showToast('Print job sent successfully', 'success');
                this.addToQueue('design', 'Custom design');
            } else {
                this.showToast('Print job failed: ' + data, 'error');
            }
        })
        .catch(error => {
            this.showLoading(false);
            this.showToast('Error: ' + error.message, 'error');
        });
    }
    
    reconnectPrinter() {
        this.showLoading(true);
        
        fetch('/api/reconnect', {
            method: 'POST'
        })
        .then(response => response.text())
        .then(data => {
            this.showLoading(false);
            this.showToast('Reconnection attempt completed', 'info');
        })
        .catch(error => {
            this.showLoading(false);
            this.showToast('Error: ' + error.message, 'error');
        });
    }
    
    refreshStatus() {
        fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            this.updatePrinterStatus(data);
        })
        .catch(error => {
            console.error('Error fetching status:', error);
        });
    }
    
    updatePrinterStatus(data) {
        this.printerConnected = data.connected;
        
        const statusDot = document.getElementById('statusDot');
        const statusText = document.getElementById('statusText');
        const printerModel = document.getElementById('printerModel');
        const tapeInfo = document.getElementById('tapeInfo');
        const sizeInfo = document.getElementById('sizeInfo');
        
        if (data.connected) {
            statusDot.className = 'status-dot connected';
            statusText.textContent = 'Connected';
            printerModel.textContent = data.name;
            tapeInfo.textContent = `${data.tapeColor} (${data.mediaType})`;
            sizeInfo.textContent = `${data.tapeWidth}px x ${data.maxWidth}px`;
        } else {
            statusDot.className = 'status-dot disconnected';
            statusText.textContent = data.status || 'Disconnected';
            printerModel.textContent = 'Unknown';
            tapeInfo.textContent = 'Unknown';
            sizeInfo.textContent = 'Unknown';
        }
        
        // Update print buttons
        const printButtons = document.querySelectorAll('#printTextBtn, #printDesignBtn');
        printButtons.forEach(btn => {
            btn.disabled = !data.connected;
        });
    }
    
    handlePrintResult(data) {
        if (data.success) {
            this.showToast('Print completed successfully', 'success');
        } else {
            this.showToast('Print failed', 'error');
        }
    }
    
    // Queue Management
    addToQueue(type, content) {
        this.printQueue.push({
            type: type,
            content: content,
            timestamp: new Date().toLocaleTimeString()
        });
        this.updateQueueDisplay();
    }
    
    updateQueueDisplay() {
        const queueList = document.getElementById('queueList');
        
        if (this.printQueue.length === 0) {
            queueList.innerHTML = '<div class="queue-empty">No print jobs in queue</div>';
            return;
        }
        
        queueList.innerHTML = this.printQueue.map((job, index) => `
            <div class="queue-item">
                <span>${job.type}: ${job.content}</span>
                <span>${job.timestamp}</span>
            </div>
        `).join('');
    }
    
    clearQueue() {
        this.printQueue = [];
        this.updateQueueDisplay();
        this.showToast('Queue cleared', 'info');
    }
    
    printQueue() {
        if (this.printQueue.length === 0) {
            this.showToast('Queue is empty', 'warning');
            return;
        }
        
        this.showToast('Printing queue not implemented yet', 'info');
    }
    
    // Utility Functions
    showLoading(show) {
        const overlay = document.getElementById('loadingOverlay');
        if (show) {
            overlay.classList.add('show');
        } else {
            overlay.classList.remove('show');
        }
    }
    
    showToast(message, type = 'info') {
        const container = document.getElementById('toastContainer');
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        toast.textContent = message;
        
        container.appendChild(toast);
        
        // Trigger animation
        setTimeout(() => {
            toast.classList.add('show');
        }, 100);
        
        // Auto remove after 5 seconds
        setTimeout(() => {
            toast.classList.remove('show');
            setTimeout(() => {
                container.removeChild(toast);
            }, 300);
        }, 5000);
    }
    
    updateConnectionStatus(status) {
        document.getElementById('connectionStatus').textContent = status;
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new PtouchInterface();
}); 