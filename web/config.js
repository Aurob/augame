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
            "name": "tileset1",
            "path": "resources/tileset1.png"
        }
    
    ],
    "textureGroups": []
};

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

CONFIG.textureGroups.push(tileset1);

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


let synth, bassSynth, loop;

function stopAudio() {
    if (synth) {
        synth.dispose();
        bassSynth.dispose();
        loop.stop();
        Tone.Transport.stop();
        synth = null;
        bassSynth = null;
        loop = null;
    }
    playButton.innerText = 'Play Music';
    playButton.removeEventListener('click', stopAudio);
    playButton.addEventListener('click', startAudio);
}

function startAudio() {
    Tone.start().then(() => {
        initializeAudio();
        playButton.innerText = 'Stop Music';
        playButton.removeEventListener('click', startAudio);
        playButton.addEventListener('click', stopAudio);
    });
}

function initializeAudio() {
    if (!synth) {
        synth = new Tone.PolySynth(Tone.Synth, {
            volume: -32,
            envelope: {
                attack: 0.05,
                decay: 0.1,
                sustain: 0.7,
                release: 1.5
            }
        }).toDestination();

        bassSynth = new Tone.Synth({
            oscillator: {
                type: "sine"
            },
            volume: -44,
            envelope: {
                attack: 0.05,
                decay: 0.1,
                sustain: 0.7,
                release: 1.5
            }
        }).toDestination();

        loop = new Tone.Loop(time => {
            const notes = generateHarmony();
            synth.triggerAttackRelease(notes.harmony, getRandomElement(["8n", "4n", "2n", "16n"]), time);
            bassSynth.triggerAttackRelease(notes.bass, getRandomElement(["1n", "2n", "4n", "16n"]), time);
        }, "4n");

        Tone.Transport.start();
        loop.start(0);
    }
}

function generateHarmony() {
    const notes = ["C", "D", "E", "F", "G", "A", "B"];
    const octaves = [1, 2, 3, 3.5];
    
    function generateRandomScale() {
        return Array.from({ length: 7 }, () => {
            const note = getRandomElement(notes);
            const octave = getRandomElement(octaves);
            return `${note}${octave}`;
        });
    }

    const scale = generateRandomScale();
    const bassScale = generateRandomScale();
    
    const noiseValue1 = noise.simplex2(Math.random(), Math.random());
    const noiseValue2 = noise.simplex3(Math.random(), Math.random(), Date.now() * 0.0001);
    const index1 = Math.floor((noiseValue1 + 1) / 2 * scale.length);
    const index2 = Math.floor((noiseValue2 + 1) / 2 * bassScale.length);
    
    const harmony = [
        scale[index1],
        scale[(index1 + Math.floor(noise.simplex2(Math.random(), Math.random()) * 5)) % scale.length],
        scale[(index1 + Math.floor(noise.simplex2(Math.random(), Math.random()) * 7)) % scale.length]
    ];
    const bass = bassScale[index2];
    
    return { harmony, bass };
}

function getRandomElement(arr) {
    return arr[Math.floor(Math.random() * arr.length)];
}

// Add buttons to the header
const header = document.getElementById('header');

const playButton = document.createElement('button');
playButton.innerText = 'Play Music';
playButton.addEventListener('click', () => {
    const script = document.createElement('script');
    script.src = 'web/tone.js';
    script.onload = startAudio;
    document.body.prepend(script);
});
header.appendChild(playButton);
