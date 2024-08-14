const CONFIG = {
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
        }
    ]
};