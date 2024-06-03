
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  shaders: [],
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
    setEvents();
    loadInputs();

    // Set input values in c_kv_data
    const inputs = document.querySelectorAll('input');
    // console.log(inputs);
    inputs.forEach(input => {
      let name = input.getAttribute('name');
      // console.log(name);
      if (name in Module.c_kv_data) {
        input.value = Module.c_kv_data[name];
      }
    });

    // on slider change, normalize all values to fit a total of 1
    // then send the values to the webassembly module
    document.querySelectorAll('.param').forEach(function (el) {
      el.addEventListener('input', function (e) {
        // console.log(e.target.id, e.target.value);
        Module.js_to_c({ option: e.target.id, value: parseFloat(e.target.value) });
      });
    });
  },

  setkv: function (key, value) {
    Module.c_kv_data[key] = value;
    if (!Object.keys(Module.c_kv_data).includes(key)) {
      Module.c_kv_data[key] = value;
    }

    switch (key) {
      case 'x':
      case 'y':
        document.getElementById('position').innerHTML = `x: ${Module.c_kv_data.x}, y: ${Module.c_kv_data.y}`;
        break;
      case 'tilex':
      case 'tiley':
        document.getElementById('tile').innerHTML = `tilex: ${Module.c_kv_data.tilex}, tiley: ${Module.c_kv_data.tiley}`;
        break;
      case 'scale':
        document.getElementById('scale').innerHTML = `scale: ${Module.c_kv_data.scale}`;
        break;
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
    let fetch_path = "demo-a";
    fetch(`web/${fetch_path}.json?${Math.random()}`)
      .then(res => res.json())
      .then(json => {

        if (json['shaders'] && Array.isArray(json['shaders'])) {
          let totalShaders = json['shaders'].length;
          let loadedShaders = 0;
          json['shaders'].forEach(shader => {
            ['vertex', 'fragment'].forEach(type => {
              if (shader[type] && shader[type].includes('.glsl')) {
                fetch(`web/${shader[type]}?${Math.random()}`)
                  .then(res => res.text())
                  .then(text => {
                    shader[type] = text;
                    if (type == 'fragment') {
                      Module.js_to_c(JSON.stringify({
                        shader: {
                          name: shader['name'],
                          vertex: shader['vertex'],
                          fragment: shader['fragment']
                        }
                      }));
                      loadedShaders++;
                      if (loadedShaders === totalShaders) {
                        Module.start();
                      }
                    }
                  });
              }
            });
          });
        }
      });
  }
}