ECONFIG = {
    "Entities": [
        {
            "New": true,
            "Components": {
                "Id": 0,
                "Test": {"value": "Room"},
                "Position": {"x": 0, "y": 10, "z": 0},
                "Shape": {"size": [15, 15, 1]},
                "Color": {"r": 0.5, "g": 0.5, "b": 0.5, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Interior": {"hideInside":true}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 1,
                "Test": {"value": "Door"},
                "Position": {"x": 2, "y": 25, "z": .1},
                "Shape": {"size": [1, 0.1, 1]},
                "Color": {"r": 0.6, "g": 0.3, "b": 0.1, "a": 1.0},
                "RenderPriority": {"priority": 2},
                "Collidable": true,
                "InteriorPortal": {"A": 0, "B": -1},
                "Inside": {"interiorEntity": 0},
                "Associated": {"entities": [0]}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 2,
                "Test": {"value": "Wall"},
                "Position": {"x": 0, "y": 10, "z": 0.5},
                "Shape": {"size": [15, 1, 2]},
                "Color": {"r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 3,
                "Test": {"value": "Wall"},
                "Position": {"x": 5, "y": 12, "z": 0.5},
                "Shape": {"size": [1, 6, 2]},
                "Color": {"r": 0.05, "g": 0.1, "b": 0.2, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 4,
                "Test": {"value": "Wall"},
                "Position": {"x": 0, "y": 24, "z": 0.5},
                "Shape": {"size": [15, 1, 2]},
                "Color": {"r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 5,
                "Test": {"value": "Other"},
                "Position": {"x": 0, "y": 12, "z": 2.1},
                "Shape": {"size": [1, 1, 1]},
                "Color": {"r": 0.1, "g": 0.2, "b": 0.3, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 6,
                "Test": {"value": "Room"},
                "Position": {"x": 9, "y": 10, "z": 0.1},
                "Shape": {"size": [6, 6, 1]},
                "Color": {"r": 0.15, "g": 0.15, "b": 0.15, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Interior": {"hideInside":true},
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 7,
                "Test": {"value": "Door"},
                "Position": {"x": 8.9, "y": 13, "z": 0},
                "Shape": {"size": [1, 0.1, 1]},
                "Color": {"r": 0.6, "g": 0.3, "b": 0.1, "a": 1.0},
                "RenderPriority": {"priority": 2},
                "Collidable": true,
                "InteriorPortal": {"A": 6, "B": 0},
                "Inside": {"interiorEntity": 0}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 8,
                "Test": {"value": "Wall"},
                "Position": {"x": 9, "y": 15, "z": 0.1},
                "Shape": {"size": [6, 1, 2]},
                "Color": {"r": 0.1, "g": 0.2, "b": 0.4, "a": 1.0},
                "RenderPriority": {"priority": 1},
                "Collidable": true,
                "Inside": {"interiorEntity": 0, "showOutside":true}
            }
        },
        {
            "New": true,
            "Components": {
                "Id": 9,
                "Test": {"value": "Other"},
                "Position": {"x": -17, "y": 12, "z": 0},
                "Shape": {"size": [1, 1, .1]},
                "Color": {"r": 0.1, "g": 0.3, "b": 0.8, "a": 1},
                "RenderPriority": {"priority": 0},
                "Collidable": true,
                "Moveable": {},
                "Movement": {
                    "speed": 1, 
                    "maxSpeed": 10, 
                    "acceleration": {"x": 0, "y": 0}, 
                    "velocity": {"x": 0, "y": 0}, 
                    "friction": 1, "mass": 1, "drag": .1},
                "Hoverable": true,
                "Interactable": true
            }
        }
    ]
}
