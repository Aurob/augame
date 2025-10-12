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
                { type: "Shape", w: 1, h: 1, d: 0 },
                { type: "Player" },
                { type: "Moveable" },
                { type: "Movement", speed: 1000, mass: 1, restitution: 0, friction: 0.1 },
                { type: "Collidable" }
            ]
        }],
        created: "2024-01-01T00:00:00.000Z"
    },
    {
        "name": "locked_room",
        "entities": [
            {
                "id": 20,
                "name": "entity20",
                "components": [
                    {
                        "type": "Position",
                        "x": 20,
                        "y": 13.5,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 20,
                        "h": 20,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 100,
                        "g": 80,
                        "b": 60,
                        "a": 1
                    },
                    {
                        "type": "RenderPriority",
                        "z": -1
                    },
                    {
                        "type": "Interior",
                        "showInside": true
                    },
                    {
                        "type": "Collidable"
                    }
                ]
            },
            {
                "id": 21,
                "name": "entity21",
                "components": [
                    {
                        "type": "Position",
                        "x": 20,
                        "y": 33.5,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 20,
                        "h": 1,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 128,
                        "g": 128,
                        "b": 128,
                        "a": 1
                    }
                ]
            },
            {
                "id": 22,
                "name": "entity22",
                "components": [
                    {
                        "type": "Position",
                        "x": 20,
                        "y": 12.5,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 20,
                        "h": 1,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 128,
                        "g": 128,
                        "b": 128,
                        "a": 1
                    }
                ]
            },
            {
                "id": 23,
                "name": "entity23",
                "components": [
                    {
                        "type": "Position",
                        "x": 19,
                        "y": 13.5,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 1,
                        "h": 20,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 128,
                        "g": 128,
                        "b": 128,
                        "a": 1
                    }
                ]
            },
            {
                "id": 24,
                "name": "entity24",
                "components": [
                    {
                        "type": "Position",
                        "x": 40,
                        "y": 13.5,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 1,
                        "h": 20,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 128,
                        "g": 128,
                        "b": 128,
                        "a": 1
                    }
                ]
            },
            {
                "id": 28,
                "name": "entity28",
                "components": [
                    {
                        "type": "Position",
                        "x": 29.5,
                        "y": 32,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 1,
                        "h": 1,
                        "d": 0
                    },
                    {
                        "type": "RenderPriority",
                        "z": 0
                    },
                    {
                        "type": "Moveable"
                    },
                    {
                        "type": "Movement",
                        "speed": 0,
                        "mass": 1,
                        "restitution": 0,
                        "friction": 0
                    },
                    {
                        "type": "Texture",
                        "name": "key",
                        "scalex": 1,
                        "scaley": 1,
                        "x": 0,
                        "y": 0,
                        "w": 1,
                        "h": 1
                    },
                    {
                        "type": "Inside",
                        "insideId": 20
                    },
                    {
                        "type": "Hoverable"
                    },
                    {
                        "type": "Interactable",
                        "radius": 0.5,
                        "toggleState": false
                    },
                    {
                        "type": "Collidable"
                    }
                ]
            },
            {
                "id": 25,
                "name": "entity25",
                "components": [
                    {
                        "type": "Position",
                        "x": 28.5,
                        "y": 12.4,
                        "z": 0
                    },
                    {
                        "type": "Shape",
                        "w": 2,
                        "h": 1.2,
                        "d": 0
                    },
                    {
                        "type": "Color",
                        "r": 139,
                        "g": 255,
                        "b": 19,
                        "a": 1
                    },
                    {
                        "type": "InteriorPortal",
                        "A": 20,
                        "B": -1,
                        "locked": true,
                        "key": 28
                    },
                    {
                        "type": "Inside",
                        "insideId": 20
                    },
                    {
                        "type": "CustomShader",
                        "shaderName": "",
                        "centerX": 0,
                        "centerY": 0,
                        "seed": 0
                    }
                ]
            }
        ],
        "created": "2025-10-11T19:25:28.194Z"
    }
];