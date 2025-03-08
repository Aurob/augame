var CONFIG = {
    "shaders": [
        {
            "name": "terrain",
            "vertex": "attribute vec4 position; void main() { gl_Position = vec4(position.xyz, 1.0); }",
            "fragment": "terrain.glsl"
        },
        {
            "name": "ui_layer", 
            "vertex": "ui_layer_v.glsl",
            "fragment": "ui_layer_f.glsl"
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
    "textures": [],
    "textureGroups": []
};

// Helper functions to generate texture configs
function createBasicTexture(name, path) {
    return {
        "name": name,
        "path": `resources/textures/${path}`
    };
}

function createTilesetGroup(name, path, width, height, tileWidth, tileHeight) {
    const group = {
        'name': name,
        'path': `resources/textures/${path}`,
        'width': width,
        'height': height,
        'tileWidth': tileWidth,
        'tileHeight': tileHeight,
        'parts': []
    };

    let counter = 1;
    for (let j = 0; j < height / tileHeight; j++) {
        for (let i = 0; i < width / tileWidth; i++) {
            group.parts.push({
                'name': `s${counter}`,
                'x': i * tileWidth,
                'y': j * tileHeight,
                'w': tileWidth,
                'h': tileHeight
            });
            counter++;
        }
    }

    return group;
}

function createJsonTileset(name, path, objectMap) {
    const group = {
        'name': name,
        'path': `resources/textures/${path}`,
        'parts': []
    };

    for (const [key, value] of Object.entries(objectMap)) {
        group.parts.push({
            'name': key,
            'x': value.x,
            'y': value.y,
            'w': value.width,
            'h': value.height
        });
    }

    return group;
}

function createSpriteSheetTextures(prefix, actions, directions) {
    return actions.flatMap((action, actionIndex) => 
        directions.map((direction) => {
            const index = actionIndex + 1;
            return createBasicTexture(
                `${index}_${prefix}_${action}_${direction}-Sheet`,
                `${index}_${prefix}_${action}_${direction}-Sheet.png`
            );
        })
    );
}

// Add basic textures
const basicTextures = [
    // ['tileset1', 'tileset1.png'],
    // ['slime', 'SlimeGreenIdle.png'],
    // ['props', 'Props.png'],
    ['trumpet', 'trumpet.png'],
    ['room1', 'Room_Builder_free_32x32.png'],
    // ['doors', 'doors.png'],
    ['instruments', 'instruments.png'],
    // ['ladders', 'ladders.png'],
    ['font', '42dotSans-Regular-ttf.png']
].map(([name, path]) => createBasicTexture(name, path));

CONFIG.textures = basicTextures;

// Add texture groups
const tilesetGroups = [
    ['tileset1', 'tileset1.png', 207, 153, 8, 8],
    // ['slime2', 'SlimeGreenIdle.png', 1430, 715, 32, 32],
    ['room1', 'Room_Builder_free_32x32.png', 544, 736, 32, 32],
    ['font', '42dotSans-Regular-ttf.png', 330, 273, 33, 39],
    ['instruments', 'instruments.png', 512, 544, 32, 32]
].map(([name, path, width, height, tileWidth, tileHeight]) => 
    createTilesetGroup(name, path, width, height, tileWidth, tileHeight)
);

CONFIG.textureGroups.push(...tilesetGroups);

// // // Add JSON tileset
const jsonTileset = createJsonTileset('font', '42dotSans-Regular-ttf.png', FONT_DATA);
CONFIG.textureGroups.push(jsonTileset);

// Add sprite sheet textures
const playerTextures = createSpriteSheetTextures(
    'Template',
    ['Idle', 'Run'],
    ['Down', 'Left', 'Right', 'Up']
);

CONFIG.textures = CONFIG.textures.concat(playerTextures);