// Blueprint management for reusable entity templates
import { getComponent } from './components.js';

const BLUEPRINT_PREFIX = 'augame_blueprint_';
const DEFAULT_BLUEPRINTS_KEY = 'augame_default_blueprints';

export class BlueprintManager {
    constructor() {
        this.blueprints = new Map();
        this.defaultBlueprints = [];
        this.loadBlueprints();
        this.loadDefaultBlueprints();
    }

    // Load blueprints from localStorage
    loadBlueprints() {
        this.blueprints.clear();

        for (let i = 0; i < localStorage.length; i++) {
            const key = localStorage.key(i);
            if (key.startsWith(BLUEPRINT_PREFIX)) {
                const name = key.replace(BLUEPRINT_PREFIX, '');
                try {
                    const data = JSON.parse(localStorage.getItem(key));
                    this.blueprints.set(name, data);
                } catch (e) {
                    console.warn(`Failed to load blueprint: ${name}`);
                }
            }
        }
    }

    // Load default blueprints from defaultBlueprints.js
    async loadDefaultBlueprints() {
        try {
            const module = await import('./defaultBlueprints.js');
            if (module.defaultBlueprints) {
                this.defaultBlueprints = module.defaultBlueprints;

                // Add default blueprints to the main blueprints map if they don't exist
                this.defaultBlueprints.forEach(bp => {
                    if (!this.blueprints.has(bp.name)) {
                        this.blueprints.set(bp.name, bp);
                    }
                });
            }
        } catch (e) {
            // No default blueprints file or error loading it
            console.log('No default blueprints loaded');
        }
    }

    // Save a blueprint (single entity or multiple)
    saveBlueprint(name, entities) {
        if (!name || !entities) return false;

        // Ensure entities is an array
        const entityArray = Array.isArray(entities) ? entities : [entities];
        if (entityArray.length === 0) return false;

        // Create blueprint data structure
        const blueprint = {
            name: name,
            entities: entityArray.map(e => this.cloneEntity(e)),
            created: new Date().toISOString()
        };

        // Save to localStorage
        localStorage.setItem(BLUEPRINT_PREFIX + name, JSON.stringify(blueprint));

        // Update internal map
        this.blueprints.set(name, blueprint);

        return true;
    }

    // Update an existing blueprint
    updateBlueprint(name, entities) {
        if (!this.blueprints.has(name)) {
            return this.saveBlueprint(name, entities);
        }

        if (confirm(`Update existing blueprint "${name}"?`)) {
            return this.saveBlueprint(name, entities);
        }
        return false;
    }

    // Load a blueprint and return new entities
    loadBlueprint(name, offsetX = 0, offsetY = 0) {
        const blueprint = this.blueprints.get(name);
        if (!blueprint) return null;

        // Handle both old format (single entity) and new format (with entities array)
        let entities = [];
        if (blueprint.entities) {
            // New format
            entities = blueprint.entities.map(e => this.cloneEntity(e));
        } else {
            // Old format - treat the whole blueprint as a single entity
            entities = [this.cloneEntity(blueprint)];
        }

        // Apply offset to positions
        entities.forEach(entity => {
            const pos = getComponent(entity, 'Position');
            if (pos) {
                pos.x += offsetX;
                pos.y += offsetY;
            }
        });

        return entities;
    }

    // Delete a blueprint
    deleteBlueprint(name) {
        localStorage.removeItem(BLUEPRINT_PREFIX + name);
        this.blueprints.delete(name);
    }

    // Get all blueprint names
    getBlueprintNames() {
        return Array.from(this.blueprints.keys()).sort();
    }

    // Clone an entity (deep copy)
    cloneEntity(entity) {
        return {
            id: entity.id,
            name: entity.name,
            components: entity.components.map(c => ({ ...c }))
        };
    }

    // Export all blueprints as JavaScript code
    exportAsJavaScript() {
        const userBlueprints = [];

        this.blueprints.forEach((blueprint, name) => {
            // Skip default blueprints in export
            if (this.defaultBlueprints.some(db => db.name === name)) {
                return;
            }
            userBlueprints.push(blueprint);
        });

        const code = `// Default blueprints for AuGame editor
// Generated on ${new Date().toISOString()}

export const defaultBlueprints = ${JSON.stringify(userBlueprints, null, 2)};
`;
        return code;
    }

    // Export blueprints as JSON for sharing
    exportAsJSON() {
        const blueprints = {};
        this.blueprints.forEach((blueprint, name) => {
            blueprints[name] = blueprint;
        });
        return JSON.stringify(blueprints, null, 2);
    }

    // Import blueprints from JSON
    importFromJSON(jsonString) {
        try {
            const blueprints = JSON.parse(jsonString);
            let imported = 0;

            Object.entries(blueprints).forEach(([name, blueprint]) => {
                // Check if blueprint already exists
                if (this.blueprints.has(name)) {
                    if (!confirm(`Blueprint "${name}" already exists. Overwrite?`)) {
                        return;
                    }
                }

                // Save the blueprint
                localStorage.setItem(BLUEPRINT_PREFIX + name, JSON.stringify(blueprint));
                this.blueprints.set(name, blueprint);
                imported++;
            });

            return imported;
        } catch (e) {
            console.error('Failed to import blueprints:', e);
            return 0;
        }
    }

    // Clear all user blueprints (not defaults)
    clearUserBlueprints() {
        const toDelete = [];
        this.blueprints.forEach((blueprint, name) => {
            // Don't delete default blueprints
            if (!this.defaultBlueprints.some(db => db.name === name)) {
                toDelete.push(name);
            }
        });

        toDelete.forEach(name => this.deleteBlueprint(name));
    }
}