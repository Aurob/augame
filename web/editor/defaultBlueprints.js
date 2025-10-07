// Default blueprints for AuGame editor
// These are loaded automatically when the editor starts

export const defaultBlueprints = [
    {
        name: "player",
        entities: [{
            id: 0,
            name: "player",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 2, h: 2, d: 0 },
                { type: "Color", r: 0, g: 255, b: 0, a: 1 },
                { type: "Player" },
                { type: "Moveable" },
                { type: "Movement", speed: 10, mass: 1, restitution: 0, friction: 0.1 }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "wall_horizontal",
        entities: [{
            id: 0,
            name: "wall",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 10, h: 1, d: 0 },
                { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                { type: "Collidable" }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "wall_vertical",
        entities: [{
            id: 0,
            name: "wall",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 1, h: 10, d: 0 },
                { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                { type: "Collidable" }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "door",
        entities: [{
            id: 0,
            name: "door",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 2, h: 3, d: 0 },
                { type: "Color", r: 139, g: 69, b: 19, a: 1 },
                { type: "Interactable", radius: 2.0, toggleState: false },
                { type: "Hoverable" },
                { type: "InteriorPortal", A: 1, B: 2, locked: false, key: -1 }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "room_basic",
        entities: [
            {
                id: 1,
                name: "floor",
                components: [
                    { type: "Position", x: 0, y: 0, z: 0 },
                    { type: "Shape", w: 20, h: 20, d: 0 },
                    { type: "Color", r: 100, g: 80, b: 60, a: 1 },
                    { type: "RenderPriority", z: -1 }
                ]
            },
            {
                id: 2,
                name: "wall_top",
                components: [
                    { type: "Position", x: 0, y: -10, z: 0 },
                    { type: "Shape", w: 20, h: 1, d: 0 },
                    { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                    { type: "Collidable" }
                ]
            },
            {
                id: 3,
                name: "wall_bottom",
                components: [
                    { type: "Position", x: 0, y: 10, z: 0 },
                    { type: "Shape", w: 20, h: 1, d: 0 },
                    { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                    { type: "Collidable" }
                ]
            },
            {
                id: 4,
                name: "wall_left",
                components: [
                    { type: "Position", x: -10, y: 0, z: 0 },
                    { type: "Shape", w: 1, h: 20, d: 0 },
                    { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                    { type: "Collidable" }
                ]
            },
            {
                id: 5,
                name: "wall_right",
                components: [
                    { type: "Position", x: 10, y: 0, z: 0 },
                    { type: "Shape", w: 1, h: 20, d: 0 },
                    { type: "Color", r: 128, g: 128, b: 128, a: 1 },
                    { type: "Collidable" }
                ]
            }
        ],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "npc",
        entities: [{
            id: 0,
            name: "npc",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 2, h: 2, d: 0 },
                { type: "Color", r: 255, g: 0, b: 255, a: 1 },
                { type: "Text", text: "Hello!", scale: 1, hidden: false, offsetX: 0, offsetY: -2 },
                { type: "Hoverable" },
                { type: "Interactable", radius: 3.0, toggleState: false }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        name: "trigger_zone",
        entities: [{
            id: 0,
            name: "trigger",
            components: [
                { type: "Position", x: 0, y: 0, z: 0 },
                { type: "Shape", w: 5, h: 5, d: 0 },
                { type: "Color", r: 255, g: 255, b: 0, a: 0.3 },
                { type: "Interactable", radius: 5.0, toggleState: false }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    }
];