
var Module = {
  initialized: false,
  c_kv_data: { x: 0, y: 0 },
  c_kv_elements: {},
  engine_json: [],
  canvas: document.getElementById('canvas'),
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
        
        // Parse multi-scene configs
        const sceneData = this.parseMultiSceneConfig(configText);
        console.log('Scene data parsed:', sceneData);
        
        // Process all scenes if multiple scenes exist
        if (sceneData.alt_scenes && sceneData.alt_scenes.length > 0) {
          // Collect all scenes, loading file references as needed
          const scenePromises = [];
          
          // Handle primary scene
          if (sceneData.primary_scene_is_file_ref) {
            console.log('Loading primary scene file reference:', sceneData.primary_scene_file_path);
            const primaryPromise = fetch(`${sceneData.primary_scene_file_path}?${Math.random()}`)
              .then(res => res.text())
              .then(fileContent => {
                console.log('Loaded primary scene file:', sceneData.primary_scene_file_path);
                return fileContent;
              })
              .catch(error => {
                console.error(`Failed to load primary scene file: ${sceneData.primary_scene_file_path}`, error);
                return sceneData.primary_scene; // Fallback to original content
              });
            scenePromises.push(primaryPromise);
          } else {
            scenePromises.push(Promise.resolve(sceneData.primary_scene));
          }
          
          // Handle alternative scenes (load file references)
          sceneData.alt_scenes.forEach((scene, index) => {
            if (scene.isFileReference) {
              console.log(`Loading alt scene ${index + 1} file reference:`, scene.filePath);
              const altPromise = fetch(`${scene.filePath}?${Math.random()}`)
                .then(res => res.text())
                .then(fileContent => {
                  console.log(`Loaded alt scene ${index + 1} file:`, scene.filePath);
                  return fileContent;
                })
                .catch(error => {
                  console.error(`Failed to load alt scene file: ${scene.filePath}`, error);
                  return scene.content; // Fallback to original content
                });
              scenePromises.push(altPromise);
            } else {
              scenePromises.push(Promise.resolve(scene.content));
            }
          });
          
          // Wait for all scenes to load, then process them
          Promise.all(scenePromises)
            .then(allSceneContents => {
              this.allScenes = allSceneContents;
              console.log('Parsed multi-scene config:', {
                total_scenes: this.allScenes.length,
                primary_scene_preview: this.allScenes[0].substring(0, 50) + '...',
                alt_scenes_count: sceneData.alt_scenes.length
              });
              
              // Process all scenes sequentially
              this.loadAllScenes();
            })
            .catch(error => {
              console.error('Error loading multi-scene config:', error);
            });
          return; // Exit early - loadAllScenes will handle the rest
        }
        
        // Single scene with file reference
        if (sceneData.primary_scene_is_file_ref) {
          console.log('Loading single scene file reference:', sceneData.primary_scene_file_path);
          // Load the referenced file content
          fetch(`${sceneData.primary_scene_file_path}?${Math.random()}`)
            .then(res => res.text())
            .then(fileContent => {
              console.log('Loaded file reference scene:', sceneData.primary_scene_file_path);
              console.log('File content preview:', fileContent.substring(0, 200));
              // Process the loaded file content directly (don't parse for multi-scenes again)
              this.processConfigTextDirect(fileContent);
            })
            .catch(error => {
              console.error(`Failed to load scene file: ${sceneData.primary_scene_file_path}`, error);
              // Fallback to processing as-is
              this.processConfigText(sceneData.primary_scene);
            });
          return; // Exit early, file loading will call processConfigText again
        }
        
        // Single scene - use primary scene content for processing
        configText = sceneData.primary_scene;

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
          } else if (key === 'slides' && typeof value === 'object') {
            // Handle slides object - check each slide for file references
            Object.keys(value).forEach(slideId => {
              const slideText = value[slideId];
              if (typeof slideText === 'string') {
                let filePath = null;
                if (slideText.startsWith('@')) {
                  filePath = slideText.substring(1).trim();
                }

                if (filePath) {
                  const fetchPromise = fetch(filePath+'?'+Math.random())
                    .then(response => response.text())
                    .then(fileContent => {
                      // Update the slide with the file content
                      console.log(filePath, fileContent);
                      metaData.slides[slideId] = fileContent.trim();
                    })
                    .catch(error => {
                      console.error(`Failed to fetch slide file: ${filePath}`, error);
                    });

                  fetchPromises.push(fetchPromise);
                }
              }
            });
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

      fetch('default.njn?' + Math.random())
        .then(res => res.text())
        .then(data => {
          this.processConfigText(data);
        })
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

  loadAllScenes() {
    console.log('Loading all scenes...');
    let sceneIndex = 0;
    
    const loadNextScene = () => {
      if (sceneIndex >= this.allScenes.length) {
        console.log('All scenes loaded, starting...');
        this.start();
        return;
      }
      
      const sceneContent = this.allScenes[sceneIndex];
      console.log(`Loading scene ${sceneIndex + 1}/${this.allScenes.length}`);
      
      this.loadSceneToRegistry(sceneContent, sceneIndex, () => {
        sceneIndex++;
        loadNextScene();
      });
    };
    
    loadNextScene();
  },

  loadSceneToRegistry(sceneContent, sceneIndex, callback) {
    console.log(`Processing scene ${sceneIndex}:`, sceneContent.substring(0, 100) + '...');
    
    // Process the scene content similar to processConfigTextDirect
    this.currentCommentBlock = null;
    const flushCommentBlock = () => {
      if (this.currentCommentBlock && this.currentCommentBlock.content.length > 0) {
        const logObj = { log: { level: this.currentCommentBlock.level } };
        logObj.log[""] = this.currentCommentBlock.content.join("\n");
        this.js_to_c(logObj);
        this.currentCommentBlock = null;
      }
    };

    const configstr = sceneContent.split("\n").filter(line => {
      if (line.length === 0) {
        flushCommentBlock();
        return false;
      }
      if (line.startsWith("--")) {
        // Comment processing logic
        if (!this.currentCommentBlock) {
          this.currentCommentBlock = { level: "CONSOLE", messageType: "", content: [] };
          const symbol = line.substring(3, 4);
          if (symbol === "@") this.currentCommentBlock.level = "INFO";
          else if (symbol === "#") this.currentCommentBlock.level = "DEBUG";
          else if (symbol === "*") this.currentCommentBlock.level = "WARN";
          else if (symbol === "!") this.currentCommentBlock.level = "ERROR";
        }
        const content = line.substring(line.indexOf("--") + 2).trim();
        if (content) {
          this.currentCommentBlock.content.push(content);
        } else if (this.currentCommentBlock.content.length > 0) {
          flushCommentBlock();
        }
        return false;
      } else if (this.currentCommentBlock) {
        flushCommentBlock();
      }
      return true;
    });
    
    // Process entities and metadata
    const entities = [];
    const metaData = {};
    
    configstr.forEach(input => {
      const builder = new EntityBuilder().parseInput(input);
      const entity = builder.build();
      
      if (entity.meta) {
        Object.assign(metaData, entity.meta);
      } else {
        entities.push(entity);
      }
    });
    
    // Process file references
    const fetchPromises = [];
    
    // Handle meta file references
    Object.keys(metaData).forEach(key => {
      const value = metaData[key];
      if (typeof value === 'string' && value.startsWith('@')) {
        const filePath = value.substring(1).trim();
        const fetchPromise = fetch(filePath+'?'+Math.random())
          .then(response => response.text())
          .then(fileContent => {
            metaData[key] = fileContent;
          })
          .catch(error => {
            console.error(`Failed to fetch meta file: ${filePath}`, error);
          });
        fetchPromises.push(fetchPromise);
      }
    });
    
    // Handle text component file references
    entities.forEach(entity => {
      if (entity.Components && entity.Components.Text) {
        const text = entity.Components.Text.text;
        if (text.startsWith('@')) {
          const filePath = text.substring(1).trim();
          const fetchPromise = fetch(filePath+'?'+Math.random())
            .then(response => response.text())
            .then(fileContent => {
              entity.Components.Text.text = fileContent;
            })
            .catch(error => {
              console.error(`Failed to fetch file: ${filePath}`, error);
            });
          fetchPromises.push(fetchPromise);
        }
      }
    });
    
    // Wait for all fetches and send to C++
    Promise.all(fetchPromises)
      .then(() => {
        // Send to C++ using createSceneFromJson
        const sceneData = {
          Entities: entities,
          meta: metaData
        };
        console.log(sceneData);
        
        
        if (sceneIndex === 0) {
          // First scene - use regular load_json
          this.js_to_c(sceneData);
        } else {
          // Additional scenes - use createSceneFromJson
          this.js_to_c_scene(sceneData);
        }

        if (callback) callback();
      })
      .catch(error => {
        console.error(`Error processing scene ${sceneIndex}:`, error);
        if (callback) callback();
      });
  },

  js_to_c_scene(str) {
    if (typeof str === 'object') {
      this.engine_json.push(str);
      str = JSON.stringify(str);
    }


    const strPtr = this._malloc(str.length + 1);
    this.stringToUTF8(str, strPtr, str.length + 1);
    this._createSceneFromJson(strPtr);
    this._free(strPtr);
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

      this.engine_json.push(str);
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
  
  processConfigTextDirect(configText) {
    // Direct processing without multi-scene parsing - for file references
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
        // Comment processing logic (same as original)
        if (!this.currentCommentBlock) {
          this.currentCommentBlock = {
            level: "CONSOLE",
            messageType: "",
            content: []
          };
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

        const content = line.substring(line.indexOf("--") + 2).trim();
        if (content) {
          this.currentCommentBlock.content.push(content);
        } else if (this.currentCommentBlock.content.length > 0) {
          flushCommentBlock();
        }

        return false;
      } else if (this.currentCommentBlock) {
        flushCommentBlock();
      }

      return true;
    });
    
    // Process the rest of the config (entities and meta) - same as original logic
    const entities = [];
    const metaData = {};
    this.metadata = [];

    configstr.forEach(input => {
      const builder = new EntityBuilder().parseInput(input);
      const entity = builder.build();
      
      if (entity.meta) {
        Object.assign(metaData, entity.meta);
        this.metadata.push(entity.meta);
      } 
      else {
        entities.push(entity);
      }
    });
    
    // Process meta data to update HTML
    if (Object.keys(metaData).length > 0) {
      this.processMeta(metaData);
    }

    // Handle async file fetches for text components and meta tags
    const fetchPromises = [];

    // Process meta tags for file references
    Object.keys(metaData).forEach(key => {
      const value = metaData[key];
      if (typeof value === 'string' && value.startsWith('@')) {
        const filePath = value.substring(1).trim();
        const fetchPromise = fetch(filePath+'?'+Math.random())
          .then(response => response.text())
          .then(fileContent => {
            metaData[key] = fileContent;
          })
          .catch(error => {
            console.error(`Failed to fetch meta file: ${filePath}`, error);
          });
        fetchPromises.push(fetchPromise);
      } else if (key === 'slides' && typeof value === 'object') {
        Object.keys(value).forEach(slideId => {
          const slideText = value[slideId];
          if (typeof slideText === 'string' && slideText.startsWith('@')) {
            const filePath = slideText.substring(1).trim();
            const fetchPromise = fetch(filePath+'?'+Math.random())
              .then(response => response.text())
              .then(fileContent => {
                metaData.slides[slideId] = fileContent.trim();
              })
              .catch(error => {
                console.error(`Failed to fetch slide file: ${filePath}`, error);
              });
            fetchPromises.push(fetchPromise);
          }
        });
      }
    });

    // Process text components
    entities.forEach(entity => {
      if (entity.Components && entity.Components.Text) {
        const text = entity.Components.Text.text;
        if (text.startsWith('@')) {
          const filePath = text.substring(1).trim();
          const fetchPromise = fetch(filePath+'?'+Math.random())
            .then(response => response.text())
            .then(fileContent => {
              entity.Components.Text.text = fileContent;
            })
            .catch(error => {
              console.error(`Failed to fetch file: ${filePath}`, error);
            });
          fetchPromises.push(fetchPromise);
        }
      }
    });

    // Wait for all fetches and send to C++
    Promise.all(fetchPromises)
      .then(() => {
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
        console.error("Error processing direct config:", error);
        ECONFIG = {
          "Entities": entities
        };
        this.js_to_c(ECONFIG);
        this.start();
      });
  },
  
  parseMultiSceneConfig(configText) {
    const lines = configText.split('\n');
    const scenes = [];
    let currentScene = null;
    
    for (let line of lines) {
      const trimmedLine = line.trim();
      
      // Skip comment lines (starting with --)
      if (trimmedLine.startsWith('--') || trimmedLine.length === 0) {
        continue;
      }
      
      // Check if this is a scene meta tag
      if (trimmedLine.startsWith('meta scene ')) {
        // Save previous scene if it exists
        if (currentScene) {
          console.log('Saving scene:', currentScene.name, 'with', currentScene.content.length, 'lines');
          scenes.push({
            name: currentScene.name,
            content: currentScene.content.join('\n'),
            isFileReference: currentScene.isFileReference,
            filePath: currentScene.filePath
          });
        }
        
        // Extract scene name/path from meta scene line
        const sceneValue = trimmedLine.substring('meta scene '.length).trim();
        // Remove quotes if present
        const sceneName = sceneValue.replace(/^["']|["']$/g, '');
        console.log('Found scene:', sceneName);
        
        // Start new scene
        currentScene = { 
          name: sceneName, 
          content: [trimmedLine] // Include the meta scene line itself
        };
        
        // If scene name starts with @, it's a file reference
        if (sceneName.startsWith('@')) {
          currentScene.isFileReference = true;
          currentScene.filePath = sceneName.substring(1);
          console.log('Scene is file reference:', currentScene.filePath);
        }
      } else if (currentScene) {
        // Add line to current scene
        currentScene.content.push(trimmedLine);
      }
      // If no currentScene and not a comment/empty line, this is pre-scene content
      // For now, we ignore pre-scene content since all content should be in scenes
    }
    
    // Add the last scene if it exists
    if (currentScene) {
      console.log('Adding final scene:', currentScene.name, 'with', currentScene.content.length, 'lines');
      scenes.push({
        name: currentScene.name,
        content: currentScene.content.join('\n'),
        isFileReference: currentScene.isFileReference,
        filePath: currentScene.filePath
      });
    }
    
    // If no scenes found, return original config as primary
    if (scenes.length === 0) {
      return {
        primary_scene: configText,
        alt_scenes: []
      };
    }
    
    // First scene is primary, rest are alternatives  
    const primary_scene = scenes[0].content;
    const alt_scenes = scenes.slice(1);
    
    // Check if primary scene is a file reference
    const primary_scene_is_file_ref = scenes[0].isFileReference || false;
    const primary_scene_file_path = scenes[0].filePath || '';
    
    return {
      primary_scene,
      alt_scenes,
      primary_scene_is_file_ref,
      primary_scene_file_path
    };
  }
}
