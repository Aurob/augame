

function setEvents() {
}

function loadInputs() {
    const parameters = [
        {'name': 'waterMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'sandMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'dirtMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'grassMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'stoneMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'snowMax', 'min': -1, 'max': 1, 'step': 0.000001},
        {'name': 'frequency', 'min': -16, 'max': 16, 'step': 0.000000000001},
        {'name': 'amplitude', 'min': -1, 'max': 1, 'step': 0.000000001},
        {'name': 'persistence', 'min': -1, 'max': 1, 'step': 0.000000001},
        {'name': 'lacunarity', 'min': -16, 'max': 16, 'step': 1},
        {'name': 'octaves', 'min': -16, 'max': 16, 'step': 0.1},
        {'name': 'scale', 'min': -100, 'max': 100, 'step': 0.000001}
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


        rangeInput.addEventListener('input', function() {
            labelVal.textContent = rangeInput.value;
        });


    });

    document.getElementById('control-instructions').appendChild(container);
}