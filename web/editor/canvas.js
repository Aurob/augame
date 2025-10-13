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
        this.gridVisible = true;

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
        this.multiMoveOffsets = []; // Store offsets for each selected entity in multi-move

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

        // Debug mode
        this.debugMode = false;
        this.debugTooltip = document.getElementById('debugTooltip');

        this.setupEvents();
    }

    setupEvents() {
        this.canvas.addEventListener('mousedown', e => this.onMouseDown(e));
        this.canvas.addEventListener('mousemove', e => this.onMouseMove(e));
        this.canvas.addEventListener('mouseup', e => this.onMouseUp(e));
        this.canvas.addEventListener('wheel', e => this.onWheel(e));

        // Keyboard events for multi-select, panning, and debug mode
        document.addEventListener('keydown', e => {
            if (e.key === 'Shift' && !e.repeat) {
                this.multiSelectMode = true;
                this.canvas.style.cursor = 'crosshair';
            } else if (e.key === 'Alt' && !e.repeat) {
                this.canvas.style.cursor = 'move';
                e.preventDefault(); // Prevent browser menu
            } else if (e.key === 'Control' && !e.repeat) {
                this.debugMode = true;
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
            } else if (e.key === 'Control') {
                this.debugMode = false;
                if (this.debugTooltip) {
                    this.debugTooltip.style.display = 'none';
                }
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

    setGridVisible(visible) {
        this.gridVisible = visible;
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

    // Find all entities at world coordinates
    // Returns array of {entity, index, priority, area} sorted by selection priority
    findAllEntitiesAt(worldX, worldY) {
        const candidates = [];

        // Find all entities that contain the point
        for (let i = 0; i < this.entityManager.entities.length; i++) {
            const e = this.entityManager.entities[i];
            const pos = e.components.find(c => c.type === "Position");
            const shape = e.components.find(c => c.type === "Shape");

            if (pos && shape) {
                if (worldX >= pos.x && worldX <= pos.x + shape.w &&
                    worldY >= pos.y && worldY <= pos.y + shape.h) {
                    const renderPriority = e.components.find(c => c.type === "RenderPriority");
                    const priority = renderPriority ? renderPriority.z : 0;
                    const area = shape.w * shape.h;

                    candidates.push({ entity: e, index: i, priority, area });
                }
            }
        }

        // Sort by priority (highest first), then by area (smallest first)
        candidates.sort((a, b) => {
            if (a.priority !== b.priority) {
                return b.priority - a.priority; // Higher priority first
            }
            return a.area - b.area; // Smaller area first
        });

        return candidates;
    }

    // Find entity at world coordinates
    // Prioritizes smaller entities and entities with higher render priority
    findEntityAt(worldX, worldY) {
        const candidates = this.findAllEntitiesAt(worldX, worldY);
        return candidates.length > 0 ? candidates[0].index : -1;
    }

    // Clear multi-selection
    clearMultiSelection() {
        this.selectedEntities.clear();
        this.render();
    }

    // Update multi-selection based on selection box (in world coordinates)
    updateMultiSelection(selBox) {
        this.selectedEntities.clear();

        this.entityManager.entities.forEach((entity, idx) => {
            const pos = entity.components.find(c => c.type === "Position");
            const shape = entity.components.find(c => c.type === "Shape");

            if (!pos || !shape) return;

            // Check if entity is within selection box (both in world coordinates)
            const entityBox = {
                x: pos.x,
                y: pos.y,
                w: shape.w,
                h: shape.h
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
                // Check if clicked entity is in multi-selection
                const isInMultiSelection = this.selectedEntities.has(idx);

                if (isInMultiSelection && this.selectedEntities.size > 0) {
                    // Start multi-entity move
                    this.isMoving = true;
                    this.multiMoveOffsets = [];

                    // Store offset for each selected entity
                    this.selectedEntities.forEach(entityIdx => {
                        const e = this.entityManager.entities[entityIdx];
                        const p = e.components.find(c => c.type === "Position");
                        if (p) {
                            this.multiMoveOffsets.push({
                                idx: entityIdx,
                                offsetX: world.x - p.x,
                                offsetY: world.y - p.y
                            });
                        }
                    });
                } else {
                    // Single entity selection and move
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
                            // Store offset in world coordinates
                            this.moveOffset.x = world.x - pos.x;
                            this.moveOffset.y = world.y - pos.y;
                        }
                    } else if (this.mode === 'move') {
                        this.isMoving = true;
                        // Store offset in world coordinates
                        this.moveOffset.x = world.x - pos.x;
                        this.moveOffset.y = world.y - pos.y;
                    }

                    if (this.onSelectionChange) this.onSelectionChange();
                }
            } else {
                this.entityManager.select(null);
                this.clearMultiSelection();
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
        } else if (this.isMoving && this.multiMoveOffsets.length > 0) {
            // Multi-entity move
            this.multiMoveOffsets.forEach(({ idx, offsetX, offsetY }) => {
                const entity = this.entityManager.entities[idx];
                const pos = entity.components.find(c => c.type === "Position");
                if (pos) {
                    pos.x = this.snap(world.x - offsetX);
                    pos.y = this.snap(world.y - offsetY);
                }
            });

            this.render();
            // Don't call onChange during drag - only on mouseup
        } else if ((this.isMoving || this.isResizing) && this.entityManager.selectedIdx !== null) {
            const entity = this.entityManager.entities[this.entityManager.selectedIdx];
            const pos = entity.components.find(c => c.type === "Position");
            const shape = entity.components.find(c => c.type === "Shape");

            if (this.isResizing) {
                const effectiveScale = this.scale * this.viewport.zoom;
                const dx = (mouse.x - this.resizeStart.x) / effectiveScale;
                const dy = (mouse.y - this.resizeStart.y) / effectiveScale;
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

                pos.x = this.snap(x);
                pos.y = this.snap(y);
                shape.w = this.snap(Math.max(0.5, w));
                shape.h = this.snap(Math.max(0.5, h));
            } else if (this.isMoving) {
                // Use world coordinates for accurate movement
                pos.x = this.snap(world.x - this.moveOffset.x);
                pos.y = this.snap(world.y - this.moveOffset.y);
            }

            this.render();
            // Don't call onChange during drag - only on mouseup
        } else if (this.mode === 'edit' && this.entityManager.selectedIdx !== null) {
            const entity = this.entityManager.entities[this.entityManager.selectedIdx];
            const handle = this.getResizeHandle(entity, mouse.x, mouse.y);
            this.canvas.style.cursor = handle ? 'nwse-resize' : 'move';
        }

        // Show debug tooltip when Ctrl is held
        if (this.debugMode && this.debugTooltip) {
            const entitiesAtCursor = this.findAllEntitiesAt(world.x, world.y);

            if (entitiesAtCursor.length > 0) {
                let tooltipHTML = '<div style="font-weight: bold; margin-bottom: 4px; color: #7ec7ff;">Entities at cursor:</div>';
                entitiesAtCursor.forEach(({entity, index}) => {
                    const pos = entity.components.find(c => c.type === "Position");
                    const shape = entity.components.find(c => c.type === "Shape");
                    const renderPriority = entity.components.find(c => c.type === "RenderPriority");
                    const priority = renderPriority ? renderPriority.z : 0;
                    const area = shape ? (shape.w * shape.h).toFixed(1) : '?';

                    tooltipHTML += `<div style="margin: 4px 0; padding: 4px; background: rgba(255,255,255,0.1); border-radius: 2px;">`;
                    tooltipHTML += `<div><span style="color: #7ec7ff;">ID:</span> ${entity.id}</div>`;
                    tooltipHTML += `<div><span style="color: #7ec7ff;">Name:</span> ${entity.name}</div>`;
                    tooltipHTML += `<div><span style="color: #7ec7ff;">Priority:</span> ${priority} <span style="color: #666;">|</span> <span style="color: #7ec7ff;">Area:</span> ${area}</div>`;
                    tooltipHTML += `</div>`;
                });

                this.debugTooltip.innerHTML = tooltipHTML;
                this.debugTooltip.style.display = 'block';
                this.debugTooltip.style.left = (evt.clientX + 15) + 'px';
                this.debugTooltip.style.top = (evt.clientY + 15) + 'px';
            } else {
                this.debugTooltip.style.display = 'none';
            }
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
            this.multiMoveOffsets = []; // Clear multi-move state
            this.render();
            if (this.onChange) this.onChange();
        }

        // End panning
        this.isPanning = false;
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

            // Check if entity has Texture component
            const textureComp = entity.components.find(c => c.type === "Texture");

            if (textureComp && textureComp.name) {
                // Try to load and draw texture
                const texturePath = `../resources/textures/${textureComp.name}.png`;
                if (!this.textureCache) this.textureCache = {};

                if (this.textureCache[textureComp.name] === undefined) {
                    // Start loading the texture
                    const img = new Image();
                    img.onload = () => {
                        this.textureCache[textureComp.name] = img;
                        this.render(); // Re-render when texture loads
                    };
                    img.onerror = () => {
                        this.textureCache[textureComp.name] = null; // Mark as failed
                    };
                    img.src = texturePath;
                    this.textureCache[textureComp.name] = 'loading';
                }

                // Draw the texture if loaded
                if (this.textureCache[textureComp.name] && this.textureCache[textureComp.name] !== 'loading') {
                    const img = this.textureCache[textureComp.name];
                    this.ctx.drawImage(img, canvasPos.x, canvasPos.y, canvasW, canvasH);
                } else {
                    // Show placeholder while loading or if failed
                    if (color) {
                        const a = color.a !== undefined ? color.a : 1.0;
                        this.ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${a})`;
                    } else {
                        this.ctx.fillStyle = 'rgba(136, 238, 255, 1.0)';
                    }
                    this.ctx.fillRect(canvasPos.x, canvasPos.y, canvasW, canvasH);
                }
            } else {
                // Draw entity rectangle
                if (color) {
                    const a = color.a !== undefined ? color.a : 1.0;
                    this.ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${a})`;
                } else {
                    this.ctx.fillStyle = 'rgba(136, 238, 255, 1.0)';
                }
                this.ctx.fillRect(canvasPos.x, canvasPos.y, canvasW, canvasH);
            }

            // Draw border
            this.ctx.lineWidth = isSelected ? 3 : 1.5;
            this.ctx.strokeStyle = isSelected ? '#3a7bd5' : '#444';
            this.ctx.strokeRect(canvasPos.x, canvasPos.y, canvasW, canvasH);

            // Check if entity has Text component
            const textComp = entity.components.find(c => c.type === "Text");
            if (textComp && textComp.text && !textComp.hidden) {
                // Draw text component content at center
                // Scale is stored as 0.01-1.0, display as if 100x larger
                const displayScale = (textComp.scale || 0.01) * 100;
                const fontSize = Math.max(12, Math.min(32, displayScale * 16));
                this.ctx.font = `${fontSize}px sans-serif`;
                this.ctx.fillStyle = 'rgba(255, 255, 255, 0.95)';
                this.ctx.textAlign = 'center';
                this.ctx.textBaseline = 'middle';
                const textX = canvasPos.x + canvasW / 2 + (textComp.offsetX || 0) * this.scale;
                const textY = canvasPos.y + canvasH / 2 + (textComp.offsetY || 0) * this.scale;
                this.ctx.fillText(String(textComp.text), textX, textY);
            } else if (!textureComp) {
                // Draw entity ID at center (if no text component and no texture)
                this.ctx.font = 'bold 16px sans-serif';
                this.ctx.fillStyle = 'rgba(255, 255, 255, 0.9)';
                this.ctx.textAlign = 'center';
                this.ctx.textBaseline = 'middle';
                this.ctx.fillText(`${entity.id}`,
                                canvasPos.x + canvasW / 2, canvasPos.y + canvasH / 2);
            }

            // Draw name label at top-left (only if entity is large enough)
            if (canvasW > 40 && canvasH > 20) {
                this.ctx.font = '11px sans-serif';
                this.ctx.fillStyle = 'rgba(255, 255, 255, 0.7)';
                this.ctx.textAlign = 'left';
                this.ctx.textBaseline = 'top';
                this.ctx.fillText(entity.name, canvasPos.x + 4, canvasPos.y + 2);
            }

            // Draw z-order at bottom-left (only if entity is large enough)
            const renderPriority = entity.components.find(c => c.type === "RenderPriority");
            if (renderPriority && canvasW > 40 && canvasH > 40) {
                this.ctx.font = '10px monospace';
                this.ctx.fillStyle = '#7ec7ff';
                this.ctx.textAlign = 'left';
                this.ctx.textBaseline = 'bottom';
                this.ctx.fillText(`z:${renderPriority.z}`,
                                canvasPos.x + 4, canvasPos.y + canvasH - 2);
            }
        }

        // Draw selection box
        if (this.isSelecting && this.selectStart) {
            // Get current mouse position in canvas coordinates
            const mouseEvent = window.event || { clientX: 0, clientY: 0 };
            const mouse = this.getMousePos(mouseEvent);
            const mouseWorld = this.canvasToWorld(mouse.x, mouse.y);

            // Convert selection start (world) to canvas
            const startCanvas = this.worldToCanvas(this.selectStart.x, this.selectStart.y);

            this.ctx.strokeStyle = 'rgba(58, 123, 213, 0.8)';
            this.ctx.lineWidth = 2;
            this.ctx.fillStyle = 'rgba(58, 123, 213, 0.1)';

            const sx = Math.min(startCanvas.x, mouse.x);
            const sy = Math.min(startCanvas.y, mouse.y);
            const sw = Math.abs(mouse.x - startCanvas.x);
            const sh = Math.abs(mouse.y - startCanvas.y);

            this.ctx.fillRect(sx, sy, sw, sh);
            this.ctx.strokeRect(sx, sy, sw, sh);

            // Update selected entities based on box in world coordinates
            const worldBox = {
                x: Math.min(this.selectStart.x, mouseWorld.x),
                y: Math.min(this.selectStart.y, mouseWorld.y),
                w: Math.abs(mouseWorld.x - this.selectStart.x),
                h: Math.abs(mouseWorld.y - this.selectStart.y)
            };
            this.updateMultiSelection(worldBox);
        }

        // Draw viewport info
        this.drawViewportInfo();
    }

    // Helper to parse color from meta value
    parseMetaColor(metaValue) {
        if (!metaValue) return null;

        // Already an array [r, g, b] (0-1 range from parser)
        if (Array.isArray(metaValue) && metaValue.length === 3) {
            const r = Math.round(metaValue[0] * 255);
            const g = Math.round(metaValue[1] * 255);
            const b = Math.round(metaValue[2] * 255);
            return { r, g, b };
        }

        // Hex color
        if (typeof metaValue === 'string' && metaValue.startsWith('#')) {
            const hex = metaValue.substring(1);
            const r = parseInt(hex.substring(0, 2), 16);
            const g = parseInt(hex.substring(2, 4), 16);
            const b = parseInt(hex.substring(4, 6), 16);
            return { r, g, b };
        }

        // RGB string "r,g,b"
        if (typeof metaValue === 'string' && metaValue.includes(',')) {
            const parts = metaValue.split(',').map(s => parseInt(s.trim()));
            if (parts.length === 3) {
                return { r: parts[0], g: parts[1], b: parts[2] };
            }
        }

        return null;
    }

    // Calculate complementary grid color
    getComplementaryGridColor(bgColor) {
        if (!bgColor) return 'rgba(42, 42, 42, 0.5)';

        // Calculate relative luminance
        const r = bgColor.r / 255;
        const g = bgColor.g / 255;
        const b = bgColor.b / 255;
        const luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;

        // If background is dark, use lighter grid lines
        // If background is light, use darker grid lines
        if (luminance < 0.5) {
            // Dark background - lighten by adding to RGB
            const gridR = Math.min(255, bgColor.r + 60);
            const gridG = Math.min(255, bgColor.g + 60);
            const gridB = Math.min(255, bgColor.b + 60);
            return `rgba(${gridR}, ${gridG}, ${gridB}, 0.3)`;
        } else {
            // Light background - darken by subtracting from RGB
            const gridR = Math.max(0, bgColor.r - 60);
            const gridG = Math.max(0, bgColor.g - 60);
            const gridB = Math.max(0, bgColor.b - 60);
            return `rgba(${gridR}, ${gridG}, ${gridB}, 0.5)`;
        }
    }

    // Draw grid
    drawGrid() {
        const effectiveScale = this.scale * this.viewport.zoom;
        const gridSize = this.snapSize * effectiveScale;

        // Get terrain bounds from scene metadata if available
        let terrainBounds = null;
        if (this.getTerrainBounds) {
            terrainBounds = this.getTerrainBounds();
        }

        // Get world and void colors from meta
        let worldColor = null;
        let voidColor = null;
        if (this.getMetaValue) {
            const worldMeta = this.getMetaValue('world');
            const voidMeta = this.getMetaValue('void');
            worldColor = this.parseMetaColor(worldMeta);
            voidColor = this.parseMetaColor(voidMeta);
        }

        // Fill entire canvas with world color (or default)
        if (worldColor) {
            this.ctx.fillStyle = `rgb(${worldColor.r}, ${worldColor.g}, ${worldColor.b})`;
        } else {
            this.ctx.fillStyle = '#222';
        }
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

        // Fill void color outside terrain bounds
        if (voidColor && terrainBounds && terrainBounds.length === 4) {
            const [minX, minY, maxX, maxY] = terrainBounds;
            const boundsStart = this.worldToCanvas(minX, minY);
            const boundsEnd = this.worldToCanvas(maxX, maxY);

            this.ctx.fillStyle = `rgb(${voidColor.r}, ${voidColor.g}, ${voidColor.b})`;

            // Fill areas outside the bounds
            // Top area
            if (boundsStart.y > 0) {
                this.ctx.fillRect(0, 0, this.canvas.width, boundsStart.y);
            }
            // Bottom area
            if (boundsEnd.y < this.canvas.height) {
                this.ctx.fillRect(0, boundsEnd.y, this.canvas.width, this.canvas.height - boundsEnd.y);
            }
            // Left area (between top and bottom bounds)
            if (boundsStart.x > 0) {
                this.ctx.fillRect(0, Math.max(0, boundsStart.y), boundsStart.x, Math.min(this.canvas.height, boundsEnd.y) - Math.max(0, boundsStart.y));
            }
            // Right area (between top and bottom bounds)
            if (boundsEnd.x < this.canvas.width) {
                this.ctx.fillRect(boundsEnd.x, Math.max(0, boundsStart.y), this.canvas.width - boundsEnd.x, Math.min(this.canvas.height, boundsEnd.y) - Math.max(0, boundsStart.y));
            }
        }

        // Draw grid lines if visible and large enough
        if (this.gridVisible && gridSize >= 4) {
            const gridColor = this.getComplementaryGridColor(worldColor);

            // Calculate visible grid range
            const startX = Math.floor(this.viewport.x / this.snapSize) * this.snapSize;
            const startY = Math.floor(this.viewport.y / this.snapSize) * this.snapSize;
            const endX = this.viewport.x + this.canvas.width / effectiveScale;
            const endY = this.viewport.y + this.canvas.height / effectiveScale;

            this.ctx.strokeStyle = gridColor;
            this.ctx.lineWidth = 0.5;

            // Clip to terrain bounds if they exist
            if (terrainBounds && terrainBounds.length === 4) {
                const [minX, minY, maxX, maxY] = terrainBounds;
                const boundsStart = this.worldToCanvas(minX, minY);
                const boundsEnd = this.worldToCanvas(maxX, maxY);

                this.ctx.save();
                this.ctx.beginPath();
                this.ctx.rect(boundsStart.x, boundsStart.y, boundsEnd.x - boundsStart.x, boundsEnd.y - boundsStart.y);
                this.ctx.clip();
            }

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

            if (terrainBounds && terrainBounds.length === 4) {
                this.ctx.restore();
            }
        }

        // Always draw terrain bounds rectangle regardless of zoom
        if (terrainBounds && terrainBounds.length === 4) {
            const [minX, minY, maxX, maxY] = terrainBounds;
            const boundsStart = this.worldToCanvas(minX, minY);
            const boundsEnd = this.worldToCanvas(maxX, maxY);

            this.ctx.strokeStyle = '#ff6b6b';
            this.ctx.lineWidth = 2;
            this.ctx.setLineDash([8, 4]);
            this.ctx.strokeRect(
                boundsStart.x,
                boundsStart.y,
                boundsEnd.x - boundsStart.x,
                boundsEnd.y - boundsStart.y
            );
            this.ctx.setLineDash([]);

            // Draw label
            this.ctx.fillStyle = '#ff6b6b';
            this.ctx.font = 'bold 12px monospace';
            this.ctx.textAlign = 'left';
            this.ctx.textBaseline = 'top';
            this.ctx.fillText('Terrain Bounds', boundsStart.x + 4, boundsStart.y + 4);
        }

        // Draw origin axes
        this.ctx.strokeStyle = this.getComplementaryGridColor(worldColor);
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