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
];

CONFIG.textures = CONFIG.textures.concat(playerTextures, textures);
