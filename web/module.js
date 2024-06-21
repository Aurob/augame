
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
  },

  ready() {
    console.log("Ready");

    // add custom c-to-js output elements
    const elements = {
      x: { id: 'xpos', hasInput: true },
      y: { id: 'ypos', hasInput: true },
      gridSpacingValue: { id: 'gridSpacingValue', hasInput: false }
    };

    for (const [key, { id, hasInput }] of Object.entries(elements)) {
      let element = document.createElement('div');
      element.id = id;
      element.innerText = `${key}: 0`;
      document.body.prepend(element);
      Module.c_kv_elements[key] = id;

      // Create input elements for keys that require them
      if (hasInput) {
        let inputElement = document.createElement('input');
        inputElement.type = 'number';
        inputElement.id = `${id}_input`;
        inputElement.value = 0;
        inputElement.addEventListener('input', (event) => {
          const value = parseFloat(event.target.value);
          Module.setkv(key, value);
          if (key === 'x' || key === 'y') {
            console.log("Teleporting...");
            Module.js_to_c({
              "Entities": [{
                "Player": true,
                "Position": {
                  x: key === 'x' ? value : Module.c_kv_data.x,
                  y: key === 'y' ? value : Module.c_kv_data.y
                }
              }]
            });
          }
        });
        document.body.prepend(inputElement);
      }
    }
  },

  setkv: function (key, value) {
    Module.c_kv_data[key] = value;
    if (!Object.keys(Module.c_kv_data).includes(key)) {
      Module.c_kv_data[key] = value;
    }

    if(key in Module.c_kv_elements) {
      document.getElementById(Module.c_kv_elements[key]).innerText = `${key}: ${value}`;
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
