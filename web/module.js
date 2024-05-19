
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  canvas: (function () {
    const canvas = document.getElementById('canvas');
    return canvas;
  })(),

  start: function () {
    console.log("Starting...");

    setTimeout(() => {
      // duplicate the canvas
      // const duplicateCanvas = self.canvas.cloneNode();
      // duplicateCanvas.id = 'duplicateCanvas';
      // // copy all the styles
      // duplicateCanvas.style = self.canvas.style;
      // // position absolute, z-index 1, pointer-events none
      // duplicateCanvas.style.position = 'absolute';
      // duplicateCanvas.style.zIndex = 1;
      // duplicateCanvas.style.pointerEvents = 'none';
      // document.getElementById('main').prepend(duplicateCanvas);

      // // render small red circel in center
      // const ctx = duplicateCanvas.getContext('2d');
      // ctx.beginPath();
      // ctx.arc(duplicateCanvas.width / 2, duplicateCanvas.height / 2, 10, 0, 2 * Math.PI, false);
      // ctx.fillStyle = 'red';
      // ctx.fill();
      // ctx.lineWidth = 5;
      // ctx.strokeStyle = '#003300';
      // ctx.stroke();
      // ctx.closePath();

    }, 1);

    Module._isready();
  },

  onRuntimeInitialized: function () {
    console.log("Runtime initialized");

    // canvas.width = window.innerWidth;
    // canvas.height = window.innerHeight;
    canvas.width = 800;
    canvas.height = 600;
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
        document.getElementById('position').innerHTML = `x: ${parseInt(Module.c_kv_data.x)}, y: ${parseInt(Module.c_kv_data.y)}`;
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
}