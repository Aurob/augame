
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  c_kv_elements: {},
  canvas: document.getElementById('canvas'),
  show_alert(message) {
    // Create a toast container if it doesn't exist
    let toastContainer = document.getElementById('toast-container');
    if (!toastContainer) {
      toastContainer = document.createElement('div');
      toastContainer.id = 'toast-container';
      toastContainer.style.cssText = 'position: fixed; bottom: 20px; right: 20px; z-index: 9999; max-width: 80%;';
      document.body.appendChild(toastContainer);
    }
    
    // Create the toast element
    const toast = document.createElement('div');
    toast.style.cssText = 'background-color: rgba(0, 0, 0, 0.8); color: white; padding: 12px 20px; border-radius: 4px; margin-top: 10px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3); opacity: 0; transition: opacity 0.3s, transform 0.3s; transform: translateY(20px); max-width: 100%; word-wrap: break-word;';
    toast.textContent = message;
    
    // Add to container
    toastContainer.appendChild(toast);
    
    // Trigger animation
    setTimeout(() => {
      toast.style.opacity = '1';
      toast.style.transform = 'translateY(0)';
    }, 10);
    
    // Remove after 5 seconds
    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(20px)';
      setTimeout(() => {
        if (toast.parentNode) {
          toast.parentNode.removeChild(toast);
        }
      }, 300);
    }, 5000);
  },
  play_tone(note, duration, volume, type) {
    if (type === "sine") {
      const synth = new Tone.Synth({
        oscillator: {
          type: "sine"
        },
        envelope: {
          attack: 0.01,
          decay: 0.5,
          sustain: 0.3,
          release: 1.5
        }
      }).toDestination();
      
      synth.volume.value = volume;
      synth.triggerAttackRelease(note, duration);
    } else {
      const player = new Tone.Player(`resources/audio/${type}`, () => {
        if (note !== 'R') {
          player.playbackRate = Tone.Frequency(note).toFrequency() / 440; // Assuming A4 = 440Hz
        } else {
          const randomNote = Tone.Frequency(Math.random() * 400 + 600).toNote(); // Keep it within a higher octave, slightly under and over
          player.playbackRate = Tone.Frequency(randomNote).toFrequency() / 440;
        }
        player.volume.value = volume;
        player.start();
      }).toDestination();
    }
  },
  fetch_configs() {
    const json = CONFIG;
    if (Array.isArray(json.textures)) {
      this.processTextures(json.textures);
    }
    if (Array.isArray(json.textureGroups)) {
      this.js_to_c({ textureGroups: json.textureGroups });
    }
    const shadersPromise = Array.isArray(json.shaders) ? this.processShaders(json.shaders) : Promise.resolve();
    shadersPromise.then(() => {
      
      // Function to process config text
      this.processConfigText = (configText) => {
        // Store the config text for future use
        this.configText = configText;
        
        // Reset current comment block
        this.currentCommentBlock = null;
        
      // / Create a floating control panel window
      var configTextarea = '';
      const headerDiv = document.getElementById('header');
      const toggleButton = document.createElement('button');
      toggleButton.textContent = 'Open Control Panel';
      toggleButton.className = 'toggle-btn';
      
      // Create the popup window when the button is clicked
      toggleButton.onclick = () => {
        const controlPanel = window.open('', 'ControlPanel', 'width=400,height=300,resizable=yes');
        
        // Style the control panel window
        controlPanel.document.write(`
          <html>
          <head>
            <title>Game Control Panel</title>
            <style>
              body {
                font-family: Arial, sans-serif;
                background-color: #2a2a2a;
                color: #e0e0e0;
                margin: 0;
                padding: 15px;
              }
              .panel-title {
                margin-top: 0;
                margin-bottom: 15px;
                font-size: 18px;
              }
              #configEditor {
                width: 100%;
                height: 150px;
                font-family: monospace;
                background-color: #333;
                color: #e0e0e0;
                border: 1px solid #555;
                padding: 8px;
                margin-bottom: 10px;
                box-sizing: border-box;
              }
              .btn {
                background-color: #4a4a4a;
                border: none;
                color: #e0e0e0;
                padding: 8px 12px;
                cursor: pointer;
                border-radius: 4px;
                margin-right: 8px;
              }
              .btn:hover {
                background-color: #5a5a5a;
              }
            </style>
          </head>
          <body>
            <h3 class="panel-title">Game Control Panel</h3>
            <pre contenteditable id="configEditor">${configText}</pre>
            <button id="applyBtn" class="btn">Apply Config</button>
          </body>
          </html>
        `);
        
        // Add event listener to the apply button
        controlPanel.document.getElementById('applyBtn').addEventListener('click', () => {
          const configText = controlPanel.document.getElementById('configEditor').innerText;
          this._reload();
          this.processConfigText(configText);
        });
        
        // Prevent the window from being closed when the main window refreshes
        controlPanel.onbeforeunload = () => {
          controlPanel.opener = null;
        };
      };
      
      headerDiv.appendChild(toggleButton);
    
        // Helper method to send the accumulated comment block to C++
        const flushCommentBlock = () => {
          if (this.currentCommentBlock && this.currentCommentBlock.content.length > 0) {
            const logObj = { log: { level: this.currentCommentBlock.level } };
            logObj.log[""] = this.currentCommentBlock.content.join("\n");
            this.js_to_c(logObj);
            this.currentCommentBlock = null;
          }
        };
        
        const configstr = configText.split("\n").filter(line => {
          if (line.length === 0)  {
            flushCommentBlock();
            return false;
          }

          if (line.startsWith("--")) {
            // Check if this is the start of a new comment block or continuation
            if (!this.currentCommentBlock) {
              this.currentCommentBlock = {
                level: "CONSOLE", // Default level
                messageType: "",
                content: []
              };

              // Determine the comment type from the first line
              const symbol = line.substring(3, 4);
              if (symbol === "@") {
                this.currentCommentBlock.level = "INFO";
                this.currentCommentBlock.messageType = "info";
              }
              else if (symbol === "#") {
                this.currentCommentBlock.level = "DEBUG";
                this.currentCommentBlock.messageType = "debug";
              }
              else if (symbol === "*") {
                this.currentCommentBlock.level = "WARN";
                this.currentCommentBlock.messageType = "warning";
              }
              else if (symbol === "!") {
                this.currentCommentBlock.level = "ERROR";
                this.currentCommentBlock.messageType = "error";
              }
            }

            // Add the content to the current block
            const content = line.substring(line.indexOf("--") + 2).trim();
            if (content) {
              this.currentCommentBlock.content.push(content);
            } else if (this.currentCommentBlock.content.length > 0) {
              // Empty comment line after content means end of block
              flushCommentBlock();
            }

            return false;
          } else if (this.currentCommentBlock) {
            // Non-comment line after a comment block, flush the block
            flushCommentBlock();
          }

          return true;
        });
        
        ECONFIG = {
          "Entities": configstr.map(input => {
            const builder = new EntityBuilder().parseInput(input);
            const entity = builder.build();
            return entity;
          })
        };
        
        this.js_to_c(ECONFIG);
        this.start();
      };

      fetch('web/econfigs/demo.txt?'+Math.random())
        .then(res => res.text())
        .then(data => {
          // // Store the config text in the textarea
          // Process the config text

          // data += this.addRoom(1, 'test', 0, 0, 10, 10);
          // data += this.addRoom(2, 'test', 10, 0, 10, 10);
          // console.log(data)
          this.processConfigText(data);
        });
      })
  },

  // Add a room to the config
  addRoom(id, name, x, y, width, height) {
      // Create room template based on the format in demo.txt
      const roomTemplate = `
id ${id} ${name} position ${x} ${y} 0 shape ${width} ${height} 0 color 0.5 0.5 0.5 1.0 renderPriority -1 interior 
id ${id+1} ${name}wall_top position ${x} ${y-1} 0 shape ${width} 2 1 color 0.1 0.2 0.3 1.0 renderPriority 2 inside ${id} textureGroupPart room1 s227 ${width} 2
id ${id+2} ${name}wall_top_col position ${x} ${y+0.5} 1 shape ${width} .1 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside ${id} collidable
id ${id+3} ${name}wall_bottom position ${x} ${y+height-1} 1 shape ${width} 2 1 color 0.1 0.2 0.3 1.0 renderPriority 1 inside ${id} textureGroupPart room1 s227 ${width} 2
id ${id+4} ${name}wall_bottom_col position ${x} ${y+height+0.5} 1 shape ${width} .1 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside ${id} collidable
id ${id+5} ${name}wall_left position ${x-0.1} ${y+.5} 1 shape .1 ${height} 1 color 0.1 0.2 0.3 1.0 inside ${id} collidable
id ${id+6} ${name}wall_right position ${x+width} ${y+.5} 1 shape .1 ${height} 1 color 0.1 0.2 0.3 1.0 inside ${id} collidable
`;
              
    //Append the room template to the existing config data
    return roomTemplate;
  },
  start() {
    console.log("Starting...");
    this._isready();
  },

  onRuntimeInitialized() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;

    update_worldsize(window.innerWidth, window.innerHeight);
  },

  ready() {
  },

  setkv(key, value) {
    const parsedValue = parseFloat(value.toFixed(2));
    this.c_kv_data[key] = parsedValue;

    if (key in this.c_kv_elements) {
      if (typeof value === 'number' && !isNaN(value)) {
        document.getElementById(this.c_kv_elements[key]).innerText = `${key}: ${parsedValue}`;
      } else {
        console.error(`Invalid value for key ${key}`);
      }
    }
  },
  js_to_c(str) {
    if (typeof str === 'object') {
      str = JSON.stringify(str);
    }
    const strPtr = this._malloc(str.length + 1);
    this.stringToUTF8(str, strPtr, str.length + 1);
    this._load_json(strPtr);
    this._free(strPtr);
  },

  processTextures(textures) {
    textures.forEach(texture => {
      if (texture.path) {
        this.js_to_c({ texture: { name: texture.name, path: texture.path } });
      }
    });
  },

  processShaders(shaders) {
    const fetchPromises = shaders.map(shader => {
      const vertexPromise = shader.vertex && shader.vertex.includes('.glsl')
        ? fetch(`resources/shaders/${shader.vertex}?${Math.random()}`).then(res => res.text())
        : Promise.resolve(shader.vertex);

      const fragmentPromise = shader.fragment && shader.fragment.includes('.glsl')
        ? fetch(`resources/shaders/${shader.fragment}?${Math.random()}`).then(res => res.text())
        : Promise.resolve(shader.fragment);

      return Promise.all([vertexPromise, fragmentPromise]).then(([vertex, fragment]) => {
        shader.vertex = vertex;
        shader.fragment = fragment;
        this.js_to_c({ shader: { name: shader.name, vertex, fragment, texture: shader.texture || null } });
      });
    });

    return Promise.all(fetchPromises);
  },

}
