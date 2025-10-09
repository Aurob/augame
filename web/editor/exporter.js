// Export/import utilities - matches augame config format
import { COMPONENT_SCHEMAS } from './components.js';
import { MetaSchemas } from '../scripts/ebuilder.js';

export class Exporter {
    static exportMeta(meta, includeDefaults = false) {
        const lines = [];

        // Always include all MetaSchemas keys with their defaults if not set
        const allKeys = new Set([...Object.keys(MetaSchemas), ...Object.keys(meta)]);

        allKeys.forEach(key => {
            // Get value from meta, or use default from schema
            let value = meta[key];
            if (value === '' || value === undefined || value === null) {
                value = MetaSchemas[key]?.default;
            }

            // Skip if still empty after applying defaults
            if (value === '' || value === undefined || value === null) {
                return;
            }

            // Skip slides if it's an empty object
            if (key === 'slides' && typeof value === 'object' && Object.keys(value).length === 0) {
                return;
            }

            // Handle slides object specially
            if (key === 'slides' && typeof value === 'object') {
                Object.entries(value).forEach(([slideId, slideText]) => {
                    if (slideText) {
                        lines.push(`meta slides ${slideId} "${slideText}"`);
                    }
                });
            } else {
                // Quote string values if they contain spaces
                let formattedValue = value;
                if (typeof value === 'string') {
                    // Check if it's a color value (RGB format)
                    if (key === 'void' || key === 'terrain') {
                        // If it's RGB values like "255,128,0", keep as is
                        if (value.match(/^\d+,\d+,\d+$/)) {
                            formattedValue = value;
                        }
                        // Otherwise quote if needed
                        else if (value.includes(' ') || value.startsWith('@')) {
                            formattedValue = `"${value}"`;
                        }
                    } else if (value.includes(' ') || value.startsWith('@')) {
                        formattedValue = `"${value}"`;
                    }
                }
                lines.push(`meta ${key} ${formattedValue}`);
            }
        });

        return lines.join('\n');
    }

    static exportScene(scene, includeDefaults = false) {
        const lines = [];

        // Export meta tags
        const metaExport = this.exportMeta(scene.meta, includeDefaults);
        if (metaExport) {
            lines.push(metaExport);
            lines.push(''); // Blank line after meta
        }

        // Export entities
        const entitiesExport = this.exportEntities(scene.entityManager.entities);
        if (entitiesExport) {
            lines.push(entitiesExport);
        }

        return lines.join('\n');
    }

    static exportMultipleScenes(scenes, includeDefaults = false) {
        const outputs = [];

        scenes.forEach(scene => {
            const sceneLines = [];

            // Always include scene meta tag for multi-scene configs
            const sceneName = scene.meta.scene || scene.name;
            sceneLines.push(`meta scene "${sceneName}"`);

            // Add other meta tags
            Object.entries(scene.meta).forEach(([key, value]) => {
                if (key === 'scene') return; // Already added

                // Skip defaults unless includeDefaults is true
                if (!includeDefaults && (!value || value === MetaSchemas[key]?.default)) {
                    return;
                }

                if (value === '' || value === undefined || value === null) {
                    return;
                }

                if (key === 'slides' && typeof value === 'object') {
                    Object.entries(value).forEach(([slideId, slideText]) => {
                        if (slideText) {
                            sceneLines.push(`meta slides ${slideId} "${slideText}"`);
                        }
                    });
                } else {
                    let formattedValue = value;
                    if (typeof value === 'string') {
                        if ((key === 'void' || key === 'terrain') && value.match(/^\d+,\d+,\d+$/)) {
                            formattedValue = value;
                        } else if (value.includes(' ') || value.startsWith('@')) {
                            formattedValue = `"${value}"`;
                        }
                    }
                    sceneLines.push(`meta ${key} ${formattedValue}`);
                }
            });

            // Add blank line after meta
            sceneLines.push('');

            // Export entities
            const entitiesExport = this.exportEntities(scene.entityManager.entities);
            if (entitiesExport) {
                sceneLines.push(entitiesExport);
            }

            outputs.push(sceneLines.join('\n'));
        });

        // Join scenes with double newline
        return outputs.join('\n\n\n');
    }
    static exportEntities(entities) {
        const lines = [];
        const sorted = entities.slice().sort((a, b) => {
            const az = a.components.find(c => c.type === "RenderPriority")?.z ?? 0;
            const bz = b.components.find(c => c.type === "RenderPriority")?.z ?? 0;
            if (az !== bz) return az - bz;
            return (a.id || 0) - (b.id || 0);
        });

        for (const entity of sorted) {
            const parts = [`id ${entity.id} ${entity.name}`];

            for (const comp of entity.components) {
                const exported = this.exportComponent(comp);
                if (exported) parts.push(exported);
            }

            lines.push(parts.join(' '));
        }

        return lines.join('\n');
    }

