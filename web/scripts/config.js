var CONFIG = {
    "shaders": [
        {
            "name": "terrain",
            "vertex": "attribute vec4 position; void main() { gl_Position = vec4(position.xyz, 1.0); }",
            "fragment": "terrain_simple.glsl"
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
        },
        {
            "name": "font",
            "vertex": "font_v.glsl",
            "fragment": "font_f.glsl"
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
    createBasicTexture("key", "key.png")
];

CONFIG.textures = CONFIG.textures.concat(playerTextures, textures);
