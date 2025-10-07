// Entity management
import { getDefaultComponents } from './components.js';

export class EntityManager {
    constructor() {
        this.entities = [];
        this.selectedIdx = null;
        this.nextId = 1;
    }

    create(x = 0, y = 0, w = 1, h = 1) {
        const entity = {
            id: this.nextId++,
            name: `entity${this.nextId - 1}`,
            components: getDefaultComponents()
        };

        // Set initial position and shape
        const pos = entity.components.find(c => c.type === "Position");
        const shape = entity.components.find(c => c.type === "Shape");
        if (pos) {
            pos.x = x;
            pos.y = y;
        }
        if (shape) {
            shape.w = w;
            shape.h = h;
        }

        this.entities.push(entity);
        return entity;
    }

    delete(idx) {
        if (idx >= 0 && idx < this.entities.length) {
            this.entities.splice(idx, 1);
            if (this.selectedIdx === idx) {
                this.selectedIdx = null;
            } else if (this.selectedIdx > idx) {
                this.selectedIdx--;
            }
        }
    }

    clear() {
        this.entities = [];
        this.selectedIdx = null;
        this.nextId = 1;
    }

    duplicate(idx) {
        if (idx < 0 || idx >= this.entities.length) return null;

        const orig = this.entities[idx];
        const copy = JSON.parse(JSON.stringify(orig));
        copy.id = this.nextId++;
        copy.name = `entity${copy.id}`;

        // Offset position
        const pos = copy.components.find(c => c.type === "Position");
        if (pos) {
            pos.x += 1;
            pos.y += 1;
        }

        this.entities.push(copy);
        return copy;
    }

    select(idx) {
        this.selectedIdx = (idx >= 0 && idx < this.entities.length) ? idx : null;
    }

    getSelected() {
        return this.selectedIdx !== null ? this.entities[this.selectedIdx] : null;
    }

    getSorted() {
        return this.entities.slice().sort((a, b) => {
            const az = a.components.find(c => c.type === "RenderPriority")?.z ?? 0;
            const bz = b.components.find(c => c.type === "RenderPriority")?.z ?? 0;
            if (az !== bz) return az - bz;
            return (a.id || 0) - (b.id || 0);
        });
    }

    findAt(x, y, scale = 16) {
        // Find topmost entity at position (reverse order for z-order)
        for (let i = this.entities.length - 1; i >= 0; i--) {
            const e = this.entities[i];
            const pos = e.components.find(c => c.type === "Position");
            const shape = e.components.find(c => c.type === "Shape");

            if (pos && shape) {
                const ex = pos.x * scale;
                const ey = pos.y * scale;
                const ew = shape.w * scale;
                const eh = shape.h * scale;

                if (x >= ex && x <= ex + ew && y >= ey && y <= ey + eh) {
                    return i;
                }
            }
        }
        return -1;
    }

    updateEntity(idx, id, name) {
        if (idx < 0 || idx >= this.entities.length) return;
        const entity = this.entities[idx];
        if (id !== undefined && id >= 0) entity.id = id;
        if (name !== undefined && name.trim()) entity.name = name.trim();
    }

    // Load/save state
    getState() {
        return {
            entities: this.entities,
            selectedIdx: this.selectedIdx,
            nextId: this.nextId
        };
    }

    setState(state) {
        this.entities = state.entities || [];
        this.selectedIdx = state.selectedIdx;
        this.nextId = state.nextId || 1;
    }
}