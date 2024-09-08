
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  c_kv_elements: {},
  canvas: document.getElementById('canvas'),

  start() {
    console.log("Starting...");
    this._isready();
  },

  onRuntimeInitialized() {
    console.log("Runtime initialized");

    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;

    update_worldsize(window.innerWidth, window.innerHeight);
  },

  ready() {
    console.log("Ready");
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
      setTimeout(() => {
        this.js_to_c(
          {"Entities": [  {
            "Player": true,
            "Position": {
                "x": 1,
                "y": 1,
                "z": 0
            }
          }]});
      }, 10);
    });

    // Object.keys(json).forEach(key => {
    //   if (key !== 'textures' && key !== 'shaders') {
    //     this.js_to_c({ [key]: json[key] });
    //   }
    // });
  }
}
