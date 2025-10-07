// Import component schemas from the single source of truth
import { ComponentSchemas } from '../scripts/ebuilder.js';

// Convert ComponentSchemas object to array format for compatibility
// Note: Color component needs special handling for editor (uses 0-255 range)
export const COMPONENT_SCHEMAS = Object.entries(ComponentSchemas).map(([key, schema]) => {
    console.log(key, schema)
    const componentSchema = {
        type: schema.type,  // Use the type from schema
        configKey: key,  // Keep original key for export
        params: [...schema.fields]  // Clone fields array (it's 'fields' not 'params')
    };

    // Special handling for color component - editor uses 0-255 range
    if (key === 'color') {
        componentSchema.params = [
            { name: "r", type: "number", min: 0, max: 255, step: 1, label: "R", default: 136 },
            { name: "g", type: "number", min: 0, max: 255, step: 1, label: "G", default: 238 },
            { name: "b", type: "number", min: 0, max: 255, step: 1, label: "B", default: 255 },
            { name: "a", type: "number", min: 0, max: 1, step: 0.01, label: "Alpha", default: 1.0 }
        ];
    }

    // Special handling for teleporter - editor uses separate fields
    if (key === 'teleporter') {
        componentSchema.params = [
            { name: "destX", type: "number", step: 0.01, label: "Dest X", default: 0 },
            { name: "destY", type: "number", step: 0.01, label: "Dest Y", default: 0 },
            { name: "destZ", type: "number", step: 0.01, label: "Dest Z", default: 0 },
            { name: "interiorEntity", type: "number", step: 1, label: "Interior Entity", default: 0 }
        ];
    }

    // Fix 'hidden' vs 'hide' inconsistency in Text component
    if (key === 'text') {
        componentSchema.params = componentSchema.params.map(p =>
            p.name === 'hide' ? { ...p, name: 'hidden', label: 'Hidden' } : p
        );
    }

    // Map cshader to CustomShader
    if (key === 'cshader') {
        componentSchema.type = 'CustomShader';
    }

    return componentSchema;
}).filter(schema => schema.configKey !== 'meta'); // Filter out meta as it's not a component

// Component utilities
export function getComponent(entity, type) {
    return entity.components.find(c => c.type === type);
}

export function ensureComponent(entity, type) {
    let c = getComponent(entity, type);
    if (!c) {
        const schema = COMPONENT_SCHEMAS.find(s => s.type === type);
        c = { type };
        if (schema) {
            for (let p of schema.params) {
                c[p.name] = p.default;
            }
        }
        entity.components.push(c);
    }
    return c;
}

export function removeComponent(entity, type) {
    entity.components = entity.components.filter(c => c.type !== type);
}

// Default components for new entities
export function getDefaultComponents() {
    return [
        { type: "Position", x: 0, y: 0, z: 0 },
        { type: "Shape", w: 1, h: 1, d: 0 },
        { type: "Color", r: 136, g: 238, b: 255, a: 1.0 },
        { type: "RenderPriority", z: 0 }
    ];
}