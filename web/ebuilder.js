
class EntityBuilder {
    static componentParameterCounts = {
        texture: 8,
        position: 4,
        shape: 4,
        color: 4,
        movement: 10,
        interiorPortal: 3,
        inside: 2,
        associated: 3,
        id: 2,
        test: 2,
        renderPriority: 2,
        collidable: 1,
        interior: 1,
        moveable: 1,
        hoverable: 1,
        interactable: 1
    };
    
    static componentParsers = {
        id: (parts, i) => ({ Id: parseInt(parts[i], 10) }),
        test: (parts, i) => ({ Test: { value: parts[i] } }),
        position: (parts, i) => ({ Position: { x: parseFloat(parts[i]), y: parseFloat(parts[i+1]), z: parseFloat(parts[i+2]) } }),
        shape: (parts, i) => ({ Shape: { size: [parseFloat(parts[i]), parseFloat(parts[i+1]), parseFloat(parts[i+2])] } }),
        color: (parts, i) => {
            const color = { r: parseFloat(parts[i]), g: parseFloat(parts[i+1]), b: parseFloat(parts[i+2]) };
            color.a = 1.0; //parts[i+3] !== undefined ? parseFloat(parts[i+3]) : 1.0;
            return { Color: color };
        },
        renderPriority: (parts, i) => ({ RenderPriority: { priority: parseInt(parts[i], 10) } }),
        collidable: () => ({ Collidable: true }),
        interior: () => ({ Interior: { hideInside: true } }),
        interiorPortal: (parts, i) => ({ InteriorPortal: { A: parseInt(parts[i], 10), B: parseInt(parts[i+1], 10) } }),
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
        moveable: () => ({ Moveable: {} }),
        movement: (parts, i) => ({
            Movement: {
                speed: parseFloat(parts[i]),
                maxSpeed: parseFloat(parts[i+1]),
                acceleration: { x: parseFloat(parts[i+2]), y: parseFloat(parts[i+3]) },
                velocity: { x: parseFloat(parts[i+4]), y: parseFloat(parts[i+5]) },
                friction: parseFloat(parts[i+6]),
                mass: parseFloat(parts[i+7]),
                drag: parseFloat(parts[i+8])
            }
        }),
        hoverable: () => ({ Hoverable: true }),
        interactable: () => ({ Interactable: true })
    };

    constructor() {
        this.components = {};
    }

    parseInput(input) {
        const parts = input.split(/\s+/);
        let i = 0;
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