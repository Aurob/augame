var CONFIG = {
    "shaders": [
        {
            "name": "grid",
            "vertex": "/web/resources/cshaders/grid_v.glsl",
            "fragment": "/web/resources/cshaders/grid_f.glsl"
        },
        {
            "name": "colorquads",
            "vertex": "/web/resources/cshaders/colorquads_v.glsl",
            "fragment": "/web/resources/cshaders/colorquads_f.glsl"
        },
        {
            "name": "terrainmap",
            "vertex": "/web/resources/cshaders/terrain_v.glsl",
            "fragment": "/web/resources/cshaders/terrain_f.glsl"
        },
        {
            "name": "water1",
            "vertex": "/web/resources/cshaders/water1_v.glsl",
            "fragment": "/web/resources/cshaders/water1_f.glsl"
        },
        {
            "name": "bookshelf",
            "vertex": "/web/resources/cshaders/bookshelf_v.glsl",
            "fragment": "/web/resources/cshaders/bookshelf_f.glsl"
        },
        {
            "name": "carpet",
            "vertex": "/web/resources/cshaders/carpet_v.glsl",
            "fragment": "/web/resources/cshaders/carpet_f.glsl"
        }
    ],
    "textures": [],
    "textureGroups": []
};

// Helper functions to generate texture configs
function createBasicTexture(name, path) {
    return {
        "name": name,
        "path": `web/resources/textures/${path}`
    };
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

function createTilesetGroup(name, path, width, height, tileWidth, tileHeight) {
    const group = {
        'name': name,
        'path': `web/resources/textures/${path}`,
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

// Add sprite sheet textures
const playerTextures = createSpriteSheetTextures(
    'Template',
    ['Idle', 'Run'],
    ['Down', 'Left', 'Right', 'Up']
);

const textures = [
    createBasicTexture("hand_open", "hand_thin_small_open.png"),
    createBasicTexture("hand_closed", "hand_thin_small_closed.png"),
    createBasicTexture("key", "key.png"),
    createBasicTexture("tilewall1", "tilewall1.png"),
    createBasicTexture("center", "center.png"),
    createBasicTexture("bot_splash1", "bot2.png"),
    createBasicTexture("bot_splash2", "bot3.png"),
    createBasicTexture("red_carpet", "carpet_1.png"),
    createBasicTexture("tree1", "tree1.png"),
    // createBasicTexture("tree2", "tree2.png"),
    // createBasicTexture("tree3", "tree3.png"),
    // createBasicTexture("tree4", "tree4.png"),
    // createBasicTexture("tree5", "tree5.png"),
    // createBasicTexture("scr1", "scr1.png"),
];

CONFIG.textures = CONFIG.textures.concat(playerTextures, textures);

// Add texture groups
const tilesetGroups = [
    ['terrain', 'terrain_s.png', 128, 64, 32, 32],
].map(([name, path, width, height, tileWidth, tileHeight]) => 
    createTilesetGroup(name, path, width, height, tileWidth, tileHeight)
);

CONFIG.textureGroups.push(...tilesetGroups);