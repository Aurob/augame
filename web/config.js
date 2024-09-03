var CONFIG = {
    "shaders": [
        {
            "name": "terrain",
            "vertex": "attribute vec4 position; void main() { gl_Position = vec4(position.xyz, 1.0); }",
            "fragment": "terrain.glsl"
        },
        {
            "name": "ui_layer",
            "vertex": "attribute vec4 position; void main() { gl_Position = vec4(position.xyz, 1.0); }",
            "fragment": "ui_layer.glsl"
        },
        {
            "name": "texture",
            "vertex": "vert_tex.glsl",
            "fragment": "frag_tex.glsl"
        },
        {
            "name": "debug_entity",
            "vertex": "test_rgb_v.glsl",
            "fragment": "test_rgb_f.glsl"
        }
    ],
    "textures": [
        {
            "name": "tree",
            "path": "resources/tree11.png"
        },
        {
            "name": "door",
            "path": "resources/door.jpg"
        },
        {
            "name": "smile",
            "path": "resources/smile1.png"
        },
        {
            "name": "treeset1",
            "path": "resources/treeset1.png"
        },
        {
            "name": "treeset2",
            "path": "resources/treeset2.png"
        },
        {
            "name": "ladder_up",
            "path": "resources/ladder_up.png"
        },
        {
            "name": "ladder_down",
            "path": "resources/ladder_down.png"
        },
        {
            "name": "table",
            "path": "resources/table.png"
        },
        {
            "name": "bed1",
            "path": "resources/bed1.png"
        },
        {
            "name": "chair1",
            "path": "resources/chair1.png"
        },
        {
            "name": "shelf1",
            "path": "resources/shelf1.png"
        },
        {
            "name": "wall1",
            "path": "resources/wall1.png"
        },
        {
            "name": "wall2",
            "path": "resources/wall2.png"
        },
        {
            "name": "roomtiles",
            "path": "resources/roomtiles.png"
        },
        {
            "name": "tileset1",
            "path": "resources/tileset1.png"
        }
    
    ],
    "textureGroups": []
};

let roomtiles = {
    'name': 'roomtiles',
    'path': 'resources/roomtiles.png',
    'width': 160,
    'height': 144,
    'tileWidth': 16,
    'tileHeight': 16,
    'parts': []
};

let c = 1; // Start from 1 to match the tilemap checker IDs
for (let j = 0; j < roomtiles.height / roomtiles.tileHeight; j++) {
    for (let i = 0; i < roomtiles.width / roomtiles.tileWidth; i++) {
        let part = {
            'name': `tile${c}`,
            'x': i * roomtiles.tileWidth,
            'y': j * roomtiles.tileHeight,
            'w': roomtiles.tileWidth,
            'h': roomtiles.tileHeight
        }
        roomtiles.parts.push(part);
        c++;
    }
}

let tileset1 = {
    'name': 'tileset1',
    'path': 'resources/tileset1.png',
    'width': 207,
    'height': 153,
    'tileWidth': 8,
    'tileHeight': 8,
    'parts': []
};

c = 1; // Start from 1 to match the tilemap checker IDs
for (let j = 0; j < tileset1.height / tileset1.tileHeight; j++) {
    for (let i = 0; i < tileset1.width / tileset1.tileWidth; i++) {
        let part = {
            'name': `s${c}`,
            'x': i * tileset1.tileWidth,
            'y': j * tileset1.tileHeight,
            'w': tileset1.tileWidth,
            'h': tileset1.tileHeight
        }
        tileset1.parts.push(part);
        c++;
    }
}

CONFIG.textureGroups.push(roomtiles, tileset1);

const actions = ["Idle", "Run"];
const directions = ["Down", "Left", "Right", "Up"];
const textures = [];

actions.forEach((action, actionIndex) => {
    directions.forEach((direction, directionIndex) => {
        const index = actionIndex + 1;
        const texture = {
            "name": `${index}_Template_${action}_${direction}-Sheet`,
            "path": `resources/${index}_Template_${action}_${direction}-Sheet.png`
        };
        textures.push(texture);
    });
});

CONFIG.textures = CONFIG.textures.concat(textures);



