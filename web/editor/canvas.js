// Canvas drawing and interaction
export class CanvasRenderer {
    constructor(canvas, entityManager) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.entityManager = entityManager;
        this.scale = 16;
        this.mode = 'draw';
        this.snapEnabled = true;
        this.snapSize = 0.5;

        // Viewport state
        this.viewport = {
            x: 0,  // World coordinates of top-left corner
            y: 0,
            zoom: 1.0
        };

        // Drawing state
        this.isDrawing = false;
        this.drawStart = { x: 0, y: 0 };

        // Moving state
        this.isMoving = false;
        this.moveOffset = { x: 0, y: 0 };

        // Resizing state
        this.isResizing = false;
        this.resizeDir = null;
        this.resizeStart = { x: 0, y: 0 };
        this.origRect = null;

        // Multi-select state
        this.multiSelectMode = false;
        this.isSelecting = false;
        this.selectStart = { x: 0, y: 0 };
        this.selectedEntities = new Set();

        // Panning state
        this.isPanning = false;
        this.panStart = { x: 0, y: 0 };
        this.panStartViewport = { x: 0, y: 0 };

        this.setupEvents();
    }

    setupEvents() {
        this.canvas.addEventListener('mousedown', e => this.onMouseDown(e));
        this.canvas.addEventListener('mousemove', e => this.onMouseMove(e));
        this.canvas.addEventListener('mouseup', e => this.onMouseUp(e));
        this.canvas.addEventListener('wheel', e => this.onWheel(e));

        // Keyboard events for multi-select and panning
        document.addEventListener('keydown', e => {
            if (e.key === 'Shift' && !e.repeat) {
                this.multiSelectMode = true;
                this.canvas.style.cursor = 'crosshair';
            } else if (e.key === 'Alt' && !e.repeat) {
                this.canvas.style.cursor = 'move';
                e.preventDefault(); // Prevent browser menu
            }
        });
        document.addEventListener('keyup', e => {
            if (e.key === 'Shift') {
                this.multiSelectMode = false;
                this.canvas.style.cursor = this.mode === 'draw' ? 'crosshair' : 'default';
                this.isSelecting = false;
                this.render();
            } else if (e.key === 'Alt') {
                this.canvas.style.cursor = this.mode === 'draw' ? 'crosshair' : 'default';
                this.isPanning = false;
            }
        });
    }

    setMode(mode) {
        this.mode = mode;
        this.canvas.style.cursor = mode === 'draw' ? 'crosshair' : 'default';
    }

    setSnap(enabled, size = 0.5) {
        this.snapEnabled = enabled;
        this.snapSize = size;
    }

    snap(val) {
        if (!this.snapEnabled) return val;
        return Math.round(val / this.snapSize) * this.snapSize;
    }

    snapPx(val) {
        if (!this.snapEnabled) return val;
        return Math.round(val / (this.scale * this.snapSize)) * (this.scale * this.snapSize);
    }

    getMousePos(evt) {
        const rect = this.canvas.getBoundingClientRect();
        const canvasX = (evt.clientX - rect.left) * (this.canvas.width / rect.width);
        const canvasY = (evt.clientY - rect.top) * (this.canvas.height / rect.height);
        return { x: canvasX, y: canvasY };
    }

    // Convert canvas coordinates to world coordinates
    canvasToWorld(canvasX, canvasY) {
        const effectiveScale = this.scale * this.viewport.zoom;
        return {
            x: this.viewport.x + canvasX / effectiveScale,
            y: this.viewport.y + canvasY / effectiveScale
        };
    }

    // Convert world coordinates to canvas coordinates
    worldToCanvas(worldX, worldY) {
        const effectiveScale = this.scale * this.viewport.zoom;
        return {
            x: (worldX - this.viewport.x) * effectiveScale,
            y: (worldY - this.viewport.y) * effectiveScale
        };
    }

    // Get center of viewport in world coordinates
    getViewCenter() {
        return {
            x: this.viewport.x + (this.canvas.width / (this.scale * this.viewport.zoom)) / 2,
            y: this.viewport.y + (this.canvas.height / (this.scale * this.viewport.zoom)) / 2
        };
    }

    // Set zoom level
    setZoom(zoom, centerX = null, centerY = null) {
        const oldZoom = this.viewport.zoom;
        this.viewport.zoom = Math.max(0.25, Math.min(4, zoom));

        // Adjust viewport to keep the zoom centered
        if (centerX !== null && centerY !== null) {
            const scale = this.viewport.zoom / oldZoom;
            const worldX = this.viewport.x + centerX / (this.scale * oldZoom);
            const worldY = this.viewport.y + centerY / (this.scale * oldZoom);

            this.viewport.x = worldX - centerX / (this.scale * this.viewport.zoom);
            this.viewport.y = worldY - centerY / (this.scale * this.viewport.zoom);
        }

        this.render();
    }

    // Handle mouse wheel for zooming
    onWheel(evt) {
        evt.preventDefault();
        const mouse = this.getMousePos(evt);
        const delta = evt.deltaY < 0 ? 1.1 : 0.9;
        this.setZoom(this.viewport.zoom * delta, mouse.x, mouse.y);
    }

    // Get currently selected entities (for multi-select)
    getSelectedEntities() {
        return Array.from(this.selectedEntities).map(idx => this.entityManager.entities[idx]);
    }

    // Find entity at world coordinates
    findEntityAt(worldX, worldY) {
        for (let i = this.entityManager.entities.length - 1; i >= 0; i--) {
            const e = this.entityManager.entities[i];
            const pos = e.components.find(c => c.type === "Position");
            const shape = e.components.find(c => c.type === "Shape");

            if (pos && shape) {
                if (worldX >= pos.x && worldX <= pos.x + shape.w &&
                    worldY >= pos.y && worldY <= pos.y + shape.h) {
                    return i;
                }
            }
        }
        return -1;
    }

    // Clear multi-selection
    clearMultiSelection() {
        this.selectedEntities.clear();
        this.render();
    }

    // Update multi-selection based on selection box
    updateMultiSelection(selBox) {
        this.selectedEntities.clear();

        this.entityManager.entities.forEach((entity, idx) => {
            const pos = entity.components.find(c => c.type === "Position");
            const shape = entity.components.find(c => c.type === "Shape");

            if (!pos || !shape) return;

            // Check if entity is within selection box
            const entityBox = {
                x: pos.x * this.scale,
                y: pos.y * this.scale,
                w: shape.w * this.scale,
                h: shape.h * this.scale
            };

            if (this.boxesIntersect(selBox, entityBox)) {
                this.selectedEntities.add(idx);
            }
        });
    }

    // Check if two boxes intersect
    boxesIntersect(box1, box2) {
        return !(box1.x > box2.x + box2.w ||
                box1.x + box1.w < box2.x ||
                box1.y > box2.y + box2.h ||
                box1.y + box1.h < box2.y);
    }

    getResizeHandle(entity, mx, my) {
        const pos = entity.components.find(c => c.type === "Position");
        const shape = entity.components.find(c => c.type === "Shape");
        if (!pos || !shape) return null;

        const handles = [
            { name: "nw", x: pos.x * this.scale, y: pos.y * this.scale },
            { name: "ne", x: pos.x * this.scale + shape.w * this.scale, y: pos.y * this.scale },
            { name: "sw", x: pos.x * this.scale, y: pos.y * this.scale + shape.h * this.scale },
            { name: "se", x: pos.x * this.scale + shape.w * this.scale, y: pos.y * this.scale + shape.h * this.scale }
        ];

        for (let h of handles) {
            if (Math.abs(mx - h.x) < 10 && Math.abs(my - h.y) < 10) return h.name;
        }
        return null;
    }

    onMouseDown(evt) {
        const mouse = this.getMousePos(evt);
        const world = this.canvasToWorld(mouse.x, mouse.y);

        // Panning mode with Alt
        if (evt.altKey) {
            this.isPanning = true;
            this.panStart = mouse;
            this.panStartViewport = { x: this.viewport.x, y: this.viewport.y };
            this.canvas.style.cursor = 'move';
            return;
        }

        // Multi-select mode with shift
        if (this.multiSelectMode) {
            this.isSelecting = true;
            this.selectStart = world;
            if (!evt.ctrlKey) {
                this.selectedEntities.clear();
            }
            return;
        }

        if (this.mode === 'draw') {
            this.isDrawing = true;
            this.drawStart = world;
        } else {
            const idx = this.findEntityAt(world.x, world.y);
            if (idx >= 0) {
                this.entityManager.select(idx);
                const entity = this.entityManager.entities[idx];
                const pos = entity.components.find(c => c.type === "Position");
                const shape = entity.components.find(c => c.type === "Shape");

                if (this.mode === 'edit') {
                    const handle = this.getResizeHandle(entity, mouse.x, mouse.y);
                    if (handle) {
                        this.isResizing = true;
                        this.resizeDir = handle;
                        this.resizeStart = mouse;
                        this.origRect = { x: pos.x, y: pos.y, w: shape.w, h: shape.h };
                    } else {
                        this.isMoving = true;
                        this.moveOffset.x = mouse.x - pos.x * this.scale;
                        this.moveOffset.y = mouse.y - pos.y * this.scale;
                    }
                } else if (this.mode === 'move') {
                    this.isMoving = true;
                    this.moveOffset.x = mouse.x - pos.x * this.scale;
                    this.moveOffset.y = mouse.y - pos.y * this.scale;
                }

                if (this.onSelectionChange) this.onSelectionChange();
            } else {
                this.entityManager.select(null);
                if (this.onSelectionChange) this.onSelectionChange();
            }
        }
    }

    onMouseMove(evt) {
        const mouse = this.getMousePos(evt);
        const world = this.canvasToWorld(mouse.x, mouse.y);

        // Handle panning
        if (this.isPanning) {
            const dx = (mouse.x - this.panStart.x) / (this.scale * this.viewport.zoom);
            const dy = (mouse.y - this.panStart.y) / (this.scale * this.viewport.zoom);
            this.viewport.x = this.panStartViewport.x - dx;
            this.viewport.y = this.panStartViewport.y - dy;
            this.render();
            return;
        }

        // Update selection box
        if (this.isSelecting) {
            this.render();
            return;
        }

        if (this.mode === 'draw' && this.isDrawing) {
            this.render();
            // Draw preview rectangle in world coordinates
            this.ctx.save();
            this.ctx.setLineDash([6, 4]);
            this.ctx.strokeStyle = "#7ec7ff";
            this.ctx.lineWidth = 2;

            const startCanvas = this.worldToCanvas(this.drawStart.x, this.drawStart.y);
            const endCanvas = this.worldToCanvas(world.x, world.y);

            const x = Math.min(startCanvas.x, endCanvas.x);
            const y = Math.min(startCanvas.y, endCanvas.y);
            const w = Math.abs(startCanvas.x - endCanvas.x);
            const h = Math.abs(startCanvas.y - endCanvas.y);

            this.ctx.strokeRect(x, y, w, h);
            this.ctx.restore();
        } else if ((this.isMoving || this.isResizing) && this.entityManager.selectedIdx !== null) {
            const entity = this.entityManager.entities[this.entityManager.selectedIdx];
            const pos = entity.components.find(c => c.type === "Position");
            const shape = entity.components.find(c => c.type === "Shape");

            if (this.isResizing) {
                const dx = (mouse.x - this.resizeStart.x) / this.scale;
                const dy = (mouse.y - this.resizeStart.y) / this.scale;
                let x = this.origRect.x, y = this.origRect.y;
                let w = this.origRect.w, h = this.origRect.h;

                if (this.resizeDir === "nw") {
                    x += dx; y += dy; w -= dx; h -= dy;
                } else if (this.resizeDir === "ne") {
                    y += dy; w += dx; h -= dy;
                } else if (this.resizeDir === "sw") {
                    x += dx; w -= dx; h += dy;
                } else if (this.resizeDir === "se") {
                    w += dx; h += dy;
                }

                pos.x = this.snap(Math.max(0, x));
                pos.y = this.snap(Math.max(0, y));
                shape.w = this.snap(Math.max(0.5, w));
                shape.h = this.snap(Math.max(0.5, h));
            } else if (this.isMoving) {
                pos.x = this.snap(Math.max(0, (mouse.x - this.moveOffset.x) / this.scale));
                pos.y = this.snap(Math.max(0, (mouse.y - this.moveOffset.y) / this.scale));
            }

            this.render();
            if (this.onChange) this.onChange();
        } else if (this.mode === 'edit' && this.entityManager.selectedIdx !== null) {
            const entity = this.entityManager.entities[this.entityManager.selectedIdx];
            const handle = this.getResizeHandle(entity, mouse.x, mouse.y);
            this.canvas.style.cursor = handle ? 'nwse-resize' : 'move';
        }
    }

    onMouseUp(evt) {
        const mouse = this.getMousePos(evt);
        const world = this.canvasToWorld(mouse.x, mouse.y);

        // End multi-select
        if (this.isSelecting) {
            this.isSelecting = false;
            this.render();
            if (this.onMultiSelectChange) {
                this.onMultiSelectChange(this.getSelectedEntities());
            }
            return;
        }

        if (this.mode === 'draw' && this.isDrawing) {
            this.isDrawing = false;
            let x = Math.min(this.drawStart.x, world.x);
            let y = Math.min(this.drawStart.y, world.y);
            let w = Math.abs(this.drawStart.x - world.x);
            let h = Math.abs(this.drawStart.y - world.y);

            x = this.snap(x);
            y = this.snap(y);
            w = Math.max(this.snapSize, this.snap(w));
            h = Math.max(this.snapSize, this.snap(h));

            if (w > 0 && h > 0) {
                const entity = this.entityManager.create(x, y, w, h);
                this.entityManager.select(this.entityManager.entities.length - 1);

                this.render();
                if (this.onChange) this.onChange();
                if (this.onSelectionChange) this.onSelectionChange();
            }
        } else if (this.isMoving || this.isResizing) {
            this.isMoving = false;
            this.isResizing = false;
            this.render();
            if (this.onChange) this.onChange();
        }
    }

    render() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        // Draw grid
        this.drawGrid();

        const effectiveScale = this.scale * this.viewport.zoom;
        const sorted = this.entityManager.getSorted();

        for (let i = 0; i < sorted.length; i++) {
            const entity = sorted[i];
            const idx = this.entityManager.entities.indexOf(entity);
            const isSelected = idx === this.entityManager.selectedIdx || this.selectedEntities.has(idx);

            const pos = entity.components.find(c => c.type === "Position");
            const shape = entity.components.find(c => c.type === "Shape");
            const color = entity.components.find(c => c.type === "Color");

            if (!pos || !shape) continue;

            // Convert world coordinates to canvas
            const canvasPos = this.worldToCanvas(pos.x, pos.y);
            const canvasW = shape.w * effectiveScale;
            const canvasH = shape.h * effectiveScale;

            // Skip if completely outside viewport
            if (canvasPos.x + canvasW < 0 || canvasPos.x > this.canvas.width ||
                canvasPos.y + canvasH < 0 || canvasPos.y > this.canvas.height) {
                continue;
            }

            // Draw entity rectangle
            if (color) {
                const a = color.a !== undefined ? color.a : 1.0;
                this.ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${a})`;
            } else {
                this.ctx.fillStyle = 'rgba(136, 238, 255, 1.0)';
            }
            this.ctx.fillRect(canvasPos.x, canvasPos.y, canvasW, canvasH);

            // Draw border
            this.ctx.lineWidth = isSelected ? 3 : 1.5;
            this.ctx.strokeStyle = isSelected ? '#3a7bd5' : '#444';
            this.ctx.strokeRect(canvasPos.x, canvasPos.y, canvasW, canvasH);

            // Draw label
            this.ctx.font = 'bold 13px sans-serif';
            this.ctx.fillStyle = '#fff';
            this.ctx.textBaseline = 'top';
            this.ctx.fillText(`${entity.id}:${entity.name}`,
                            canvasPos.x + 4, canvasPos.y + 2);

            // Draw z-order
            const renderPriority = entity.components.find(c => c.type === "RenderPriority");
            if (renderPriority) {
                this.ctx.font = '10px monospace';
                this.ctx.fillStyle = '#7ec7ff';
                this.ctx.fillText(`z:${renderPriority.z}`,
                                canvasPos.x + 4, canvasPos.y + 18);
            }
        }

        // Draw selection box
        if (this.isSelecting && this.selectStart) {
            const mouse = this.getMousePos(event);

            this.ctx.strokeStyle = 'rgba(58, 123, 213, 0.8)';
            this.ctx.lineWidth = 2;
            this.ctx.fillStyle = 'rgba(58, 123, 213, 0.1)';

            const sx = Math.min(this.selectStart.x, mouse.x);
            const sy = Math.min(this.selectStart.y, mouse.y);
            const sw = Math.abs(mouse.x - this.selectStart.x);
            const sh = Math.abs(mouse.y - this.selectStart.y);

            this.ctx.fillRect(sx, sy, sw, sh);
            this.ctx.strokeRect(sx, sy, sw, sh);

            // Update selected entities based on box in world coordinates
            const worldBox = {
                x: Math.min(this.selectStart.x, this.canvasToWorld(mouse.x, mouse.y).x),
                y: Math.min(this.selectStart.y, this.canvasToWorld(mouse.x, mouse.y).y),
                w: Math.abs(this.canvasToWorld(mouse.x, mouse.y).x - this.selectStart.x),
                h: Math.abs(this.canvasToWorld(mouse.x, mouse.y).y - this.selectStart.y)
            };
            this.updateMultiSelection(worldBox);
        }

        // Draw viewport info
        this.drawViewportInfo();
    }

    // Draw grid
    drawGrid() {
        const effectiveScale = this.scale * this.viewport.zoom;
        const gridSize = this.snapSize * effectiveScale;

        if (gridSize < 4) return; // Don't draw grid if too small

        this.ctx.strokeStyle = '#2a2a2a';
        this.ctx.lineWidth = 0.5;

        // Calculate visible grid range
        const startX = Math.floor(this.viewport.x / this.snapSize) * this.snapSize;
        const startY = Math.floor(this.viewport.y / this.snapSize) * this.snapSize;
        const endX = this.viewport.x + this.canvas.width / effectiveScale;
        const endY = this.viewport.y + this.canvas.height / effectiveScale;

        // Draw vertical lines
        for (let x = startX; x <= endX; x += this.snapSize) {
            const canvasX = (x - this.viewport.x) * effectiveScale;
            this.ctx.beginPath();
            this.ctx.moveTo(canvasX, 0);
            this.ctx.lineTo(canvasX, this.canvas.height);
            this.ctx.stroke();
        }

        // Draw horizontal lines
        for (let y = startY; y <= endY; y += this.snapSize) {
            const canvasY = (y - this.viewport.y) * effectiveScale;
            this.ctx.beginPath();
            this.ctx.moveTo(0, canvasY);
            this.ctx.lineTo(this.canvas.width, canvasY);
            this.ctx.stroke();
        }

        // Draw origin axes
        this.ctx.strokeStyle = '#444';
        this.ctx.lineWidth = 1;

        const originX = -this.viewport.x * effectiveScale;
        const originY = -this.viewport.y * effectiveScale;

        if (originX >= 0 && originX <= this.canvas.width) {
            this.ctx.beginPath();
            this.ctx.moveTo(originX, 0);
            this.ctx.lineTo(originX, this.canvas.height);
            this.ctx.stroke();
        }

        if (originY >= 0 && originY <= this.canvas.height) {
            this.ctx.beginPath();
            this.ctx.moveTo(0, originY);
            this.ctx.lineTo(this.canvas.width, originY);
            this.ctx.stroke();
        }
    }

    // Draw viewport info
    drawViewportInfo() {
        this.ctx.fillStyle = 'rgba(42, 42, 42, 0.9)';
        this.ctx.fillRect(10, 10, 220, 65);

        this.ctx.fillStyle = '#7ec7ff';
        this.ctx.font = '12px monospace';
        this.ctx.fillText(`Viewport: ${this.viewport.x.toFixed(1)}, ${this.viewport.y.toFixed(1)}`, 20, 30);
        this.ctx.fillText(`Zoom: ${(this.viewport.zoom * 100).toFixed(0)}%`, 20, 45);

        const center = this.getViewCenter();
        this.ctx.fillText(`Center: ${center.x.toFixed(1)}, ${center.y.toFixed(1)}`, 20, 60);
    }
}