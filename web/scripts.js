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
  Module.js_to_c(
    {"world": {
      "width": width,
      "height": height
    }});
}