// Main application
import { COMPONENT_SCHEMAS, getComponent, ensureComponent, removeComponent } from './components.js';
import { MetaSchemas } from '../scripts/ebuilder.js';
import { SceneManager } from './scenes.js';
import { CanvasRenderer } from './canvas.js';
import { Exporter } from './exporter.js';
import { BlueprintManager } from './blueprints.js';

// Undo/Redo Manager
class UndoManager {
    constructor(maxHistory = 100) {
        this.history = [];
        this.currentIndex = -1;
        this.maxHistory = maxHistory;
    }

    pushState(state) {
        // Truncate forward history if we're not at the end
        this.history = this.history.slice(0, this.currentIndex + 1);

        // Add new state as JSON string
        this.history.push(JSON.stringify(state));
        this.currentIndex++;

        // Limit history size - pop oldest when exceeded
        if (this.history.length > this.maxHistory) {
            this.history.shift();
            this.currentIndex--;
        }
    }

    canUndo() {
        return this.currentIndex > 0;
    }

    canRedo() {
        return this.currentIndex < this.history.length - 1;
    }

    undo() {
        if (this.canUndo()) {
            this.currentIndex--;
            return JSON.parse(this.history[this.currentIndex]);
        }
        return null;
    }

    redo() {
        if (this.canRedo()) {
            this.currentIndex++;
            return JSON.parse(this.history[this.currentIndex]);
        }
        return null;
    }

    clear() {
        this.history = [];
        this.currentIndex = -1;
    }

    getHistoryInfo() {
        return {
            total: this.history.length,
            current: this.currentIndex + 1,
            canUndo: this.canUndo(),
            canRedo: this.canRedo()
        };
    }
}

class SceneEditor {
    constructor() {
        this.sceneManager = new SceneManager();
        this.blueprintManager = new BlueprintManager();
        this.undoManager = new UndoManager(100); // 100 action history limit
        this.canvas = document.getElementById('canvas');

        // Initialize renderer with first scene's entity manager
        this.renderer = new CanvasRenderer(
            this.canvas,
            this.sceneManager.getCurrentEntityManager()
        );

        this.setupUI();
        this.setupEventHandlers();
        this.loadAutosave();
        this.updateSceneTabs();
        this.setupMetaPanel();

        // Set up terrain bounds callback for renderer
        this.renderer.getTerrainBounds = () => {
            const meta = this.sceneManager.getCurrentMeta();
            const bounds = meta.terrain_bounds;

            // If it's a string, parse it
            if (typeof bounds === 'string' && bounds.includes(',')) {
                const parsed = bounds.split(',').map(v => parseFloat(v.trim()));
                if (parsed.length === 4) {
                    return parsed;
                }
            }

            // If it's already an array, return it
            if (Array.isArray(bounds) && bounds.length === 4) {
                return bounds;
            }

            return null;
        };

        // Wait for default blueprints to load before updating list
        this.initializeBlueprintList();

        // Save initial state for undo
        this.saveStateForUndo();

        // Initialize play link
        this.updatePlayLink();

        // Auto-save every 10 seconds
        setInterval(() => this.autosave(), 10000);
    }

    async initializeBlueprintList() {
        // Wait for default blueprints to load
        await this.blueprintManager.defaultBlueprintsLoaded;
        this.updateBlueprintList();
    }

    setupUI() {
        this.elements = {
            // Scene tabs
            sceneTabs: document.getElementById('sceneTabs'),
            addSceneBtn: document.getElementById('addSceneBtn'),

            // Mode buttons
            modeBtns: document.querySelectorAll('.mode-btn'),

            // Entity controls
            nextId: document.getElementById('nextId'),
            nextName: document.getElementById('nextName'),
            snapToggle: document.getElementById('snapToggle'),

            // Entity list
            entityList: document.getElementById('entityList'),
            deleteBtn: document.getElementById('deleteBtn'),
            duplicateBtn: document.getElementById('duplicateBtn'),
            clearBtn: document.getElementById('clearBtn'),
            undoBtn: document.getElementById('undoBtn'),
            redoBtn: document.getElementById('redoBtn'),

            // Export/Import
            exportText: document.getElementById('exportText'),
            exportBtn: document.getElementById('exportBtn'),
            copyBtn: document.getElementById('copyBtn'),
            importBtn: document.getElementById('importBtn'),
            exportMultiScene: document.getElementById('exportMultiScene'),
            includeEmptyMeta: document.getElementById('includeEmptyMeta'),
            playInEngineLink: document.getElementById('playInEngineLink'),
            reuseTabCheckbox: document.getElementById('reuseTabCheckbox'),

            // Storage
            sceneName: document.getElementById('sceneName'),
            saveBtn: document.getElementById('saveBtn'),
            sceneList: document.getElementById('sceneList'),

            // Blueprints
            saveBlueprintBtn: document.getElementById('saveBlueprintBtn'),
            loadBlueprintBtn: document.getElementById('loadBlueprintBtn'),
            blueprintList: document.getElementById('blueprintList'),
            exportBlueprintsBtn: document.getElementById('exportBlueprintsBtn'),
            importBlueprintsBtn: document.getElementById('importBlueprintsBtn'),

            // Panels
            componentPanel: document.getElementById('componentPanel'),
            componentList: document.getElementById('componentList'),
            metaPanel: document.getElementById('metaPanel'),
            metaFields: document.getElementById('metaFields'),
            toggleMetaBtn: document.getElementById('toggleMetaBtn'),

            // Status
            status: document.getElementById('status'),

            // Import Modal
            importModal: document.getElementById('importModal'),
            importMethod: document.getElementById('importMethod'),
            importTextArea: document.getElementById('importTextArea'),
            importTextInput: document.getElementById('importTextInput'),
            importUrlArea: document.getElementById('importUrlArea'),
            importUrlInput: document.getElementById('importUrlInput'),
            importExampleArea: document.getElementById('importExampleArea'),
            importExampleSelect: document.getElementById('importExampleSelect'),
            importReplace: document.getElementById('importReplace')
        };

        // Setup import method change handler
        this.elements.importMethod.addEventListener('change', () => {
            const method = this.elements.importMethod.value;
            this.elements.importTextArea.style.display = method === 'text' ? 'block' : 'none';
            this.elements.importUrlArea.style.display = method === 'url' ? 'block' : 'none';
            this.elements.importExampleArea.style.display = method === 'example' ? 'block' : 'none';
        });
    }

