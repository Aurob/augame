
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  c_kv_elements: {},
  canvas: (function () {
    const canvas = document.getElementById('canvas');
    return canvas;
  })(),

  start: function () {
    console.log("Starting...");
    Module._isready();
  },

  onRuntimeInitialized: function () {
    console.log("Runtime initialized");

    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    
    update_worldsize(window.innerWidth, window.innerHeight);
  },

  ready() {
    console.log("Ready");
    loadInputs();
  },

  setkv: function (key, value) {
    Module.c_kv_data[key] = parseFloat(value.toFixed(2));
    if (!Object.keys(Module.c_kv_data).includes(key)) {
      Module.c_kv_data[key] = parseFloat(value.toFixed(2));
    }

    if (key in Module.c_kv_elements) {
      if (typeof value === 'number' && !isNaN(value)) {
        document.getElementById(Module.c_kv_elements[key]).innerText = `${key}: ${parseFloat(value.toFixed(2))}`;
      } else {
        console.error(`Invalid value for key ${key}`);
      }
    }
  },

  js_to_c: function (str) {
    if (typeof str === 'object') {
      str = JSON.stringify(str);
    }
    var strPtr = Module._malloc(str.length + 1);
    Module.stringToUTF8(str, strPtr, str.length + 1);
    Module._load_json(strPtr);
    Module._free(strPtr);
  },
  
  fetch_configs: function () {
    let json = CONFIG;

      if (json['textures'] && Array.isArray(json['textures'])) {
        json['textures'].forEach(texture => {
          if (texture['path']) {
            Module.js_to_c(JSON.stringify({
              texture: {
                name: texture['name'],
                path: texture['path'],
              }
            }));
          }
        });
      }

      if (json['shaders'] && Array.isArray(json['shaders'])) {
        let fetchPromises = json['shaders'].map(shader => {
          let vertexPromise = shader['vertex'] && shader['vertex'].includes('.glsl') 
            ? fetch(`web/${shader['vertex']}?${Math.random()}`).then(res => res.text())
            : Promise.resolve(shader['vertex']);
          
          let fragmentPromise = shader['fragment'] && shader['fragment'].includes('.glsl') 
            ? fetch(`web/${shader['fragment']}?${Math.random()}`).then(res => res.text())
            : Promise.resolve(shader['fragment']);
          
          return Promise.all([vertexPromise, fragmentPromise]).then(([vertex, fragment]) => {
            shader['vertex'] = vertex;
            shader['fragment'] = fragment;
            Module.js_to_c(JSON.stringify({
              shader: {
                name: shader['name'],
                vertex: shader['vertex'],
                fragment: shader['fragment'],
                texture: shader['texture'] || null,
              }
            }));
          });
        });

        Promise.all(fetchPromises).then(() => {
          Module.start();
        });
      }

      // Send all other keys other than textures and shaders
      let keys = Object.keys(json);
      keys.forEach(key => {
        if (key !== 'textures' && key !== 'shaders') {
          Module.js_to_c(JSON.stringify({ [key]: json[key] }));
        }
      });
    }
  }
