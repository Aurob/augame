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
        interior: 1,
        moveable: 1,
        hoverable: 1,
        interactable: 3,
        configurable: 2,
        teleporter: 4,
        teleportable: 2,
        draggable: 2,
        tone: 5,
        ui: 5,
        text: 4,
        player: 1,
        world: 1,
        terrain: 1,
        camera: 6
    };
    
    static componentParsers = {
        id: (parts, i) => ({ Id: { id: parseInt(parts[i], 10), name: parts[i+1] } }),
        player: () => ({ Player: true }),
        terrain: () => ({ Terrain: true }),
        world: (parts, i) => ({ World: true }),
        test: (parts, i) => ({ Test: { value: parts[i] } }),
        text: (parts, i) => ({ Text: { text: parts[i], scale: parseFloat(parts[i+1]), hide: parseInt(parts[i+2])}}),
        position: (parts, i) => ({ Position: { x: parseFloat(parts[i]), y: parseFloat(parts[i+1]), z: parseFloat(parts[i+2]) } }),
        shape: (parts, i) => ({ Shape: { size: [parseFloat(parts[i]), parseFloat(parts[i+1]), parseFloat(parts[i+2])] } }),
        color: (parts, i) => {
            const color = { r: parseFloat(parts[i]), g: parseFloat(parts[i+1]), b: parseFloat(parts[i+2]) };
            color.a = parts[i+3] !== undefined ? parseFloat(parts[i+3]) : 1.0;
            return { Color: color };
        },
        renderPriority: (parts, i) => ({ RenderPriority: { priority: parseInt(parts[i], 10) } }),
        collidable: () => ({ Collidable: true }),
        interior: () => ({ Interior: { hideInside: true } }),
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
    };

    constructor() {
        this.components = {};
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
                Object.assign(this.components, result);
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
        return {
            New: true,
            Components: this.components
        };
    }
}