    setupEventHandlers() {
        // Scene tabs
        this.elements.addSceneBtn.addEventListener('click', () => this.addScene());

        // Meta panel toggle
        this.elements.toggleMetaBtn.addEventListener('click', () => {
            const isVisible = this.elements.metaPanel.style.display !== 'none';
            this.elements.metaPanel.style.display = isVisible ? 'none' : 'block';
        });

        // Mode switching
        this.elements.modeBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                this.elements.modeBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.renderer.setMode(btn.dataset.mode);
                if (btn.dataset.mode === 'edit' && this.renderer.entityManager.getSelected()) {
                    this.showComponentPanel();
                } else {
                    this.hideComponentPanel();
                }
            });
        });

        // Snap toggle
        this.elements.snapToggle.addEventListener('change', () => {
            this.renderer.setSnap(this.elements.snapToggle.checked);
        });

        // Entity controls
        this.elements.deleteBtn.addEventListener('click', () => this.deleteSelected());
        this.elements.duplicateBtn.addEventListener('click', () => this.duplicateSelected());
        this.elements.clearBtn.addEventListener('click', () => this.clearAll());
        this.elements.undoBtn.addEventListener('click', () => this.undo());
        this.elements.redoBtn.addEventListener('click', () => this.redo());

        // Export/Import
        this.elements.exportBtn.addEventListener('click', () => this.export());
        this.elements.copyBtn.addEventListener('click', () => this.copyToClipboard());
        this.elements.importBtn.addEventListener('click', () => this.showImportModal());

        // Play in Engine link
        this.elements.playInEngineLink.addEventListener('click', (e) => {
            e.preventDefault();
            const url = this.elements.playInEngineLink.href;
            const reuseTab = this.elements.reuseTabCheckbox.checked;

            if (reuseTab) {
                // Open in a named window (reuses same window each time)
                const currentScene = this.sceneManager.getCurrentScene();
                const sceneName = currentScene.meta.scene || currentScene.name;
                window.open(url, `augame_${sceneName}`);
            } else {
                // Open in new tab each time
                window.open(url, '_blank');
            }
        });

        // Storage
        this.elements.saveBtn.addEventListener('click', () => this.saveProject());

        // Blueprints
        this.elements.saveBlueprintBtn.addEventListener('click', () => this.saveBlueprint());
        this.elements.loadBlueprintBtn.addEventListener('click', () => this.loadBlueprintMenu());
        this.elements.exportBlueprintsBtn.addEventListener('click', () => this.exportBlueprints());
        this.elements.importBlueprintsBtn.addEventListener('click', () => this.importBlueprints());

        // Renderer callbacks
        this.renderer.onChange = () => {
            this.updateEntityList();
            this.updateNextValues();
            this.saveStateForUndo();
            this.updatePlayLink();
            this.autosave();
        };

        this.renderer.onSelectionChange = () => {
            this.updateEntityList();
            this.updateButtons();
            if (this.renderer.mode === 'edit' && this.renderer.entityManager.getSelected()) {
                this.showComponentPanel();
            } else {
                this.hideComponentPanel();
            }
        };

        // Multi-select callback
        this.renderer.onMultiSelectChange = (selectedEntities) => {
            this.elements.saveBlueprintBtn.disabled = selectedEntities.length === 0;
            if (selectedEntities.length > 0) {
                this.showStatus(`Selected ${selectedEntities.length} entities`);
            }
        };

        // Keyboard shortcuts
        document.addEventListener('keydown', e => {
            if (e.key === 'Delete' && this.renderer.entityManager.selectedIdx !== null) {
                this.deleteSelected();
            }
            if ((e.ctrlKey || e.metaKey) && e.key === 'd' && this.renderer.entityManager.selectedIdx !== null) {
                e.preventDefault();
                this.duplicateSelected();
            }
            if ((e.ctrlKey || e.metaKey) && e.key === 'c') {
                this.copyToClipboard();
            }
            if ((e.ctrlKey || e.metaKey) && e.key === 's') {
                e.preventDefault();
                this.saveProject();
            }
            // Undo/Redo
            if ((e.ctrlKey || e.metaKey) && e.key === 'z') {
                e.preventDefault();
                if (e.shiftKey) {
                    this.redo();
                } else {
                    this.undo();
                }
            }
            if ((e.ctrlKey || e.metaKey) && e.key === 'y') {
                e.preventDefault();
                this.redo();
            }
            // Scene switching with shift + < or >
            if (e.shiftKey) {
                if (e.key === '<') {
                    this.switchToPreviousScene();
                } else if (e.key === '>') {
                    this.switchToNextScene();
                }
            }
        });

        // Initial render
        this.updateEntityList();
        this.updateSceneList();
        this.renderer.render();
    }

    setupMetaPanel() {
        this.elements.metaFields.innerHTML = '';

        Object.entries(MetaSchemas).forEach(([key, schema]) => {
            const field = document.createElement('div');
            field.className = 'meta-field';

            const label = document.createElement('label');
            label.className = 'meta-label';
            label.textContent = schema.displayName;
            field.appendChild(label);

            let input;
            if (schema.type === 'select' && schema.options) {
                input = document.createElement('select');
                schema.options.forEach(opt => {
                    const option = document.createElement('option');
                    option.value = opt;
                    option.textContent = opt;
                    input.appendChild(option);
                });
            } else if (schema.type === 'object') {
                // For slides, create a textarea with example
                input = document.createElement('textarea');
                input.style.minHeight = '60px';
                input.placeholder = 'Format: slideId:text (one per line)\nExample:\nintro:Welcome to the game\nhelp:Press WASD to move';
            } else {
                input = document.createElement('input');
                input.type = 'text';
            }

            // Set the input value - handle slides object specially
            if (key === 'slides' && schema.type === 'object') {
                const slides = this.sceneManager.getCurrentMeta()[key] || {};
                if (typeof slides === 'object') {
                    // Convert slides object to text format
                    const lines = Object.entries(slides).map(([id, text]) => `${id}:${text}`);
                    input.value = lines.join('\n');
                } else {
                    input.value = '';
                }
            } else {
                input.value = this.sceneManager.getCurrentMeta()[key] || schema.default || '';
            }
            input.onchange = () => {
                if (key === 'slides' && schema.type === 'object') {
                    // Parse slides format
                    const slides = {};
                    input.value.split('\n').forEach(line => {
                        const [id, ...textParts] = line.split(':');
                        if (id && textParts.length > 0) {
                            slides[id.trim()] = textParts.join(':').trim();
                        }
                    });
                    this.sceneManager.updateMeta(key, slides);
                } else if (key === 'terrain_bounds') {
                    // Parse terrain_bounds value immediately
                    const value = input.value.trim();
                    if (value && value.includes(',')) {
                        const parsed = value.split(',').map(v => parseFloat(v.trim()));
                        if (parsed.length === 4 && parsed.every(v => !isNaN(v))) {
                            this.sceneManager.updateMeta(key, parsed);
                        } else {
                            this.sceneManager.updateMeta(key, value);
                        }
                    } else {
                        this.sceneManager.updateMeta(key, value);
                    }
                    // Force re-render to update visualization
                    this.renderer.render();
                } else {
                    this.sceneManager.updateMeta(key, input.value);
                    // If scene name changed, update the tab
                    if (key === 'scene') {
                        this.sceneManager.getCurrentScene().name = input.value;
                        this.updateSceneTabs();
                    }
                }
                this.saveStateForUndo();
                this.autosave();
            };

            field.appendChild(input);

            if (schema.description) {
                const desc = document.createElement('div');
                desc.className = 'meta-description';
                desc.textContent = schema.description;
                field.appendChild(desc);
            }

            this.elements.metaFields.appendChild(field);
        });
    }

    updateSceneTabs() {
        // Clear existing tabs (except add button)
        const addBtn = this.elements.addSceneBtn;
        this.elements.sceneTabs.innerHTML = '';

        this.sceneManager.scenes.forEach((scene, index) => {
            const tab = document.createElement('button');
            tab.className = 'tab';
            if (index === this.sceneManager.currentSceneIndex) {
                tab.classList.add('active');
            }
            tab.dataset.scene = index;

            // Create tab content with name and close button
            const nameSpan = document.createElement('span');
            nameSpan.textContent = scene.meta.scene || scene.name;
            nameSpan.className = 'tab-name';
            tab.appendChild(nameSpan);

            // Add close button if there's more than one scene
            if (this.sceneManager.scenes.length > 1) {
                const closeBtn = document.createElement('button');
                closeBtn.className = 'tab-close';
                closeBtn.textContent = '×';
                closeBtn.onclick = (e) => {
                    e.stopPropagation();
                    this.deleteScene(index);
                };
                tab.appendChild(closeBtn);
            }

            tab.onclick = () => this.switchToScene(index);

            // Double-click to rename
            tab.ondblclick = (e) => {
                e.stopPropagation();
                this.renameSceneTab(index);
            };

            this.elements.sceneTabs.appendChild(tab);
        });

        this.elements.sceneTabs.appendChild(addBtn);
    }

    renameSceneTab(index) {
        const scene = this.sceneManager.scenes[index];
        const currentName = scene.meta.scene || scene.name;
        const newName = prompt('Enter new scene name:', currentName);

        if (newName && newName.trim() && newName !== currentName) {
            // Update both the scene meta and the scene name
            this.sceneManager.updateMeta('scene', newName.trim());
            scene.name = newName.trim();

            // Update UI
            this.updateSceneTabs();
            this.setupMetaPanel(); // Refresh meta panel to show updated name
            this.saveStateForUndo();
            this.autosave();
            this.showStatus(`Scene renamed to "${newName.trim()}"`);
        }
    }

    addScene() {
        const name = prompt('Scene name:', `Scene ${this.sceneManager.scenes.length + 1}`);
        if (name) {
            const scene = this.sceneManager.createScene(name);
            this.switchToScene(this.sceneManager.scenes.length - 1);
        }
    }

    deleteScene(index) {
        // Don't delete if it's the only scene
        if (this.sceneManager.scenes.length <= 1) {
            alert('Cannot delete the only scene');
            return;
        }

        const scene = this.sceneManager.scenes[index];
        const sceneName = scene.meta.scene || scene.name;

        if (confirm(`Delete scene "${sceneName}"?\n\nThis will remove all entities in this scene.`)) {
            // If we're deleting the current scene, switch to another first
            let needsSwitch = false;
            let newIndex = this.sceneManager.currentSceneIndex;

            if (index === this.sceneManager.currentSceneIndex) {
                needsSwitch = true;
                // Switch to the previous scene, or the next if we're deleting the first one
                newIndex = index > 0 ? index - 1 : 0;
            } else if (index < this.sceneManager.currentSceneIndex) {
                // Adjust current index if deleting a scene before the current one
                this.sceneManager.currentSceneIndex--;
            }

            // Remove the scene
            this.sceneManager.scenes.splice(index, 1);

            // Switch if needed
            if (needsSwitch) {
                this.sceneManager.currentSceneIndex = newIndex;
                this.renderer.entityManager = this.sceneManager.getCurrentEntityManager();
            }

            // Update UI
            this.updateSceneTabs();
            this.updateEntityList();
            this.updateButtons();
            this.updateNextValues();
            this.setupMetaPanel();
            this.renderer.render();
            this.showStatus(`Scene "${sceneName}" deleted`);
            this.autosave();
        }
    }

    switchToScene(index) {
        if (this.sceneManager.switchToScene(index)) {
            // Update renderer's entity manager reference
            this.renderer.entityManager = this.sceneManager.getCurrentEntityManager();

            // Update UI
            this.updateSceneTabs();
            this.updateEntityList();
            this.updateButtons();
            this.updateNextValues();
            this.setupMetaPanel();
            this.updatePlayLink();
            this.renderer.render();
            this.hideComponentPanel();
        }
    }

    switchToNextScene() {
        const nextIndex = (this.sceneManager.currentSceneIndex + 1) % this.sceneManager.scenes.length;
        this.switchToScene(nextIndex);
    }

    switchToPreviousScene() {
        const prevIndex = this.sceneManager.currentSceneIndex === 0
            ? this.sceneManager.scenes.length - 1
            : this.sceneManager.currentSceneIndex - 1;
        this.switchToScene(prevIndex);
    }

    updateEntityList() {
        this.elements.entityList.innerHTML = '';

        const entityManager = this.sceneManager.getCurrentEntityManager();
        // Don't use sorted list - we want to show the actual creation order
        const entities = entityManager.entities;

        entities.forEach((entity, idx) => {
            const pos = getComponent(entity, 'Position');
            const shape = getComponent(entity, 'Shape');

            const div = document.createElement('div');
            div.className = 'entity-item';
            div.draggable = true;
            div.dataset.idx = idx;

            if (idx === entityManager.selectedIdx) {
                div.classList.add('selected');
            }

            let info = '';
            if (pos && shape) {
                info = ` <span class="entity-info">[${pos.x.toFixed(1)}, ${pos.y.toFixed(1)}] ${shape.w.toFixed(1)}×${shape.h.toFixed(1)}</span>`;
            }

            div.innerHTML = `<span style="cursor: grab;">⋮⋮</span> <span>${entity.id}: ${entity.name}${info}</span>`;

            div.onclick = () => {
                entityManager.select(idx);
                this.updateEntityList();
                this.updateButtons();
                this.renderer.render();
                if (this.renderer.mode === 'edit') {
                    this.showComponentPanel();
                }
            };

            // Drag and drop handlers
            div.ondragstart = (e) => {
                e.dataTransfer.effectAllowed = 'move';
                e.dataTransfer.setData('text/plain', idx);
                div.style.opacity = '0.5';
            };

            div.ondragend = (e) => {
                div.style.opacity = '1';
            };

            div.ondragover = (e) => {
                e.preventDefault();
                e.dataTransfer.dropEffect = 'move';
                div.style.borderTop = '2px solid #3a7bd5';
            };

            div.ondragleave = (e) => {
                div.style.borderTop = '';
            };

            div.ondrop = (e) => {
                e.preventDefault();
                div.style.borderTop = '';

                const fromIdx = parseInt(e.dataTransfer.getData('text/plain'));
                const toIdx = idx;

                if (entityManager.reorder(fromIdx, toIdx)) {
                    this.updateEntityList();
                    this.renderer.render();
                    this.saveStateForUndo();
                    this.autosave();
                    this.showStatus(`Moved entity to position ${toIdx + 1}`);
                }
            };

            this.elements.entityList.appendChild(div);
        });
    }

    updateButtons() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        const hasSelection = entityManager.selectedIdx !== null;
        const hasMultiSelection = this.renderer.selectedEntities.size > 0;

        this.elements.deleteBtn.disabled = !hasSelection;
        this.elements.duplicateBtn.disabled = !hasSelection;
        this.elements.saveBlueprintBtn.disabled = !hasSelection && !hasMultiSelection;
    }

    updateNextValues() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        const ids = entityManager.entities.map(e => e.id);
        const maxId = ids.length > 0 ? Math.max(...ids) : 0;
        this.elements.nextId.value = maxId + 1;
        this.elements.nextName.value = `entity${maxId + 1}`;
    }

    deleteSelected() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        if (entityManager.selectedIdx === null) return;
        entityManager.delete(entityManager.selectedIdx);
        this.updateEntityList();
        this.updateButtons();
        this.renderer.render();
        this.hideComponentPanel();
        this.saveStateForUndo();
        this.autosave();
    }

    duplicateSelected() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        if (entityManager.selectedIdx === null) return;
        const copy = entityManager.duplicate(entityManager.selectedIdx);
        if (copy) {
            entityManager.select(entityManager.entities.length - 1);
            this.updateEntityList();
            this.updateButtons();
            this.renderer.render();
            this.saveStateForUndo();
            this.autosave();
        }
    }

    clearAll() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        if (entityManager.entities.length > 0 &&
            !confirm('Clear all entities?')) {
            return;
        }
        entityManager.clear();
        this.updateEntityList();
        this.updateButtons();
        this.updateNextValues();
        this.renderer.render();
        this.hideComponentPanel();
        this.saveStateForUndo();
        this.autosave();
    }

    export() {
        let text;

        if (this.elements.exportMultiScene.checked) {
            // Export all scenes
            text = Exporter.exportMultipleScenes(
                this.sceneManager.scenes,
                this.elements.includeEmptyMeta.checked
            );
        } else {
            // Export current scene only
            const currentScene = this.sceneManager.getCurrentScene();
            text = Exporter.exportScene(
                currentScene,
                this.elements.includeEmptyMeta.checked
            );
        }

        this.elements.exportText.value = text;
        this.showStatus('Exported to text area');
    }

    copyToClipboard() {
        const text = this.elements.exportText.value;
        if (!text) {
            this.export();
        }
        navigator.clipboard.writeText(this.elements.exportText.value).then(() => {
            this.showStatus('Copied to clipboard');
        });
    }

    showImportModal() {
        this.elements.importModal.style.display = 'block';
        this.elements.importMethod.value = 'text';
        this.elements.importTextInput.value = '';
        this.elements.importUrlInput.value = '';
        this.elements.importExampleSelect.value = '/web/econfigs/default.txt';
        this.elements.importReplace.checked = true;

        // Show the correct input area
        this.elements.importTextArea.style.display = 'block';
        this.elements.importUrlArea.style.display = 'none';
        this.elements.importExampleArea.style.display = 'none';
    }

    cancelImport() {
        this.elements.importModal.style.display = 'none';
    }

    async confirmImport() {
        const method = this.elements.importMethod.value;
        let configText = null;

        try {
            if (method === 'text') {
                configText = this.elements.importTextInput.value;
                if (!configText.trim()) {
                    alert('Please paste config text');
                    return;
                }
            } else if (method === 'url') {
                const url = this.elements.importUrlInput.value;
                if (!url.trim()) {
                    alert('Please enter a URL');
                    return;
                }
                const response = await fetch(url);
                if (!response.ok) {
                    throw new Error(`Failed to fetch: ${response.statusText}`);
                }
                configText = await response.text();
            } else if (method === 'example') {
                const url = this.elements.importExampleSelect.value;
                const response = await fetch(url);
                if (!response.ok) {
                    throw new Error(`Failed to fetch example: ${response.statusText}`);
                }
                configText = await response.text();
            }

            // Process the config text
            this.processImportedConfig(configText);

            // Close modal
            this.elements.importModal.style.display = 'none';
        } catch (e) {
            alert('Import failed: ' + e.message);
        }
    }

    processImportedConfig(configText) {
        try {
            // Parse the config text to extract meta tags and entities
            const lines = configText.split('\n').filter(line => {
                const trimmed = line.trim();
                return trimmed && !trimmed.startsWith('--'); // Skip empty lines and comments
            });

            // Separate meta tags from entity definitions
            const metaLines = [];
            const entityLines = [];

            for (const line of lines) {
                if (line.trim().startsWith('meta ')) {
                    metaLines.push(line);
                } else if (line.trim().startsWith('id ')) {
                    entityLines.push(line);
                }
            }

            // Process meta tags
            if (metaLines.length > 0) {
                const meta = {};
                for (const line of metaLines) {
                    const parts = line.substring(5).trim().split(' '); // Remove 'meta ' prefix
                    const key = parts[0];

                    if (key === 'slides') {
                        // Handle slides specially
                        if (!meta.slides) meta.slides = {};
                        if (parts.length >= 3) {
                            const slideId = parts[1];
                            // Rest is the text, remove quotes if present
                            const text = parts.slice(2).join(' ').replace(/^"(.*)"$/, '$1');
                            meta.slides[slideId] = text;
                        }
                    } else if (key === 'scene') {
                        // Handle multi-scene imports - create new scene if needed
                        const sceneName = parts.slice(1).join(' ').replace(/^"(.*)"$/, '$1');
                        // Check if this is a new scene marker
                        if (entityLines.length > 0) {
                            // Process current accumulated entities before switching scenes
                            this.importEntitiesToCurrentScene(entityLines);
                            entityLines.length = 0; // Clear for next scene
                        }
                        // Add new scene or switch to existing
                        const existingIdx = this.sceneManager.scenes.findIndex(s =>
                            (s.meta.scene || s.name) === sceneName
                        );
                        if (existingIdx >= 0) {
                            this.switchToScene(existingIdx);
                        } else {
                            this.addScene();
                            this.sceneManager.updateMeta('scene', sceneName);
                        }
                        meta.scene = sceneName;
                    } else {
                        // Regular meta tag - join remaining parts and remove quotes
                        const value = parts.slice(1).join(' ').replace(/^"(.*)"$/, '$1');
                        meta[key] = value;
                    }
                }

                // Update current scene's meta (except scene tag which was handled)
                const { scene, ...otherMeta } = meta;
                Object.assign(this.sceneManager.getCurrentMeta(), otherMeta);
                this.setupMetaPanel(); // Refresh meta panel
            }

            // Process remaining entities
            if (entityLines.length > 0) {
                this.importEntitiesToCurrentScene(entityLines);
            }

            this.showStatus(`Import complete${metaLines.length > 0 ? ' with meta tags' : ''}`);
        } catch (e) {
            alert('Failed to process config: ' + e.message);
        }
    }

    importEntitiesToCurrentScene(entityLines) {
        const entities = Exporter.importEntities(entityLines.join('\n'));
        if (entities.length > 0) {
            const entityManager = this.sceneManager.getCurrentEntityManager();
            const shouldReplace = this.elements.importReplace.checked;

            if (shouldReplace) {
                entityManager.entities = entities;
                entityManager.selectedIdx = null;

                // Update nextId to be higher than all imported IDs
                const maxId = Math.max(...entities.map(e => e.id || 0));
                entityManager.nextId = maxId + 1;
            } else {
                // Append entities with new IDs
                entities.forEach(entity => {
                    entity.id = entityManager.nextId++;
                    entityManager.entities.push(entity);
                });
            }

            this.updateEntityList();
            this.updateButtons();
            this.updateNextValues();
            this.renderer.render();
            this.autosave();
        }
    }

    showComponentPanel() {
        const entityManager = this.sceneManager.getCurrentEntityManager();
        const entity = entityManager.getSelected();
        if (!entity) {
            this.hideComponentPanel();
            return;
        }

        this.elements.componentPanel.style.display = 'block';
        this.elements.componentList.innerHTML = '';

        // Add entity ID/Name editor first
        const entityInfo = document.createElement('div');
        entityInfo.className = 'component-item';
        entityInfo.innerHTML = `
            <div class="component-header">
                <div class="component-name">Entity Info</div>
            </div>
            <div class="component-fields">
                <div class="field-row">
                    <label class="field-label">ID</label>
                    <input type="number" id="entityIdInput" value="${entity.id}">
                </div>
                <div class="field-row">
                    <label class="field-label">Name</label>
                    <input type="text" id="entityNameInput" value="${entity.name}">
                </div>
            </div>
        `;
        this.elements.componentList.appendChild(entityInfo);

        // Add event handlers for entity info
        document.getElementById('entityIdInput').onchange = (e) => {
            entity.id = parseInt(e.target.value);
            this.updateEntityList();
            this.saveStateForUndo();
            this.updatePlayLink();
            this.autosave();
        };
        document.getElementById('entityNameInput').onchange = (e) => {
            entity.name = e.target.value;
            this.updateEntityList();
            this.saveStateForUndo();
            this.updatePlayLink();
            this.autosave();
        };

        // Add component editors for each schema (sorted alphabetically)
        const sortedSchemas = [...COMPONENT_SCHEMAS].sort((a, b) => a.type.localeCompare(b.type));
        sortedSchemas.forEach(schema => {
            const comp = getComponent(entity, schema.type);
            const item = document.createElement('div');
            item.className = 'component-item';

            const header = document.createElement('div');
            header.className = 'component-header';

            const name = document.createElement('div');
            name.className = 'component-name';
            name.textContent = schema.type;
            header.appendChild(name);

            if (schema.params.length === 0) {
                // Toggle component
                const toggle = document.createElement('input');
                toggle.type = 'checkbox';
                toggle.checked = !!comp;
                toggle.onchange = () => {
                    if (toggle.checked) {
                        ensureComponent(entity, schema.type);
                    } else {
                        removeComponent(entity, schema.type);
                    }
                    this.showComponentPanel();
                    this.renderer.render();
                    this.saveStateForUndo();
                    this.updatePlayLink();
                    this.autosave();
                };
                header.appendChild(toggle);
            } else if (comp) {
                // Remove button
                const removeBtn = document.createElement('button');
                removeBtn.textContent = 'Remove';
                removeBtn.className = 'btn-danger';
                removeBtn.style.padding = '2px 8px';
                removeBtn.style.fontSize = '11px';
                removeBtn.onclick = () => {
                    removeComponent(entity, schema.type);
                    this.showComponentPanel();
                    this.renderer.render();
                    this.saveStateForUndo();
                    this.updatePlayLink();
                    this.autosave();
                };
                header.appendChild(removeBtn);
            } else {
                // Add button
                const addBtn = document.createElement('button');
                addBtn.textContent = 'Add';
                addBtn.className = 'btn-success';
                addBtn.style.padding = '2px 8px';
                addBtn.style.fontSize = '11px';
                addBtn.onclick = () => {
                    ensureComponent(entity, schema.type);
                    this.showComponentPanel();
                    this.renderer.render();
                    this.saveStateForUndo();
                    this.updatePlayLink();
                    this.autosave();
                };
                header.appendChild(addBtn);
            }

            item.appendChild(header);

            // Add fields if component exists
            if (comp && schema.params.length > 0) {
                const fields = document.createElement('div');
                fields.className = 'component-fields';

                schema.params.forEach(param => {
                    const row = document.createElement('div');
                    row.className = 'field-row';

                    const label = document.createElement('label');
                    label.className = 'field-label';
                    label.textContent = param.label;
                    row.appendChild(label);

                    let input;
                    if (param.type === 'checkbox' || param.type === 'boolean') {
                        input = document.createElement('input');
                        input.type = 'checkbox';
                        input.checked = !!comp[param.name];
                        input.onchange = () => {
                            comp[param.name] = input.checked;
                            this.renderer.render();
                            this.saveStateForUndo();
                            this.updatePlayLink();
                            this.autosave();
                        };
                    } else if (param.type === 'text' || param.type === 'string') {
                        input = document.createElement('input');
                        input.type = 'text';
                        input.value = comp[param.name] || '';
                        input.onchange = () => {
                            comp[param.name] = input.value;
                            this.renderer.render();
                            this.saveStateForUndo();
                            this.updatePlayLink();
                            this.autosave();
                        };
                    } else {
                        input = document.createElement('input');
                        input.type = 'number';
                        input.value = comp[param.name] ?? param.default;
                        if (param.min !== undefined) input.min = param.min;
                        if (param.max !== undefined) input.max = param.max;
                        if (param.step !== undefined) input.step = param.step;
                        input.onchange = () => {
                            comp[param.name] = parseFloat(input.value);
                            this.renderer.render();
                            this.updateEntityList();
                            this.saveStateForUndo();
                            this.updatePlayLink();
                            this.autosave();
                        };
                    }

                    row.appendChild(input);
                    fields.appendChild(row);
                });

                item.appendChild(fields);
            }

            this.elements.componentList.appendChild(item);
        });
    }

    hideComponentPanel() {
        this.elements.componentPanel.style.display = 'none';
    }

    saveProject() {
        const name = this.elements.sceneName.value.trim();
        if (!name) {
            alert('Please enter a project name');
            return;
        }

        const key = `augame_project_${name}`;
        const data = JSON.stringify(this.sceneManager.getState());
        localStorage.setItem(key, data);

        // Also save individual scenes to be accessible via ?world= parameter
        this.sceneManager.scenes.forEach(scene => {
            const sceneName = scene.meta.scene || scene.name;
            const sceneKey = `augame_scene_${sceneName}`;
            const sceneConfig = Exporter.exportScene(scene, false);
            localStorage.setItem(sceneKey, sceneConfig);
        });

        this.updateSceneList();
        this.showStatus(`Project "${name}" saved (${this.sceneManager.scenes.length} scene(s))`);
    }

    loadProject(name) {
        const key = `augame_project_${name}`;
        const data = localStorage.getItem(key);
        if (!data) return;

        try {
            const state = JSON.parse(data);
            this.sceneManager.setState(state);

            // Update renderer reference
            this.renderer.entityManager = this.sceneManager.getCurrentEntityManager();

            // Update UI
            this.updateSceneTabs();
            this.updateEntityList();
            this.updateButtons();
            this.updateNextValues();
            this.setupMetaPanel();
            this.renderer.render();

            this.showStatus(`Project "${name}" loaded`);
        } catch (e) {
            alert('Failed to load project');
        }
    }

    deleteProject(name) {
        if (!confirm(`Delete project "${name}"?`)) return;
        const key = `augame_project_${name}`;
        localStorage.removeItem(key);
        this.updateSceneList();
        this.showStatus(`Project "${name}" deleted`);
    }

    updateSceneList() {
        this.elements.sceneList.innerHTML = '';

        const projects = [];
        for (let i = 0; i < localStorage.length; i++) {
            const key = localStorage.key(i);
            if (key.startsWith('augame_project_')) {
                projects.push(key.replace('augame_project_', ''));
            }
        }

        projects.sort().forEach(name => {
            const div = document.createElement('div');
            div.className = 'entity-item';
            div.innerHTML = `
                <span>${name}</span>
                <div>
                    <button onclick="editor.loadProject('${name}')" style="padding: 2px 8px; font-size: 11px;">Load</button>
                    <button onclick="editor.deleteProject('${name}')" class="btn-danger" style="padding: 2px 8px; font-size: 11px;">Delete</button>
                </div>
            `;
            this.elements.sceneList.appendChild(div);
        });
    }

    // Blueprint methods
    saveBlueprint() {
        let entitiesToSave = [];

        // Check for multi-selected entities
        if (this.renderer.selectedEntities.size > 0) {
            entitiesToSave = this.renderer.getSelectedEntities();
        }
        // Check for single selected entity
        else if (this.renderer.entityManager.selectedIdx !== null) {
            entitiesToSave = [this.renderer.entityManager.getSelected()];
        }

        if (entitiesToSave.length === 0) {
            alert('No entities selected');
            return;
        }

        const name = prompt(`Save blueprint with ${entitiesToSave.length} entity(ies). Enter name:`);
        if (!name || !name.trim()) return;

        if (this.blueprintManager.saveBlueprint(name.trim(), entitiesToSave)) {
            this.updateBlueprintList();
            this.showStatus(`Blueprint "${name}" saved with ${entitiesToSave.length} entity(ies)`);
        }
    }

    loadBlueprintMenu() {
        const names = this.blueprintManager.getBlueprintNames();
        if (names.length === 0) {
            alert('No blueprints available');
            return;
        }

        // Simple selection - in future could be a modal
        const name = prompt(`Available blueprints:\n${names.join('\n')}\n\nEnter blueprint name to load:`);
        if (!name || !names.includes(name)) return;

        const entities = this.blueprintManager.loadBlueprint(name, 0, 0);
        if (entities) {
            const entityManager = this.sceneManager.getCurrentEntityManager();

            // Add entities and generate new IDs
            entities.forEach(entity => {
                entity.id = entityManager.nextId++;
                entity.name = `entity${entity.id}`;

                // Position at center of canvas
                const pos = getComponent(entity, 'Position');
                if (pos) {
                    pos.x = Math.floor(this.canvas.width / 32);
                    pos.y = Math.floor(this.canvas.height / 32);
                }

                entityManager.entities.push(entity);
            });

            this.updateEntityList();
            this.updateNextValues();
            this.renderer.render();
            this.showStatus(`Loaded blueprint "${name}" with ${entities.length} entity(ies)`);
        }
    }

    updateBlueprintList() {
        this.elements.blueprintList.innerHTML = '';

        const names = this.blueprintManager.getBlueprintNames();
        names.forEach(name => {
            const div = document.createElement('div');
            div.className = 'entity-item';

            const blueprint = this.blueprintManager.blueprints.get(name);
            const entityCount = blueprint.entities ? blueprint.entities.length : 1;

            div.innerHTML = `
                <span>${name} (${entityCount} entities)</span>
                <div>
                    <button onclick="editor.loadBlueprint('${name}')" style="padding: 2px 8px; font-size: 11px;">Load</button>
                    <button onclick="editor.deleteBlueprint('${name}')" class="btn-danger" style="padding: 2px 8px; font-size: 11px;">Delete</button>
                </div>
            `;
            this.elements.blueprintList.appendChild(div);
        });
    }

    loadBlueprint(name) {
        // Get viewport center for placement
        const center = this.renderer.getViewCenter();

        // Load blueprint at center
        const entities = this.blueprintManager.loadBlueprint(name, center.x, center.y);
        if (entities) {
            const entityManager = this.sceneManager.getCurrentEntityManager();

            // Find the bounding box of the blueprint entities to center them properly
            let minX = Infinity, minY = Infinity;
            let maxX = -Infinity, maxY = -Infinity;

            entities.forEach(entity => {
                const pos = getComponent(entity, 'Position');
                if (pos) {
                    minX = Math.min(minX, pos.x);
                    minY = Math.min(minY, pos.y);
                    const shape = getComponent(entity, 'Shape');
                    if (shape) {
                        maxX = Math.max(maxX, pos.x + shape.w);
                        maxY = Math.max(maxY, pos.y + shape.h);
                    } else {
                        maxX = Math.max(maxX, pos.x);
                        maxY = Math.max(maxY, pos.y);
                    }
                }
            });

            // Calculate center offset
            const blueprintCenterX = (minX + maxX) / 2;
            const blueprintCenterY = (minY + maxY) / 2;
            const offsetX = center.x - blueprintCenterX;
            const offsetY = center.y - blueprintCenterY;

            // Store old ID -> new ID mapping for entity reference updates
            const idMapping = new Map();
            const loadedEntityIndices = [];

            // Add entities with proper offsets to center them
            entities.forEach(entity => {
                const oldId = entity.id;
                entity.id = entityManager.nextId++;
                entity.name = `entity${entity.id}`;

                // Store ID mapping
                idMapping.set(oldId, entity.id);

                // Apply centering offset
                const pos = getComponent(entity, 'Position');
                if (pos) {
                    pos.x += offsetX;
                    pos.y += offsetY;
                }

                const entityIndex = entityManager.entities.length;
                entityManager.entities.push(entity);
                loadedEntityIndices.push(entityIndex);
            });

            // Update entity references in components (InteriorPortal keys, etc.)
            entities.forEach(entity => {
                const portalComp = getComponent(entity, 'InteriorPortal');
                if (portalComp && portalComp.key !== undefined && portalComp.key !== -1) {
                    // Update key reference to new ID
                    if (idMapping.has(portalComp.key)) {
                        portalComp.key = idMapping.get(portalComp.key);
                    }
                }

                // Update A and B portal references if needed
                if (portalComp) {
                    if (portalComp.A !== undefined && portalComp.A !== -1 && idMapping.has(portalComp.A)) {
                        portalComp.A = idMapping.get(portalComp.A);
                    }
                    if (portalComp.B !== undefined && portalComp.B !== -1 && idMapping.has(portalComp.B)) {
                        portalComp.B = idMapping.get(portalComp.B);
                    }
                }

                // Update other component references as needed
                const insideComp = getComponent(entity, 'Inside');
                if (insideComp) {
                    // Handle both interiorEntity (from parser) and insideId (from schema/export)
                    const interiorRef = insideComp.interiorEntity !== undefined ? insideComp.interiorEntity : insideComp.insideId;
                    if (interiorRef !== undefined && idMapping.has(interiorRef)) {
                        const newId = idMapping.get(interiorRef);
                        insideComp.interiorEntity = newId;
                        insideComp.insideId = newId;
                    }
                }
            });

            // Clear single selection
            entityManager.select(null);

            // Select all loaded entities for multi-selection
            this.renderer.selectedEntities.clear();
            loadedEntityIndices.forEach(idx => {
                this.renderer.selectedEntities.add(idx);
            });

            this.updateEntityList();
            this.updateNextValues();
            this.updateButtons();
            this.renderer.render();
            this.showStatus(`Loaded blueprint "${name}" with ${entities.length} entities (all selected)`);

            // Notify multi-select change
            if (this.renderer.onMultiSelectChange) {
                this.renderer.onMultiSelectChange(this.renderer.getSelectedEntities());
            }
        }
    }

    deleteBlueprint(name) {
        if (!confirm(`Delete blueprint "${name}"?`)) return;
        this.blueprintManager.deleteBlueprint(name);
        this.updateBlueprintList();
        this.showStatus(`Blueprint "${name}" deleted`);
    }

    exportBlueprints() {
        const code = this.blueprintManager.exportAsJavaScript();

        // Create download
        const blob = new Blob([code], { type: 'text/javascript' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'blueprints.js';
        a.click();
        URL.revokeObjectURL(url);

        this.showStatus('Blueprints exported');
    }

    importBlueprints() {
        const json = prompt('Paste blueprint JSON:');
        if (!json) return;

        const count = this.blueprintManager.importFromJSON(json);
        if (count > 0) {
            this.updateBlueprintList();
            this.showStatus(`Imported ${count} blueprints`);
        } else {
            alert('Failed to import blueprints');
        }
    }

    saveStateForUndo() {
        const state = this.sceneManager.getState();
        this.undoManager.pushState(state);
        this.updateUndoButtons();
    }

    undo() {
        const state = this.undoManager.undo();
        if (state) {
            this.restoreState(state);
            this.showStatus('Undo');
        }
    }

    redo() {
        const state = this.undoManager.redo();
        if (state) {
            this.restoreState(state);
            this.showStatus('Redo');
        }
    }

    restoreState(state) {
        this.sceneManager.setState(state);

        // Update renderer reference
        this.renderer.entityManager = this.sceneManager.getCurrentEntityManager();

        // Update UI
        this.updateSceneTabs();
        this.updateEntityList();
        this.updateButtons();
        this.updateNextValues();
        this.setupMetaPanel();
        this.renderer.render();
        this.hideComponentPanel();
        this.updateUndoButtons();
    }

    updateUndoButtons() {
        this.elements.undoBtn.disabled = !this.undoManager.canUndo();
        this.elements.redoBtn.disabled = !this.undoManager.canRedo();
    }

    updatePlayLink() {
        const currentScene = this.sceneManager.getCurrentScene();
        const sceneName = currentScene.meta.scene || currentScene.name;

        // Save current scene to localStorage
        const sceneKey = `augame_scene_${sceneName}`;
        const sceneConfig = Exporter.exportScene(currentScene, false);
        localStorage.setItem(sceneKey, sceneConfig);

        // Update link URL
        const playUrl = `../?world=${encodeURIComponent(sceneName)}`;
        this.elements.playInEngineLink.href = playUrl;

        // Show toast notification
        this.showToast(`Scene "${sceneName}" saved and ready to play!`);
    }

    showToast(message, duration = 2000) {
        const toast = document.getElementById('toast');
        if (!toast) return;

        toast.textContent = message;
        toast.classList.add('show');

        setTimeout(() => {
            toast.classList.remove('show');
        }, duration);
    }

    autosave() {
        const data = JSON.stringify(this.sceneManager.getState());
        localStorage.setItem('augame_editor_autosave', data);
    }

    loadAutosave() {
        const data = localStorage.getItem('augame_editor_autosave');
        if (!data) return;

        try {
            const state = JSON.parse(data);
            this.sceneManager.setState(state);

            // Update renderer reference
            this.renderer.entityManager = this.sceneManager.getCurrentEntityManager();

            // Update UI
            this.updateSceneTabs();
            this.updateEntityList();
            this.updateButtons();
            this.updateNextValues();
            this.setupMetaPanel();
            this.renderer.render();
        } catch (e) {
            console.warn('Failed to load autosave');
        }
    }

    showStatus(message) {
        this.elements.status.textContent = message;
        this.elements.status.classList.add('show');
        setTimeout(() => {
            this.elements.status.classList.remove('show');
        }, 3000);
    }
}

// Initialize
const editor = new SceneEditor();
window.editor = editor; // For inline event handlers