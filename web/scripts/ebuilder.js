class EntityBuilder {
    static componentParameterCounts = {
        texture: 8,
        textureGroupPart: 5,
        position: 4,
        shape: 4,
        color: 5,
        movement: 5, //10,
        interiorPortal: 5,
        inside: 2,
        associated: 3,
        id: 3,
        test: 2,
        renderPriority: 2,
        collidable: 1,
        interior: 2,
        moveable: 1,
        hoverable: 1,
        interactable: 3,
        configurable: 2,
        teleporter: 4,
        teleportable: 2,
        draggable: 2,
        tone: 5,
        ui: 5,
        text: 6,
        player: 1,
        terrain: 1,
        camera: 6,
        cshader: 5,
        meta: 3
    };
    
    static componentParsers = {
        id: (parts, i) => ({ Id: { id: parseInt(parts[i], 10), name: parts[i+1] } }),
        player: () => ({ Player: true }),
        terrain: () => ({ Terrain: true }),
        test: (parts, i) => ({ Test: { value: parts[i] } }),
        text: (parts, i) => ({ Text: { text: parts[i], scale: parseFloat(parts[i+1]), hide: parseInt(parts[i+2]), offsetX: parseFloat(parts[i+3]) || 0.0, offsetY: parseFloat(parts[i+4]) || 0.0}}),
        position: (parts, i) => ({ Position: { x: parseFloat(parts[i]), y: parseFloat(parts[i+1]), z: parseFloat(parts[i+2]) } }),
        shape: (parts, i) => ({ Shape: { size: [parseFloat(parts[i]), parseFloat(parts[i+1]), parseFloat(parts[i+2])] } }),
        color: (parts, i) => {
            const color = { r: parseFloat(parts[i]), g: parseFloat(parts[i+1]), b: parseFloat(parts[i+2]) };
            color.a = parts[i+3] !== undefined ? parseFloat(parts[i+3]) : 1.0;
            return { Color: color };
        },
        renderPriority: (parts, i) => ({ RenderPriority: { priority: parseInt(parts[i], 10) } }),
        collidable: () => ({ Collidable: true }),
        interior: (parts, i) => ({ Interior: { showInside: parts[i] === "1" || parts[i] === "true" || false } }),
        interiorPortal: (parts, i) => ({ InteriorPortal: { A: parseInt(parts[i], 10), B: parseInt(parts[i+1], 10), locked: parts[i+2] === "1", key: parseInt(parts[i+3], 10) } }),
        inside: (parts, i) => ({ Inside: { interiorEntity: parseInt(parts[i], 10), showOutside: parts[i+1] === "true" } }),
        associated: (parts, i) => ({ Associated: { entities: [parseInt(parts[i], 10)] } }),
        texture: (parts, i) => ({
            Texture: {
                name: parts[i],
                scalex: parseFloat(parts[i+1]) || 1.0,
                scaley: parseFloat(parts[i+2]) || 1.0,
                x: parseFloat(parts[i+3]) || 0.0,
                y: parseFloat(parts[i+4]) || 0.0,
                w: parseFloat(parts[i+5]) || 1.0,
                h: parseFloat(parts[i+6]) || 1.0
            }
        }),
        textureGroupPart: (parts, i) => ({
            TextureGroupPart: {
                groupName: parts[i],
                partName: parts[i+1],
                tilex: parseFloat(parts[i+2]) || 0.0,
                tiley: parseFloat(parts[i+3]) || 0.0
            }
        }),
        moveable: () => ({ Moveable: {} }),
        movement: (parts, i) => ({
            Movement: {
                speed: parseFloat(parts[i]),
                // maxSpeed: parseFloat(parts[i+1]),
                // acceleration: { x: parseFloat(parts[i+2]), y: parseFloat(parts[i+3]) },
                // velocity: { x: parseFloat(parts[i+4]), y: parseFloat(parts[i+5]) },
                mass: parseFloat(parts[i+1]),
                restitution: parseFloat(parts[i+2]),
                friction: parseFloat(parts[i+3])
            }
        }),
        hoverable: () => ({ Hoverable: true }),
        interactable: (parts, i) => ({ 
            Interactable: { 
                radius: parts[i] !== undefined ? parseFloat(parts[i]) : 0.5,
                toggleState: parts[i+1] === "true" || parts[i+1] === "1"
            } 
        }),
        configurable: () => ({ Configurable: true }),
        teleporter: (parts, i) => ({ Teleporter: { destination: { x: parseFloat(parts[i]), y: parseFloat(parts[i+1]), z: parseFloat(parts[i+2]) }, interiorEntity: parseInt(parts[i+3], 10) } }),
        teleportable: () => ({ Teleportable: true }),
        draggable: (parts, i) => ({ Draggable: { radius: parseFloat(parts[i]) } }),
        ui: (parts, i) => ({ UIElement: { content: parts[i], visible: parseInt(parts[i+1], 10) === 1 || parts[i+1] === "true", offset: { x: parseFloat(parts[i+2]), y: parseFloat(parts[i+3]) } } }),
        camera: (parts, i) => ({
            Camera: {
                gridSpacing: parseFloat(parts[i]) || 1024.0,
                defaultGSV: parseFloat(parts[i+1]) || 16.0,
                priority: parseInt(parts[i+2], 10) || 0,
                radius: parseFloat(parts[i+3]) || 0.0,
                important: parts[i+4] === "1" || parts[i+4] === "true" || false
            }
        }),
        cshader: (parts, i) => {
            let seed = 0.0;
            if (parts[i+3] !== undefined) {
                if (parts[i+3] === '~') {
                    seed = Math.random() * 1000.0;
                } else {
                    seed = parseFloat(parts[i+3]) || 0.0;
                }
            }
            
            return {
                CustomShader: {
                    shaderName: parts[i],
                    uniformCount: 3,
                    uniforms: [
                        parseFloat(parts[i+1]) || 0.0,  // centerX
                        parseFloat(parts[i+2]) || 0.0,  // centerY
                        seed                             // seed
                    ]
                }
            };
        },
        meta: (parts, i) => {
            // Meta doesn't create components, it returns special metadata
            const type = parts[i];
            let value = parts[i+1];
            
            // Handle slides parsing for format like "text","text","@resources/text/test.txt"
            if (type === 'slides' && value) {
                // Split value by comma first
                const scenesArray = value.split(',').map(scene => scene.trim().replace(/^['"]|['"]$/g, ''));
                
                // Convert array to map with auto-assigned IDs starting from 2
                const slidesMap = {};
                for (let i = 0; i < scenesArray.length; i++) {
                    slidesMap[i + 2] = scenesArray[i]; // Start slide IDs from 2
                }
                value = slidesMap;
            }
            // Handle color parsing for world and void meta tags
            else if ((type === 'world' || type === 'void') && value) {
                // Check for hex color format
                if (value.startsWith('#') && value.length === 7) {
                    const hex = value.substring(1);
                    const r = parseInt(hex.substring(0, 2), 16) / 255.0;
                    const g = parseInt(hex.substring(2, 4), 16) / 255.0;
                    const b = parseInt(hex.substring(4, 6), 16) / 255.0;
                    value = [r, g, b];
                }
                // Check for RGB format (r,g,b)
                else if (value.includes(',')) {
                    const rgbParts = value.split(',').map(s => s.trim());
                    if (rgbParts.length === 3) {
                        const r = Math.max(0, Math.min(255, parseInt(rgbParts[0]))) / 255.0;
                        const g = Math.max(0, Math.min(255, parseInt(rgbParts[1]))) / 255.0;
                        const b = Math.max(0, Math.min(255, parseInt(rgbParts[2]))) / 255.0;
                        value = [r, g, b];
                    }
                }
            }
            // Handle terrain_bounds parsing (minX,minY,maxX,maxY)
            else if (type === 'terrain_bounds' && value) {
                if (value.includes(',')) {
                    const boundsParts = value.split(',').map(s => s.trim());
                    if (boundsParts.length === 4) {
                        value = boundsParts.map(v => parseFloat(v));
                    }
                }
            }
            
            return {
                _meta: {
                    type: type,
                    value: value
                }
            };
        },
    };

    constructor() {
        this.components = {};
        this.metaData = {};
    }
    
    parseInput(input) {
        const parts = [];
        let i = 0;
        let inQuotes = false;
        let currentQuote = '';
        
        // Split the input handling quoted strings
        const tokens = input.split(/\s+/).filter(part => part.length > 0);
        for (const token of tokens) {
            if (inQuotes) {
                currentQuote += ' ' + token;
                if (token.endsWith('"')) {
                    // End of quoted string
                    parts.push(currentQuote.substring(0, currentQuote.length - 1));
                    inQuotes = false;
                    currentQuote = '';
                }
            } else if (token.startsWith('"') && !token.endsWith('"')) {
                // Start of quoted string
                inQuotes = true;
                currentQuote = token.substring(1);
            } else if (token.startsWith('"') && token.endsWith('"') && token.length > 1) {
                // Complete quoted string in one token
                parts.push(token.substring(1, token.length - 1));
            } else {
                parts.push(token);
            }
        }
        
        i = 0;
        while (i < parts.length) {
            const componentName = parts[i];
            const parser = EntityBuilder.componentParsers[componentName];
            if (parser) {
                const result = parser(parts, i + 1);
                if (result._meta) {
                    // Handle meta data specially
                    this.metaData[result._meta.type] = result._meta.value;
                } else {
                    Object.assign(this.components, result);
                }
                i += this.getComponentParameterCount(componentName, result);
            } else {
                console.warn(`Unknown component or parser not implemented: ${componentName}`);
                i++;
            }
        }
        return this;
    }

    getComponentParameterCount(componentName, result) {
        if (componentName in EntityBuilder.componentParameterCounts) {
            return EntityBuilder.componentParameterCounts[componentName];
        }
        const componentData = result[Object.keys(result)[0]];
        return Object.keys(componentData).length + 1;
    }

    build() {
        // If this is a meta-only entity, return meta data instead
        if (Object.keys(this.components).length === 0 && Object.keys(this.metaData).length > 0) {
            return {
                meta: this.metaData
            };
        }
        
        return {
            New: true,
            Components: this.components
        };
    }
}

// Export ComponentSchemas and MetaSchemas for use in editor
export const ComponentSchemas = [
    { type: 'Position', configKey: 'position', fields: [
        { name: 'x', type: 'number', default: 0, label: 'X' },
        { name: 'y', type: 'number', default: 0, label: 'Y' },
        { name: 'z', type: 'number', default: 0, label: 'Z' }
    ]},
    { type: 'Shape', configKey: 'shape', fields: [
        { name: 'w', type: 'number', default: 1, label: 'Width' },
        { name: 'h', type: 'number', default: 1, label: 'Height' },
        { name: 'd', type: 'number', default: 0, label: 'Depth' }
    ]},
    { type: 'Color', configKey: 'color', fields: [
        { name: 'r', type: 'number', default: 255, label: 'Red' },
        { name: 'g', type: 'number', default: 255, label: 'Green' },
        { name: 'b', type: 'number', default: 255, label: 'Blue' },
        { name: 'a', type: 'number', default: 1, label: 'Alpha' }
    ]},
    { type: 'RenderPriority', configKey: 'renderPriority', fields: [
        { name: 'z', type: 'number', default: 0, label: 'Z-Order' }
    ]},
    { type: 'Player', configKey: 'player', fields: [] },
    { type: 'Collidable', configKey: 'collidable', fields: [] },
    { type: 'Moveable', configKey: 'moveable', fields: [] },
    { type: 'Hoverable', configKey: 'hoverable', fields: [] },
    { type: 'Terrain', configKey: 'terrain', fields: [] },
    { type: 'Interior', configKey: 'interior', fields: [
        { name: 'showInside', type: 'boolean', default: true, label: 'Show Inside' }
    ]},
    { type: 'Inside', configKey: 'inside', fields: [
        { name: 'insideId', type: 'number', default: 0, label: 'Interior ID' }
    ]},
    { type: 'Text', configKey: 'text', fields: [
        { name: 'text', type: 'string', default: '', label: 'Text' },
        { name: 'scale', type: 'number', default: 1, label: 'Scale' },
        { name: 'hidden', type: 'boolean', default: false, label: 'Hidden' },
        { name: 'offsetX', type: 'number', default: 0, label: 'Offset X' },
        { name: 'offsetY', type: 'number', default: 0, label: 'Offset Y' }
    ]},
    { type: 'Movement', configKey: 'movement', fields: [
        { name: 'speed', type: 'number', default: 0, label: 'Speed' },
        { name: 'mass', type: 'number', default: 1, label: 'Mass' },
        { name: 'restitution', type: 'number', default: 0, label: 'Restitution' },
        { name: 'friction', type: 'number', default: 0, label: 'Friction' }
    ]},
    { type: 'InteriorPortal', configKey: 'interiorPortal', fields: [
        { name: 'A', type: 'number', default: 0, label: 'Interior A' },
        { name: 'B', type: 'number', default: 0, label: 'Interior B' },
        { name: 'locked', type: 'boolean', default: false, label: 'Locked' },
        { name: 'key', type: 'number', default: -1, label: 'Key ID' }
    ]},
    { type: 'Camera', configKey: 'camera', fields: [
        { name: 'gridSpacing', type: 'number', default: 1024, label: 'Grid Spacing' },
        { name: 'defaultGSV', type: 'number', default: 16, label: 'Default GSV' },
        { name: 'priority', type: 'number', default: 0, label: 'Priority' },
        { name: 'radius', type: 'number', default: 0, label: 'Radius' },
        { name: 'important', type: 'boolean', default: false, label: 'Important' }
    ]},
    { type: 'Texture', configKey: 'texture', fields: [
        { name: 'name', type: 'string', default: '', label: 'Texture Name' },
        { name: 'scalex', type: 'number', default: 1, label: 'Scale X' },
        { name: 'scaley', type: 'number', default: 1, label: 'Scale Y' },
        { name: 'x', type: 'number', default: 0, label: 'X' },
        { name: 'y', type: 'number', default: 0, label: 'Y' },
        { name: 'w', type: 'number', default: 1, label: 'Width' },
        { name: 'h', type: 'number', default: 1, label: 'Height' }
    ]},
    { type: 'TextureGroupPart', configKey: 'textureGroupPart', fields: [
        { name: 'groupName', type: 'string', default: '', label: 'Group Name' },
        { name: 'partName', type: 'string', default: '', label: 'Part Name' },
        { name: 'tilex', type: 'number', default: 0, label: 'Tile X' },
        { name: 'tiley', type: 'number', default: 0, label: 'Tile Y' }
    ]},
    { type: 'Interactable', configKey: 'interactable', fields: [
        { name: 'radius', type: 'number', default: 0.5, label: 'Radius' },
        { name: 'toggleState', type: 'boolean', default: false, label: 'Toggle State' }
    ]},
    { type: 'Teleporter', configKey: 'teleporter', fields: [
        { name: 'destX', type: 'number', default: 0, label: 'Dest X' },
        { name: 'destY', type: 'number', default: 0, label: 'Dest Y' },
        { name: 'destZ', type: 'number', default: 0, label: 'Dest Z' },
        { name: 'interiorEntity', type: 'number', default: 0, label: 'Interior ID' }
    ]},
    { type: 'Teleportable', configKey: 'teleportable', fields: [] },
    { type: 'Draggable', configKey: 'draggable', fields: [
        { name: 'radius', type: 'number', default: 1, label: 'Radius' }
    ]},
    { type: 'CustomShader', configKey: 'cshader', fields: [
        { name: 'shaderName', type: 'string', default: '', label: 'Shader Name' },
        { name: 'centerX', type: 'number', default: 0, label: 'Center X' },
        { name: 'centerY', type: 'number', default: 0, label: 'Center Y' },
        { name: 'seed', type: 'number', default: 0, label: 'Seed' }
    ]}
];

export const MetaSchemas = {
    scene: { type: 'string', displayName: 'Scene Name', default: 'world1', description: 'Unique identifier for this scene' },
    title: { type: 'string', displayName: 'Title', default: 'Default World', description: 'Display title for the world' },
    description: { type: 'string', displayName: 'Description', default: 'This is the default world configuration file for the game.', description: 'World description' },
    seed: { type: 'string', displayName: 'Seed', default: 'Hello World', description: 'World generation seed' },
    author: { type: 'string', displayName: 'Author', default: 'rau', description: 'World creator name' },
    font: { type: 'string', displayName: 'Font', default: 'HomeVideo-Regular.ttf', description: 'Font file path' },
    world: { type: 'string', displayName: 'World Shader', default: 'terrain', description: 'Shader/color for outside area (terrain/tiles/#hex/r,g,b)' },
    void: { type: 'string', displayName: 'Void Color', default: '#000000', description: 'Color for inside area (#hex/r,g,b)' },
    terrain_bounds: { type: 'string', displayName: 'Terrain Bounds', default: '', description: 'Terrain area limits (minX,minY,maxX,maxY). Empty=infinite, 0,0,0,0=none, -1,-1,1,1=2x2 area' },
    start_menu: { type: 'string', displayName: 'Start Menu Text', default: 'Demo', description: 'Text shown on start screen' },
    pause_menu: { type: 'string', displayName: 'Pause Menu Text', default: 'Paused', description: 'Text shown on pause screen' },
    slides: { type: 'object', displayName: 'Slides', default: {}, description: 'Slide definitions (id:text format)' }
};

// Export EntityBuilder as default
export default EntityBuilder;
