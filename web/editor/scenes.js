// Scene management for multi-scene support
import { MetaSchemas } from '../scripts/ebuilder.js';
import { EntityManager } from './entities.js';

export class SceneManager {
    constructor() {
        this.scenes = [];
        this.currentSceneIndex = 0;
        this.init();
    }

    init() {
        // Create initial scene
        this.createScene('Scene 1');
    }

    createScene(name = null) {
        const sceneNum = this.scenes.length + 1;
        const scene = {
            name: name || `Scene ${sceneNum}`,
            entityManager: new EntityManager(),
            meta: this.getDefaultMeta()
        };
        this.scenes.push(scene);
        return scene;
    }

    getDefaultMeta() {
        const meta = {};
        Object.entries(MetaSchemas).forEach(([key, schema]) => {
            if (schema.type === 'object') {
                meta[key] = {};
            } else {
                meta[key] = schema.default || '';
            }
        });
        return meta;
    }

    deleteScene(index) {
        if (this.scenes.length <= 1) {
            console.warn('Cannot delete last scene');
            return false;
        }

        this.scenes.splice(index, 1);

        // Adjust current index if needed
        if (this.currentSceneIndex >= this.scenes.length) {
            this.currentSceneIndex = this.scenes.length - 1;
        }

        // Rename remaining scenes
        this.scenes.forEach((scene, i) => {
            if (scene.name.match(/^Scene \d+$/)) {
                scene.name = `Scene ${i + 1}`;
            }
        });

        return true;
    }

    duplicateScene(index) {
        const original = this.scenes[index];
        const copy = {
            name: original.name + ' (copy)',
            entityManager: new EntityManager(),
            meta: JSON.parse(JSON.stringify(original.meta))
        };

        // Copy entities
        copy.entityManager.setState(original.entityManager.getState());

        this.scenes.splice(index + 1, 0, copy);
        return copy;
    }

    switchToScene(index) {
        if (index >= 0 && index < this.scenes.length) {
            this.currentSceneIndex = index;
            return true;
        }
        return false;
    }

    getCurrentScene() {
        return this.scenes[this.currentSceneIndex];
    }

    getCurrentEntityManager() {
        return this.getCurrentScene().entityManager;
    }

    getCurrentMeta() {
        return this.getCurrentScene().meta;
    }

    updateSceneName(index, newName) {
        if (index >= 0 && index < this.scenes.length) {
            this.scenes[index].name = newName;
        }
    }

    updateMeta(key, value) {
        const meta = this.getCurrentMeta();
        meta[key] = value;
    }

    // Export single scene
    exportScene(index) {
        const scene = this.scenes[index];
        let output = [];

        // Add meta tags
        Object.entries(scene.meta).forEach(([key, value]) => {
            if (value && value !== MetaSchemas[key]?.default) {
                if (key === 'slides' && typeof value === 'object') {
                    // Handle slides object
                    Object.entries(value).forEach(([slideId, slideText]) => {
                        if (slideText) {
                            output.push(`meta slides ${slideId} "${slideText}"`);
                        }
                    });
                } else if (value) {
                    // Quote string values
                    const quotedValue = typeof value === 'string' && value.includes(' ')
                        ? `"${value}"`
                        : value;
                    output.push(`meta ${key} ${quotedValue}`);
                }
            }
        });

        // Add blank line between meta and entities if there are meta tags
        if (output.length > 0) {
            output.push('');
        }

        // Export entities (handled by exporter)
        return output.join('\n');
    }

    // Export multiple scenes as multi-scene config
    exportMultipleScenes(indices) {
        const outputs = [];

        indices.forEach(index => {
            const scene = this.scenes[index];
            let sceneOutput = [];

            // Always include scene meta tag for multi-scene
            const sceneName = scene.meta.scene || scene.name;
            sceneOutput.push(`meta scene "${sceneName}"`);

            // Add other meta tags
            Object.entries(scene.meta).forEach(([key, value]) => {
                if (key === 'scene') return; // Already added

                if (value && value !== MetaSchemas[key]?.default) {
                    if (key === 'slides' && typeof value === 'object') {
                        Object.entries(value).forEach(([slideId, slideText]) => {
                            if (slideText) {
                                sceneOutput.push(`meta slides ${slideId} "${slideText}"`);
                            }
                        });
                    } else if (value) {
                        const quotedValue = typeof value === 'string' && value.includes(' ')
                            ? `"${value}"`
                            : value;
                        sceneOutput.push(`meta ${key} ${quotedValue}`);
                    }
                }
            });

            outputs.push(sceneOutput.join('\n'));
        });

        // Join scenes with double newline
        return outputs.join('\n\n');
    }

    // Load from saved state
    getState() {
        return {
            scenes: this.scenes.map(scene => ({
                name: scene.name,
                entityState: scene.entityManager.getState(),
                meta: scene.meta
            })),
            currentSceneIndex: this.currentSceneIndex
        };
    }

    setState(state) {
        this.scenes = state.scenes.map(sceneData => {
            const entityManager = new EntityManager();
            entityManager.setState(sceneData.entityState);
            return {
                name: sceneData.name,
                entityManager: entityManager,
                meta: sceneData.meta || this.getDefaultMeta()
            };
        });
        this.currentSceneIndex = state.currentSceneIndex || 0;
    }
}