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