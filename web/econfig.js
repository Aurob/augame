ECONFIG = {
    "Entities": [
        {
            "New": true,
            "Components": {
                "Id": 0,
                "Test": { "value": "Room" },
                "Position": { "x": 0, "y": 10, "z": -1 },
                "Shape": { "size": [15, 15, 1] },
                "Color": { "r": 0.5, "g": 0.5, "b": 0.5, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Interior": { "hideInside": true }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 1,
                "Test": { "value": "Door" },
                "Position": { "x": 2, "y": 25, "z": .1 },
                "Shape": { "size": [1, 0.1, 1] },
                "Color": { "r": 0.6, "g": 0.3, "b": 0.1, "a": 1.0 },
                "RenderPriority": { "priority": 2 },
                "Collidable": true,
                "InteriorPortal": { "A": 0, "B": -1 },
                "Inside": { "interiorEntity": 0 },
                "Associated": { "entities": [0] }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 2,
                "Test": { "value": "Wall" },
                "Position": { "x": 0, "y": 10, "z": 0.5 },
                "Shape": { "size": [15, 1, 2] },
                "Color": { "r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Inside": { "interiorEntity": 0 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 3,
                "Test": { "value": "Wall" },
                "Position": { "x": 5, "y": 12, "z": 0.5 },
                "Shape": { "size": [1, 6, 2] },
                "Color": { "r": 0.05, "g": 0.1, "b": 0.2, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Inside": { "interiorEntity": 0 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 4,
                "Test": { "value": "Wall" },
                "Position": { "x": 0, "y": 24, "z": 0.5 },
                "Shape": { "size": [15, 1, 2] },
                "Color": { "r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Inside": { "interiorEntity": 0 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 5,
                "Test": { "value": "Other" },
                "Position": { "x": 0, "y": 12, "z": 2.1 },
                "Shape": { "size": [1, 1, 1] },
                "Color": { "r": 0.1, "g": 0.2, "b": 0.3, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Inside": { "interiorEntity": 0 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 6,
                "Test": { "value": "Room" },
                "Position": { "x": 9, "y": 10, "z": .2 },
                "Shape": { "size": [6, 6, 0] },
                "Color": { "r": 0.15, "g": 0.15, "b": 0.15, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Interior": { "hideInside": true },
                "Inside": { "interiorEntity": 0 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 7,
                "Test": { "value": "Door" },
                "Position": { "x": 12, "y": 16, "z": 0 },
                "Shape": { "size": [1, 0.1, 1] },
                "Color": { "r": 0.6, "g": 0.3, "b": 0.1, "a": 1.0 },
                "RenderPriority": { "priority": 2 },
                "Collidable": true,
                "InteriorPortal": { "A": 6, "B": 0 },
                "Inside": { "interiorEntity": 0 },
                "Texture": { "name": "door" },
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 8,
                "Test": { "value": "Wall" },
                "Position": { "x": 9, "y": 15, "z": 0.1 },
                "Shape": { "size": [6, 1, 2] },
                "Color": { "r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Inside": { "interiorEntity": 0, "showOutside": true }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 9,
                "Test": { "value": "Other" },
                "Position": { "x": -17, "y": 12, "z": 0 },
                "Shape": { "size": [1, 1, .1] },
                "Color": { "r": 0.1, "g": 0.3, "b": 0.8, "a": 1 },
                "RenderPriority": { "priority": 0 },
                "Collidable": true,
                "Moveable": {},
                "Movement": {
                    "speed": 1,
                    "maxSpeed": 10,
                    "acceleration": { "x": 0, "y": 0 },
                    "velocity": { "x": 0, "y": 0 },
                    "friction": 1, "mass": 1, "drag": .1
                },
                "Hoverable": true,
                "Interactable": true
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 10,
                "Test": { "value": "Table" },
                "Position": { "x": 12, "y": 11, "z": .1 },
                "Shape": { "size": [1.5, 1.5, 1] },
                "Color": { "r": 0.5, "g": 0.25, "b": 0.1, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Collidable": true,
                "Texture": {
                    "name": "table",
                    "scalex": 1.0,
                    "scaley": 1.0,
                    "x": 1,
                    "y": 1,
                    "w": 11,
                    "h": 11
                },
                "Inside": { "interiorEntity": 6 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 11,
                "Test": { "value": "Chair1" },
                "Position": { "x": 13, "y": 13, "z": -.1 },
                "Shape": { "size": [.75, .75, .5] },
                "Color": { "r": 0.7, "g": 0.4, "b": 0.2, "a": 1.0 },
                "RenderPriority": { "priority": 0 },
                "Texture": {
                    "name": "chair1",
                    "scalex": 1.0,
                    "scaley": 1.0,
                    "x": 0,
                    "y": 0,
                    "w": 1,
                    "h": 1
                },
                "Inside": { "interiorEntity": 6 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 12,
                "Test": { "value": "Chair2" },
                "Position": { "x": 14, "y": 12, "z": -.1 },
                "Shape": { "size": [.75, .75, .5] },
                "Color": { "r": 0.7, "g": 0.4, "b": 0.2, "a": 1.0 },
                "RenderPriority": { "priority": 0 },
                "Texture": {
                    "name": "chair1",
                    "scalex": 1.0,
                    "scaley": 1.0,
                    "x": 0,
                    "y": 0,
                    "w": 1,
                    "h": 1
                },
                "Inside": { "interiorEntity": 6 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 14,
                "Test": { "value": "Bed" },
                "Position": { "x": 9, "y": 14, "z": .1 },
                "Shape": { "size": [2, 2, 1] },
                "Color": { "r": 0.4, "g": 0.3, "b": 0.2, "a": 1.0 },
                "RenderPriority": { "priority": 1 },
                "Texture": {
                    "name": "bed1",
                    "scalex": 1.0,
                    "scaley": 1.0,
                    "x": 0,
                    "y": 0,
                    "w": 1,
                    "h": 1
                },
                "Inside": { "interiorEntity": 6 }
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 13,
                "Test": { "value": "Shelf" },
                "Position": { "x": 9, "y": 8, "z": .2 },
                "Shape": { "size": [2, 2, 2] },
                "Color": { "r": 0.3, "g": 0.2, "b": 0.1, "a": 1.0 },
                "RenderPriority": { "priority": 2 },
                "Texture": {
                    "name": "shelf1",
                    "scalex": 1.0,
                    "scaley": 1.0,
                    "x": 0,
                    "y": 0,
                    "w": 1,
                    "h": 1
                },
                "Inside": { "interiorEntity": 6 },
                
            }
        }
    ]
}

// Create 4 rows, each with 12 books, and place on the shelf with random colors and sizes
for (var row = 3; row >= 0; row--) {
    for (var col = 0; col < 13; col++) {
        const width = 0.05 + Math.random() * 0.04; // Random width between 0.08 and 0.12
        const height = 0.15; // Random height between 0.2 and 0.3
        ECONFIG.Entities.push({
            "New": true,
            "Components": {
                "Id": 15 + ((3 - row) * 12) + col,
                "Test": { "value": "Book" },
                "Position": { 
                    "x": 9.3 + (col * 0.11), 
                    "y": 8.64 + ((3 - row) * height * 3), 
                    "z": row },
                "Shape": { "size": [width, height, .1] },
                "Color": (function () {
                    const colors = [
                        { "r": 0.36, "g": 0.25, "b": 0.20, "a": 1.0 }, // Earthy Brown
                        { "r": 0.40, "g": 0.55, "b": 0.40, "a": 1.0 }, // Earthy Green
                        { "r": 0.55, "g": 0.27, "b": 0.27, "a": 1.0 }, // Earthy Red
                        { "r": 0.33, "g": 0.33, "b": 0.33, "a": 1.0 }  // Earthy Gray
                    ];
                    return colors[Math.floor(Math.random() * colors.length)];
                })(),
                "RenderPriority": { "priority": 3 },
                "Inside": { "interiorEntity": 6 },
                "Interactable": true,
                "Hoverable": true,
            }
        });
    }
}