    static exportComponent(comp) {
        // Find the schema to get the config key
        const schema = COMPONENT_SCHEMAS.find(s => s.type === comp.type);
        const configKey = schema?.configKey || comp.type.toLowerCase();

        switch (comp.type) {
            case "Position":
                return `position ${comp.x.toFixed(4)} ${comp.y.toFixed(4)} ${comp.z?.toFixed(4) || '0'}`;

            case "Shape":
                return `shape ${comp.w.toFixed(4)} ${comp.h.toFixed(4)} ${comp.d?.toFixed(4) || '0'}`;

            case "Color":
                // Convert RGB 0-255 to 0-1 for engine
                const r = (comp.r / 255).toFixed(2);
                const g = (comp.g / 255).toFixed(2);
                const b = (comp.b / 255).toFixed(2);
                return `color ${r} ${g} ${b} ${comp.a.toFixed(2)}`;

            case "RenderPriority":
                return `renderPriority ${comp.z}`;

            case "Player":
                return "player";

            case "Collidable":
                return "collidable";

            case "Moveable":
                return "moveable";

            case "Hoverable":
                return "hoverable";

            case "World":
                return "world";

            case "Terrain":
                return "terrain";

            case "Interior":
                return comp.showInside ? "interior true" : "interior false";

            case "Inside":
                return `inside ${comp.insideId}`;

            case "Text":
                const hidden = comp.hidden ? 1 : 0;
                const text = String(comp.text || "").replace(/"/g, '\\"');
                return `text "${text}" ${comp.scale || 1} ${hidden} ${comp.offsetX || 0} ${comp.offsetY || 0}`;

            case "Movement":
                return `movement ${comp.speed || 0} ${comp.mass || 0} ${comp.restitution || 0} ${comp.friction || 0}`;

            case "InteriorPortal":
                const locked = comp.locked ? 1 : 0;
                return `interiorPortal ${comp.A || 0} ${comp.B || 0} ${locked} ${comp.key || -1}`;

            case "Camera":
                const important = comp.important ? 1 : 0;
                return `camera ${comp.gridSpacing || 1024} ${comp.defaultGSV || 16} ${comp.priority || 0} ${comp.radius || 0} ${important}`;

            case "Texture":
                return `texture "${comp.name}" ${comp.scalex || 1} ${comp.scaley || 1} ${comp.x || 0} ${comp.y || 0} ${comp.w || 1} ${comp.h || 1}`;

            case "TextureGroupPart":
                return `textureGroupPart "${comp.groupName}" "${comp.partName}" ${comp.tilex || 0} ${comp.tiley || 0}`;

            case "Interactable":
                const toggle = comp.toggleState ? "true" : "false";
                return `interactable ${comp.radius || 0.5} ${toggle}`;

            case "Teleporter":
                return `teleporter ${comp.destX || 0} ${comp.destY || 0} ${comp.destZ || 0} ${comp.interiorEntity || 0}`;

            case "Teleportable":
                return "teleportable";

            case "Draggable":
                return `draggable ${comp.radius || 1}`;

            case "CustomShader":
                return `cshader "${comp.shaderName}" ${comp.centerX || 0} ${comp.centerY || 0} ${comp.seed || 0}`;

            default:
                console.warn(`Unknown component type: ${comp.type}`);
                return null;
        }
    }

    static importEntities(text) {
        const entities = [];
        const lines = text.split('\n').filter(line => line.trim() && !line.trim().startsWith('--'));

        for (const line of lines) {
            const entity = this.parseLine(line);
            if (entity) entities.push(entity);
        }

        return entities;
    }

    static parseLine(line) {
        // Simple parser - assumes well-formed input
        // Real implementation would use ebuilder.js parser
        const parts = this.tokenize(line);
        if (parts.length < 3 || parts[0] !== 'id') return null;

        const entity = {
            id: parseInt(parts[1]),
            name: parts[2],
            components: []
        };

        let i = 3;
        while (i < parts.length) {
            const comp = this.parseComponent(parts, i);
            if (comp) {
                entity.components.push(comp.component);
                i += comp.consumed;
            } else {
                i++;
            }
        }

        return entity;
    }

    static tokenize(line) {
        const tokens = [];
        let current = '';
        let inQuotes = false;

        for (let i = 0; i < line.length; i++) {
            const char = line[i];

            if (char === '"') {
                inQuotes = !inQuotes;
            } else if (char === ' ' && !inQuotes) {
                if (current) {
                    tokens.push(current);
                    current = '';
                }
            } else {
                current += char;
            }
        }

        if (current) tokens.push(current);
        return tokens;
    }

    static parseComponent(parts, index) {
        const type = parts[index];

        // Map config names to component types
        const componentMap = {
            'position': { type: 'Position', params: 3 },
            'shape': { type: 'Shape', params: 3 },
            'color': { type: 'Color', params: 4 },
            'renderPriority': { type: 'RenderPriority', params: 1 },
            'player': { type: 'Player', params: 0 },
            'collidable': { type: 'Collidable', params: 0 },
            'moveable': { type: 'Moveable', params: 0 },
            'hoverable': { type: 'Hoverable', params: 0 },
            'world': { type: 'World', params: 0 },
            'terrain': { type: 'Terrain', params: 0 },
            'interior': { type: 'Interior', params: 1 },
            'inside': { type: 'Inside', params: 1 },
            'text': { type: 'Text', params: 5 },
            'movement': { type: 'Movement', params: 4 },
            'interiorPortal': { type: 'InteriorPortal', params: 4 },
            'camera': { type: 'Camera', params: 5 },
            'texture': { type: 'Texture', params: 7 },
            'textureGroupPart': { type: 'TextureGroupPart', params: 4 },
            'interactable': { type: 'Interactable', params: 2 },
            'teleporter': { type: 'Teleporter', params: 4 },
            'teleportable': { type: 'Teleportable', params: 0 },
            'draggable': { type: 'Draggable', params: 1 },
            'cshader': { type: 'CustomShader', params: 4 }
        };

        const mapping = componentMap[type];
        if (!mapping) return null;

        const component = { type: mapping.type };

        // Parse based on component type
        switch (mapping.type) {
            case 'Position':
                component.x = parseFloat(parts[index + 1]) || 0;
                component.y = parseFloat(parts[index + 2]) || 0;
                component.z = parseFloat(parts[index + 3]) || 0;
                break;

            case 'Shape':
                component.w = parseFloat(parts[index + 1]) || 1;
                component.h = parseFloat(parts[index + 2]) || 1;
                component.d = parseFloat(parts[index + 3]) || 0;
                break;

            case 'Color':
                // Convert from 0-1 to 0-255 for editor
                component.r = Math.round(parseFloat(parts[index + 1]) * 255) || 0;
                component.g = Math.round(parseFloat(parts[index + 2]) * 255) || 0;
                component.b = Math.round(parseFloat(parts[index + 3]) * 255) || 0;
                component.a = parseFloat(parts[index + 4]) || 1;
                break;

            case 'RenderPriority':
                component.z = parseInt(parts[index + 1]) || 0;
                break;

            case 'Interior':
                component.showInside = parts[index + 1] === 'true';
                break;

            case 'Inside':
                component.insideId = parseInt(parts[index + 1]) || 0;
                break;

            case 'Text':
                component.text = parts[index + 1] || '';
                component.scale = parseFloat(parts[index + 2]) || 1;
                component.hidden = parseInt(parts[index + 3]) === 1;
                component.offsetX = parseFloat(parts[index + 4]) || 0;
                component.offsetY = parseFloat(parts[index + 5]) || 0;
                break;

            case 'Movement':
                component.speed = parseFloat(parts[index + 1]) || 0;
                component.mass = parseFloat(parts[index + 2]) || 0;
                component.restitution = parseFloat(parts[index + 3]) || 0;
                component.friction = parseFloat(parts[index + 4]) || 0;
                break;

            case 'InteriorPortal':
                component.A = parseInt(parts[index + 1]) || 0;
                component.B = parseInt(parts[index + 2]) || 0;
                component.locked = parseInt(parts[index + 3]) === 1;
                component.key = parseInt(parts[index + 4]) || -1;
                break;

            case 'Camera':
                component.gridSpacing = parseFloat(parts[index + 1]) || 1024;
                component.defaultGSV = parseFloat(parts[index + 2]) || 16;
                component.priority = parseInt(parts[index + 3]) || 0;
                component.radius = parseFloat(parts[index + 4]) || 0;
                component.important = parseInt(parts[index + 5]) === 1;
                break;

            case 'Interactable':
                component.radius = parseFloat(parts[index + 1]) || 0.5;
                component.toggleState = parts[index + 2] === 'true';
                break;

            case 'Draggable':
                component.radius = parseFloat(parts[index + 1]) || 1;
                break;

            // Add more as needed...
        }

        return {
            component,
            consumed: mapping.params + 1
        };
    }
}