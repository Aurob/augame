
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
  speak(text, options = {}) {
    const synth = window.speechSynthesis;
    if (!synth) return console.error('Speech synthesis not supported');

    // Cancel any ongoing speech
    synth.cancel();

    const utterance = new SpeechSynthesisUtterance(text);

    // Apply options
    Object.assign(utterance, {
      rate: options.rate ?? 1,
      pitch: options.pitch ?? 1,
      volume: options.volume ?? 1,
      voice: options.voice ?? null,
      lang: options.lang ?? 'en-US',
    });

    if (options.onEnd) utterance.onend = options.onEnd;
    if (options.onError) utterance.onerror = options.onError;

    // Add event listener for page unload/refresh to stop speech
    const cancelSpeechOnUnload = () => synth.cancel();
    window.addEventListener('beforeunload', cancelSpeechOnUnload);

    // Clean up event listener when speech ends
    utterance.onend = (event) => {
      window.removeEventListener('beforeunload', cancelSpeechOnUnload);
      if (options.onEnd) options.onEnd(event);
    };

    // Also clean up on error
    utterance.onerror = (event) => {
      window.removeEventListener('beforeunload', cancelSpeechOnUnload);
      if (options.onError) options.onError(event);
    };

    synth.speak(utterance);

    // Return a function that can be used to manually stop the speech
    return {
      stop: () => {
        synth.cancel();
        window.removeEventListener('beforeunload', cancelSpeechOnUnload);
      }
    };
  },
  fetch_configs() {
    const json = CONFIG;
    if (Array.isArray(json.textures)) {
      this.processTextures(json.textures);
    }
    const shadersPromise = Array.isArray(json.shaders) ? this.processShaders(json.shaders) : Promise.resolve();
    shadersPromise.then(() => {

      // Function to process config text
      this.processConfigText = (configText) => {
        // Store the config text for future use
        this.configText = configText;

        // Reset current comment block
        this.currentCommentBlock = null;
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
          if (line.length === 0) {
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
        // Create entities from config strings and collect meta data
        const entities = [];
        const metaData = {};
        this.metadata = [];

        configstr.forEach(input => {
          const builder = new EntityBuilder().parseInput(input);
          const entity = builder.build();
          
          if (entity.meta) {
            // Collect meta data
            Object.assign(metaData, entity.meta);
            this.metadata.push(entity.meta);
          } 
          else {
            // Regular entity
            entities.push(entity);
          }
        });
        
        // Process meta data to update HTML
        if (Object.keys(metaData).length > 0) {
          this.processMeta(metaData);
        }

        // Collect all fetch promises for text components and meta tags
        const fetchPromises = [];

        // Process meta tags for file references
        Object.keys(metaData).forEach(key => {
          const value = metaData[key];
          if (typeof value === 'string' && value.startsWith('@')) {
            const filePath = value.substring(1).trim();
            const fetchPromise = fetch(filePath+'?'+Math.random())
              .then(response => response.text())
              .then(fileContent => {
                // Update the meta data with the file content
                metaData[key] = fileContent;
              })
              .catch(error => {
                console.error(`Failed to fetch meta file: ${filePath}`, error);
              });

            fetchPromises.push(fetchPromise);
          }
        });

        // Process each entity for text components that need fetching
        entities.forEach(entity => {
          if (entity.Components && entity.Components.Text) {
            const text = entity.Components.Text.text;

            // If text starts with @, treat it as a file path and fetch it
            if (text.startsWith('@')) {
              const filePath = text.substring(1).trim();
              const fetchPromise = fetch(filePath+'?'+Math.random())
                .then(response => response.text())
                .then(fileContent => {
                  // Update the entity's Text component with the file content
                  entity.Components.Text.text = fileContent;
                })
                .catch(error => {
                  console.error(`Failed to fetch file: ${filePath}`, error);
                });

              fetchPromises.push(fetchPromise);
            }
          }
        });

        // Wait for all fetches to complete before sending to C++
        Promise.all(fetchPromises)
          .then(() => {
            // Send meta data to C++ after all file fetches complete
            if (Object.keys(metaData).length > 0) {
              this.js_to_c({ meta: metaData });
            }

            ECONFIG = {
              "Entities": entities
            };

            this.js_to_c(ECONFIG);
            this.start();
          })
          .catch(error => {
            console.error("Error processing entity text components:", error);
            // Still proceed with available data
            ECONFIG = {
              "Entities": entities
            };

            this.js_to_c(ECONFIG);
            this.start();
          });
      };

      // Check URL parameters for config file
      const urlParams = new URLSearchParams(window.location.search);
      const configParam = urlParams.get('world');
      const configFile = configParam ? `web/econfigs/${configParam}.txt` : 'web/econfigs/default.txt';

      fetch(`${configFile}?` + Math.random())
        .then(res => res.text())
        .then(data => {
          this.processConfigText(data);
        })
        .catch(error => {
          console.error(`Failed to load config file: ${configFile}`, error);
          // Fallback to demo.txt if specified config fails
          if (configParam) {
            fetch('web/econfigs/default.txt?' + Math.random())
              .then(res => res.text())
              .then(data => {
                this.processConfigText(data);
              });
          }
        });
    })
  },

  processMeta(metaData) {
    // Update document title
    if (metaData.title) {
      document.title = metaData.title;
    }
    
    // Update or create meta description tag
    if (metaData.description) {
      let metaDesc = document.querySelector('meta[name="description"]');
      if (!metaDesc) {
        metaDesc = document.createElement('meta');
        metaDesc.name = 'description';
        document.head.appendChild(metaDesc);
      }
      metaDesc.content = metaData.description;
    }
    
    // Update or create meta author tag  
    if (metaData.author) {
      let metaAuthor = document.querySelector('meta[name="author"]');
      if (!metaAuthor) {
        metaAuthor = document.createElement('meta');
        metaAuthor.name = 'author';
        document.head.appendChild(metaAuthor);
      }
      metaAuthor.content = metaData.author;
    }

    console.log('Meta data processed:', metaData);
  },
  
  start() {
    console.log("Starting...");
    this._isready();
    this.canvas.style.display = "block"
  },

  onRuntimeInitialized() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;

    update_worldsize(window.innerWidth, window.innerHeight);

    // Set up window resize event to update world size
    window.addEventListener('resize', () => {
      update_worldsize(window.innerWidth, window.innerHeight);
    });

  },

  ready() {
  },
  update_color(r, g, b) {
    document.querySelector('#tcolor').style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
  },
  update_user_position(x, y) {
    document.querySelector('#upos').innerText = `${Math.round(x)}, ${Math.round(y)}`;
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
        ? fetch(`${shader.vertex}?${Math.random()}`).then(res => res.text())
        : Promise.resolve(shader.vertex);

      const fragmentPromise = shader.fragment && shader.fragment.includes('.glsl')
        ? fetch(`${shader.fragment}?${Math.random()}`).then(res => res.text())
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
