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
        }
    ]
};

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



