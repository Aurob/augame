
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  c_kv_elements: {},
  canvas: document.getElementById('canvas'),

  fetch_configs() {
    const json = CONFIG;
    if (Array.isArray(json.textures)) {
      this.processTextures(json.textures);
    }
    if (Array.isArray(json.textureGroups)) {
      this.js_to_c({ textureGroups: json.textureGroups });
    }
    const shadersPromise = Array.isArray(json.shaders) ? this.processShaders(json.shaders) : Promise.resolve();
    shadersPromise.then(() => {
      this.js_to_c(ECONFIG);
      this.start();
    });
    
  },
  
  start() {
    console.log("Starting...");
    this._isready();
  },

  onRuntimeInitialized() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;

    update_worldsize(window.innerWidth, window.innerHeight);
  },

  ready() {
    loadInputs();
  },

  setkv(key, value) {
    const parsedValue = parseFloat(value.toFixed(2));
    this.c_kv_data[key] = parsedValue;

    if (key in this.c_kv_elements) {
      if (typeof value === 'number' && !isNaN(value)) {
        document.getElementById(this.c_kv_elements[key]).innerText = `${key}: ${parsedValue}`;
      } else {
        console.error(`Invalid value for key ${key}`);
      }
    }
  },
  play_tone(note, duration, volume, type) {
    if (type === "sine") {
      const synth = new Tone.Synth({
        oscillator: {
          type: "sine"
        },
        envelope: {
          attack: 0.01,
          decay: 0.5,
          sustain: 0.3,
          release: 1.5
        }
      }).toDestination();
      
      synth.volume.value = volume;
      synth.triggerAttackRelease(note, duration);
    } else {
      const player = new Tone.Player(`web/audio/${type}`, () => {
        if (note !== 'R') {
          player.playbackRate = Tone.Frequency(note).toFrequency() / 440; // Assuming A4 = 440Hz
        } else {
          const randomNote = Tone.Frequency(Math.random() * 400 + 600).toNote(); // Keep it within a higher octave, slightly under and over
          player.playbackRate = Tone.Frequency(randomNote).toFrequency() / 440;
        }
        player.volume.value = volume;
        player.start();
      }).toDestination();
    }
  },

  play_string_tones(inputString) {
    const letterToNote = {
      a: "C4", b: "D4", c: "E4", d: "F4", e: "G4", f: "A4", g: "B4",
      h: "C5", i: "D5", j: "E5", k: "F5", l: "G5", m: "A5", n: "B5",
      o: "C6", p: "D6", q: "E6", r: "F6", s: "G6", t: "A6", u: "B6",
      v: "C7", w: "D7", x: "E7", y: "F7", z: "G7"
    };

    const playNote = (note) => {
      return new Promise(resolve => {
        this.play_tone(note, "16n", -12, "sine");
        setTimeout(resolve, 200); // Adjust the delay as needed
      });
    };

    const playSyllable = async (syllable) => {
      const note = letterToNote[syllable[0].toLowerCase()] || "C4";
      await playNote(note);
    };

    const playWord = async (word) => {
      const syllables = syllabify(word).split('-');
      for (const syllable of syllables) {
        await playSyllable(syllable);
      }
    };

    const playSentence = async (sentence) => {
      const words = sentence.split(' ');
      for (const word of words) {
        if (word.trim() !== "") {
          await playWord(word);
        }
      }
    };

    playSentence(inputString);
  },

  js_to_c(str) {
    if (typeof str === 'object') {
      str = JSON.stringify(str);
    }
    const strPtr = this._malloc(str.length + 1);
    this.stringToUTF8(str, strPtr, str.length + 1);
    this._load_json(strPtr);
    this._free(strPtr);
  },

  processTextures(textures) {
    textures.forEach(texture => {
      if (texture.path) {
        this.js_to_c({ texture: { name: texture.name, path: texture.path } });
      }
    });
  },

  processShaders(shaders) {
    const fetchPromises = shaders.map(shader => {
      const vertexPromise = shader.vertex && shader.vertex.includes('.glsl')
        ? fetch(`web/${shader.vertex}?${Math.random()}`).then(res => res.text())
        : Promise.resolve(shader.vertex);

      const fragmentPromise = shader.fragment && shader.fragment.includes('.glsl')
        ? fetch(`web/${shader.fragment}?${Math.random()}`).then(res => res.text())
        : Promise.resolve(shader.fragment);

      return Promise.all([vertexPromise, fragmentPromise]).then(([vertex, fragment]) => {
        shader.vertex = vertex;
        shader.fragment = fragment;
        this.js_to_c({ shader: { name: shader.name, vertex, fragment, texture: shader.texture || null } });
      });
    });

    return Promise.all(fetchPromises);
  },

}
