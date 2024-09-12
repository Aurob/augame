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
  Module.js_to_c(
    {"world": {
      "width": width,
      "height": height
    }});
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

function syllabify(word) {
  // Helper function to check if a character is a vowel
  function isVowel(char) {
      return 'aeiouy'.includes(char.toLowerCase());
  }

  // Helper function to find the syllable break point
  function findBreakPoint(word, start) {
      for (let i = start; i < word.length - 1; i++) {
          if (isVowel(word[i]) && !isVowel(word[i + 1])) {
              return i + 1;
          }
      }
      return word.length; // If no break point found, return the end of the word
  }

  let result = [];
  let start = 0;

  while (start < word.length) {
      let breakPoint = findBreakPoint(word, start);
      result.push(word.slice(start, breakPoint));
      start = breakPoint;
  }

  return result.join("-");
}
