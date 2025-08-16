function teleport(x, y, z) {
  Module.js_to_c(
    {"Entities": [  {
      "Player": true,
      "Position": {
          "x": x,
          "y": y,
          "z": z
      }
    }]});
}

function move(w=0, a=0, s=0, d=0) {
  Module.js_to_c(
    {"Entities": [  {
      "Player": true,
      "MobileMovement": {
          "w": w,
          "a": a,
          "s": s,
          "d": d
      }
    }]});
}

function interact() {
  Module.js_to_c(
    {"Entities": [  {
      "Player": true,
      "Action": {
        "player": [{"interact": true}]
      }
    }]});
}


function update_worldsize(width, height) {
  if (width == null && height == null) {
    width = window.innerWidth;
    height = window.innerHeight;
  }
  Module.js_to_c({
    "world": {
      "width": width,
      "height": height
    }
  });
}

function zoom(zoom) {
  Module.js_to_c(
    {"world": {
      "zoom": zoom
    }});  
}

function test() {
  Module.js_to_c(
    {"Entities": [  {
      "New": true,
      "Position": {
          "x": 30,
          "y": 30
      },
      "Shape": {
          "size": [1, 1, 1]
      },
      "Color": {
          "r": 255,
          "g": 0,
          "b": 0,
          "a": 255
      }
    }]});
}

function generateBookshelfConfig(rowWidth, numberOfRows) {
  const configs = [];
  let idCounter = 5;
  let positionY = 5;

  for (let i = 0; i < numberOfRows; i++) {

      let shape = rowWidth;
      configs.push(`id ${idCounter} bs1 position 5 ${positionY} 0 shape ${shape} 3.3 0 renderPriority 1 inside 1 cshader bookshelf`);
      idCounter++;
      positionY += 6.6;

      configs.push(`id ${idCounter} bs1_blocker position 5 ${positionY+3.3} 0 shape ${shape} 0.1 0 color 0 0 0 0 renderPriority 1 inside 1 collidable`);
      idCounter++;
      positionY += 0.1;

  }

  return configs.join('\n');
}
