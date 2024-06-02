function teleport(x, y) {
  Module.js_to_c(
    {"Entities": [  {
      "Player": true,
      "Position": {
          "x": x,
          "y": y
      }
    }]});
}

function setEvents() {
  // duplicate the canvas
  const duplicateCanvas = self.canvas.cloneNode();

  // on anywhere click run Module.start();
  function startModuleOnce(event) {
    Module.start();
    document.removeEventListener('click', startModuleOnce);
    document.removeEventListener('touchstart', startModuleOnce);

    // remove the temp canvas
    duplicateCanvas.remove();

    // hide cursor on main canvas
    self.canvas.style.cursor = 'none';
  }


  duplicateCanvas.id = 'duplicateCanvas';
  // copy all the styles
  duplicateCanvas.style = self.canvas.style;
  // position absolute, z-index 1, pointer-events none
  duplicateCanvas.style.position = 'absolute';
  duplicateCanvas.style.zIndex = 1;
  duplicateCanvas.style.pointerEvents = 'none';
  document.getElementById('main').prepend(duplicateCanvas);

  const ctx = duplicateCanvas.getContext('2d');

  // Load World config
  let fetch_path = "demo-a";


  fetch(`web/${fetch_path}.json?${Math.random()}`)
    .then(res => res.json())
    .then(json => {

      // json['shaders']['fragment'] = 
      if (json['shaders'] && Array.isArray(json['shaders'])) {
        json['shaders'].forEach(shader => {
          ['vertex', 'fragment'].forEach(type => {
            if (shader[type] && shader[type].includes('.glsl')) {
              fetch(`web/${shader[type]}?${Math.random()}`)
                .then(res => res.text())
                .then(text => {
                  shader[type] = text;
                  Module.js_to_c(JSON.stringify(json));

                  // on anywhere click run Module.start();                 
                  document.addEventListener('click', startModuleOnce);
                  document.addEventListener('touchstart', startModuleOnce);

                  // update temp canvas text
                  ctx.clearRect(0, 0, duplicateCanvas.width, duplicateCanvas.height);
                  ctx.font = '30px Arial';
                  ctx.fillStyle = 'white';
                  ctx.textAlign = 'center';
                  ctx.fillText('Click to start', duplicateCanvas.width / 2, duplicateCanvas.height / 2);
                });
            }
          });
        });
      }

      // Check for url params for x, y, and scale using URLSearchParams
      const params = new URLSearchParams(window.location.search);
      let x = params.get('x');
      let y = params.get('y');
      let scale = params.get('scale');

      // Render temp canvas that says "Loading..."

      ctx.font = '30px Arial';
      ctx.fillStyle = 'white';
      ctx.textAlign = 'center';
      ctx.fillText('Loading...', duplicateCanvas.width / 2, duplicateCanvas.height / 2);

    });
}

function loadInputs() {
  const parameters = [
    { 'name': 'waterMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'sandMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'dirtMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'grassMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'stoneMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'snowMax', 'min': -1, 'max': 1, 'step': 0.0001 },
    { 'name': 'frequency', 'min': -16, 'max': 16, 'step': 0.000000001 },
    { 'name': 'amplitude', 'min': -1, 'max': 1, 'step': 0.00001 },
    { 'name': 'persistence', 'min': -1, 'max': 1, 'step': 0.00001 },
    { 'name': 'lacunarity', 'min': -16, 'max': 16, 'step': 1 },
    { 'name': 'octaves', 'min': -16, 'max': 16, 'step': 0.1 },
    { 'name': 'scale', 'min': -100, 'max': 100, 'step': 0.001 }
  ];

  const container = document.createElement('div');
  parameters.forEach(param => {
    const div = document.createElement('div');
    const label = document.createElement('div');
    const title = document.createElement('div');
    const labelVal = document.createElement('div');

    title.textContent = `${param.name}: `;
    label.style.display = 'flex';
    label.appendChild(title);
    label.appendChild(labelVal);
    div.appendChild(label);

    const rangeInput = document.createElement('input');
    rangeInput.className = 'param';
    rangeInput.type = 'range';
    rangeInput.id = param.name;
    rangeInput.name = param.name;
    rangeInput.min = param.min;
    rangeInput.max = param.max;
    rangeInput.step = param.step;
    div.appendChild(rangeInput);

    labelVal.textContent = rangeInput.value;

    container.appendChild(div);


    rangeInput.addEventListener('input', function () {
      labelVal.textContent = rangeInput.value;
    });

  });

  document.getElementById('control-instructions').appendChild(container);


  const createButton = document.createElement('button');
  createButton.textContent = 'Create Link';
  createButton.addEventListener('click', function () {
    console.log('Create link clicked');
    let x = Module.c_kv_data.x;
    let y = Module.c_kv_data.y;
    let scale = Module.c_kv_data.scale;
    let url = `?x=${x}&y=${y}&scale=${scale}`;
    window.history.pushState({}, '', url);
  });
  document.getElementById('control-instructions').appendChild(createButton);
}