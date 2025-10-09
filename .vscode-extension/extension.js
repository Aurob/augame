const vscode = require('vscode');

// Component definitions from ebuilder.js ComponentSchemas
const COMPONENTS = {
  'id': {
    type: 'Id',
    fields: [
      { name: 'id', type: 'number' },
      { name: 'name', type: 'string' }
    ],
    description: 'Entity identifier with numeric ID and name'
  },
  'position': {
    type: 'Position',
    fields: [
      { name: 'x', type: 'number', default: 0 },
      { name: 'y', type: 'number', default: 0 },
      { name: 'z', type: 'number', default: 0 }
    ],
    description: 'Entity position in 3D space'
  },
  'shape': {
    type: 'Shape',
    fields: [
      { name: 'w', type: 'number', default: 1 },
      { name: 'h', type: 'number', default: 1 },
      { name: 'd', type: 'number', default: 0 }
    ],
    description: 'Entity dimensions (width, height, depth)'
  },
  'color': {
    type: 'Color',
    fields: [
      { name: 'r', type: 'number', default: 1 },
      { name: 'g', type: 'number', default: 1 },
      { name: 'b', type: 'number', default: 1 },
      { name: 'a', type: 'number', default: 1 }
    ],
    description: 'RGBA color values (all 0-1 range)'
  },
  'renderPriority': {
    type: 'RenderPriority',
    fields: [
      { name: 'z', type: 'number', default: 0 }
    ],
    description: 'Render layer priority (higher = drawn on top)'
  },
  'player': {
    type: 'Player',
    fields: [],
    description: 'Marks entity as the player (no parameters)'
  },
  'collidable': {
    type: 'Collidable',
    fields: [],
    description: 'Entity participates in collision detection'
  },
  'moveable': {
    type: 'Moveable',
    fields: [],
    description: 'Entity can be moved by player'
  },
  'hoverable': {
    type: 'Hoverable',
    fields: [
      { name: 'duration', type: 'number', default: 0, optional: true }
    ],
    description: 'Entity highlights on mouse hover (can be used as tag or with duration parameter)'
  },
  'world': {
    type: 'World',
    fields: [],
    description: 'Marks entity as world entity'
  },
  'rgb_terrain': {
    type: 'Terrain',
    fields: [],
    description: 'Entity is terrain/ground'
  },
  'interior': {
    type: 'Interior',
    fields: [
      { name: 'showInside', type: 'boolean', default: true, optional: true }
    ],
    description: 'Entity is an interior space container (can be used as tag or with showInside parameter)'
  },
  'inside': {
    type: 'Inside',
    fields: [
      { name: 'insideId', type: 'number', default: 0 }
    ],
    description: 'Entity is inside another entity (by ID)'
  },
  'text': {
    type: 'Text',
    fields: [
      { name: 'text', type: 'string', default: '' },
      { name: 'scale', type: 'number', default: 1 },
      { name: 'hidden', type: 'boolean', default: false },
      { name: 'offsetX', type: 'number', default: 0 },
      { name: 'offsetY', type: 'number', default: 0 }
    ],
    description: 'Text rendering component'
  },
  'movement': {
    type: 'Movement',
    fields: [
      { name: 'speed', type: 'number', default: 0 },
      { name: 'mass', type: 'number', default: 1 },
      { name: 'restitution', type: 'number', default: 0 },
      { name: 'friction', type: 'number', default: 0 }
    ],
    description: 'Physics movement properties'
  },
  'interiorPortal': {
    type: 'InteriorPortal',
    fields: [
      { name: 'A', type: 'number', default: 0 },
      { name: 'B', type: 'number', default: 0 },
      { name: 'locked', type: 'boolean', default: false },
      { name: 'key', type: 'number', default: -1 }
    ],
    description: 'Portal connecting two interior entities'
  },
  'camera': {
    type: 'Camera',
    fields: [
      { name: 'gridSpacing', type: 'number', default: 1024 },
      { name: 'defaultGSV', type: 'number', default: 16 },
      { name: 'priority', type: 'number', default: 0 },
      { name: 'radius', type: 'number', default: 0 },
      { name: 'important', type: 'boolean', default: false }
    ],
    description: 'Camera view control'
  },
  'texture': {
    type: 'Texture',
    fields: [
      { name: 'name', type: 'string', default: '' },
      { name: 'scalex', type: 'number', default: 1 },
      { name: 'scaley', type: 'number', default: 1 },
      { name: 'x', type: 'number', default: 0 },
      { name: 'y', type: 'number', default: 0 },
      { name: 'w', type: 'number', default: 1 },
      { name: 'h', type: 'number', default: 1 }
    ],
    description: 'Texture rendering with UV coordinates'
  },
  'textureGroupPart': {
    type: 'TextureGroupPart',
    fields: [
      { name: 'groupName', type: 'string', default: '' },
      { name: 'partName', type: 'string', default: '' },
      { name: 'tilex', type: 'number', default: 0 },
      { name: 'tiley', type: 'number', default: 0 }
    ],
    description: 'Reference to part of a texture group'
  },
  'interactable': {
    type: 'Interactable',
    fields: [
      { name: 'radius', type: 'number', default: 0.5 },
      { name: 'toggleState', type: 'boolean', default: false }
    ],
    description: 'Entity can be clicked/interacted with'
  },
  'teleporter': {
    type: 'Teleporter',
    fields: [
      { name: 'destX', type: 'number', default: 0 },
      { name: 'destY', type: 'number', default: 0 },
      { name: 'destZ', type: 'number', default: 0 },
      { name: 'interiorEntity', type: 'number', default: 0 }
    ],
    description: 'Teleports entities to destination'
  },
  'teleportable': {
    type: 'Teleportable',
    fields: [],
    description: 'Entity can be teleported'
  },
  'draggable': {
    type: 'Draggable',
    fields: [
      { name: 'radius', type: 'number', default: 1 }
    ],
    description: 'Entity can be dragged by mouse'
  },
  'cshader': {
    type: 'CustomShader',
    fields: [
      { name: 'shaderName', type: 'string', default: '' },
      { name: 'centerX', type: 'number', default: 0 },
      { name: 'centerY', type: 'number', default: 0 },
      { name: 'seed', type: 'number', default: 0 }
    ],
    description: 'Custom shader component'
  },
  'meta': {
    type: 'Meta',
    fields: [
      { name: 'type', type: 'string' },
      { name: 'value', type: 'string' }
    ],
    description: 'World metadata (scene, terrain, void, font, etc.)'
  }
};

// Meta tag type definitions
const META_TAGS = {
  'scene': {
    description: 'Scene name or reference to another scene file',
    example: 'meta scene "world1" or meta scene "@file.txt"'
  },
  'title': {
    description: 'World title/name',
    example: 'meta title "My World"'
  },
  'description': {
    description: 'World description text',
    example: 'meta description "A test world"'
  },
  'author': {
    description: 'World author name',
    example: 'meta author "username"'
  },
  'font': {
    description: 'Font file path for text rendering',
    example: 'meta font "HomeVideo-Regular.ttf"'
  },
  'world': {
    description: 'Outside area rendering: shader name (terrain/tiles), hex color (#RRGGBB), or RGB (r,g,b)',
    example: 'meta world terrain\nmeta world "#ff0000"\nmeta world "128,64,32"'
  },
  'void': {
    description: 'Inside area clear color: hex (#RRGGBB) or RGB (r,g,b)',
    example: 'meta void "#000000"\nmeta void "0,0,0"'
  },
  'start_menu': {
    description: 'Start screen text or file reference',
    example: 'meta start_menu "Press Start"\nmeta start_menu "@menu.txt"'
  },
  'pause_menu': {
    description: 'Pause screen text or file reference',
    example: 'meta pause_menu "Paused"\nmeta pause_menu "@pause.txt"'
  },
  'seed': {
    description: 'World generation seed (string or number)',
    example: 'meta seed "Hello World"\nmeta seed "12345"'
  }
};

function activate(context) {
  // Diagnostics collection for validation errors
  const diagnosticsCollection = vscode.languages.createDiagnosticCollection('njn');
  context.subscriptions.push(diagnosticsCollection);

  // Semantic tokens legend
  const tokenTypes = ['keyword', 'property', 'string', 'number', 'comment', 'variable', 'type'];
  const tokenModifiers = ['declaration', 'readonly', 'meta'];
  const legend = new vscode.SemanticTokensLegend(tokenTypes, tokenModifiers);

  // Semantic tokens provider for syntax highlighting
  const semanticTokensProvider = {
    provideDocumentSemanticTokens(document) {
      const tokensBuilder = new vscode.SemanticTokensBuilder(legend);
      const componentNames = Object.keys(COMPONENTS);

      for (let lineNum = 0; lineNum < document.lineCount; lineNum++) {
        const line = document.lineAt(lineNum);
        const lineText = line.text;

        // Skip empty lines
        if (!lineText.trim()) continue;

        // Handle comments
        const commentIndex = lineText.indexOf('--');
        if (commentIndex !== -1) {
          tokensBuilder.push(
            new vscode.Range(lineNum, commentIndex, lineNum, lineText.length),
            'comment'
          );
        }

        // Get text before comment
        const codeText = commentIndex !== -1 ? lineText.substring(0, commentIndex) : lineText;

        // Tokenize with quote handling
        let pos = 0;
        let inQuote = false;
        let quoteChar = null;
        let currentToken = '';
        let tokenStart = 0;
        const tokens = [];

        for (let i = 0; i < codeText.length; i++) {
          const char = codeText[i];

          if ((char === '"' || char === "'") && !inQuote) {
            if (currentToken.trim()) {
              tokens.push({ text: currentToken.trim(), start: tokenStart, end: i });
            }
            inQuote = true;
            quoteChar = char;
            currentToken = char;
            tokenStart = i;
          } else if (char === quoteChar && inQuote) {
            currentToken += char;
            tokens.push({ text: currentToken, start: tokenStart, end: i + 1, isString: true });
            inQuote = false;
            quoteChar = null;
            currentToken = '';
            tokenStart = i + 1;
          } else if (/\s/.test(char) && !inQuote) {
            if (currentToken.trim()) {
              tokens.push({ text: currentToken.trim(), start: tokenStart, end: i });
            }
            currentToken = '';
            tokenStart = i + 1;
          } else {
            if (!currentToken) tokenStart = i;
            currentToken += char;
          }
        }

        if (currentToken.trim()) {
          tokens.push({ text: currentToken.trim(), start: tokenStart, end: codeText.length });
        }

        // Analyze tokens and add semantic highlighting
        let isMeta = false;
        let isId = false;
        let metaType = null;

        for (let i = 0; i < tokens.length; i++) {
          const token = tokens[i];
          const tokenText = token.text;

          // String literals
          if (token.isString) {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'string'
            );
            continue;
          }

          // Meta keyword
          if (tokenText === 'meta' && i === 0) {
            isMeta = true;
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'keyword',
              ['meta']
            );
            continue;
          }

          // Meta type (terrain, void, font, etc.)
          if (isMeta && i === 1) {
            metaType = tokenText;
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'type',
              ['meta']
            );
            continue;
          }

          // Meta value
          if (isMeta && i === 2) {
            // Determine if it's a reference (@file), hex color, or regular value
            if (tokenText.startsWith('@')) {
              tokensBuilder.push(
                new vscode.Range(lineNum, token.start, lineNum, token.end),
                'variable',
                ['readonly']
              );
            } else if (tokenText.startsWith('#')) {
              tokensBuilder.push(
                new vscode.Range(lineNum, token.start, lineNum, token.end),
                'number'
              );
            } else {
              tokensBuilder.push(
                new vscode.Range(lineNum, token.start, lineNum, token.end),
                'string'
              );
            }
            continue;
          }

          // Id keyword
          if (tokenText === 'id' && i === 0) {
            isId = true;
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'keyword',
              ['declaration']
            );
            continue;
          }

          // Entity ID (number after 'id')
          if (isId && i === 1 && /^-?\d+(\.\d+)?$/.test(tokenText)) {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'number',
              ['declaration']
            );
            continue;
          }

          // Entity name (string after id number)
          if (isId && i === 2) {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'variable',
              ['declaration']
            );
            continue;
          }

          // Component names
          if (componentNames.includes(tokenText)) {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'property'
            );
            continue;
          }

          // Numbers (component values)
          if (/^-?\d+(\.\d+)?$/.test(tokenText)) {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'number'
            );
            continue;
          }

          // Booleans
          if (tokenText === 'true' || tokenText === 'false') {
            tokensBuilder.push(
              new vscode.Range(lineNum, token.start, lineNum, token.end),
              'keyword'
            );
            continue;
          }

          // Other values (enums, names, etc.)
          tokensBuilder.push(
            new vscode.Range(lineNum, token.start, lineNum, token.end),
            'string'
          );
        }
      }

      return tokensBuilder.build();
    }
  };

  context.subscriptions.push(
    vscode.languages.registerDocumentSemanticTokensProvider(
      { language: 'njn' },
      semanticTokensProvider,
      legend
    )
  );

  // Parse a line and extract component names with their value counts
  function parseLineComponents(line) {
    const components = [];
    const componentNames = Object.keys(COMPONENTS);

    // Remove comments
    const cleanLine = line.split('--')[0].trim();
    if (!cleanLine || cleanLine.startsWith('--')) return components;

    // Tokenize with proper quote handling
    const tokens = [];
    let current = '';
    let inQuote = false;
    let quoteChar = null;

    for (let i = 0; i < cleanLine.length; i++) {
      const char = cleanLine[i];

      if ((char === '"' || char === "'") && !inQuote) {
        // Start of quoted string
        inQuote = true;
        quoteChar = char;
        current += char;
      } else if (char === quoteChar && inQuote) {
        // End of quoted string
        inQuote = false;
        current += char;
        quoteChar = null;
      } else if (/\s/.test(char) && !inQuote) {
        // Whitespace outside quotes - end current token
        if (current) {
          tokens.push(current);
          current = '';
        }
      } else {
        // Regular character
        current += char;
      }
    }

    // Add final token
    if (current) {
      tokens.push(current);
    }

    // If we have an unclosed quote, the line is malformed - don't validate
    if (inQuote) {
      return components;
    }

    // Find component boundaries
    for (let i = 0; i < tokens.length; i++) {
      const token = tokens[i];
      if (componentNames.includes(token)) {
        const comp = { name: token, startIndex: i, values: [] };

        // Collect values until next component or end of line
        for (let j = i + 1; j < tokens.length; j++) {
          if (componentNames.includes(tokens[j])) break;
          comp.values.push(tokens[j]);
        }

        components.push(comp);
      }
    }

    return components;
  }

  // Validate document and update diagnostics
  function validateDocument(document) {
    const diagnostics = [];

    for (let lineNum = 0; lineNum < document.lineCount; lineNum++) {
      const line = document.lineAt(lineNum);
      const lineText = line.text;

      const parsedComponents = parseLineComponents(lineText);

      for (const comp of parsedComponents) {
        const componentDef = COMPONENTS[comp.name];
        const expectedCount = componentDef.fields.length;
        const actualCount = comp.values.length;

        // Count required (non-optional) fields
        const requiredCount = componentDef.fields.filter(f => !f.optional).length;
        const allOptional = componentDef.fields.every(f => f.optional);

        // Validation logic:
        // - If all fields are optional, allow 0 or expectedCount
        // - Otherwise, actualCount must equal expectedCount
        const isValid = allOptional
          ? (actualCount === 0 || actualCount === expectedCount)
          : (actualCount === expectedCount);

        if (!isValid) {
          // Find the position of the component in the line
          const componentIndex = lineText.indexOf(comp.name);
          if (componentIndex === -1) continue;

          // Calculate the range for the entire component and its values
          const startPos = new vscode.Position(lineNum, componentIndex);
          const endPos = new vscode.Position(lineNum, componentIndex + comp.name.length);
          const range = new vscode.Range(startPos, endPos);

          let message;
          if (expectedCount === 0) {
            message = `Component '${comp.name}' should have no parameters (found ${actualCount})`;
          } else if (allOptional) {
            message = `Component '${comp.name}' expects 0 or ${expectedCount} value${expectedCount !== 1 ? 's' : ''} (${componentDef.fields.map(f => f.name).join(', ')}), but found ${actualCount}`;
          } else {
            message = `Component '${comp.name}' expects ${expectedCount} value${expectedCount !== 1 ? 's' : ''} (${componentDef.fields.map(f => f.name).join(', ')}), but found ${actualCount}`;
          }

          const diagnostic = new vscode.Diagnostic(
            range,
            message,
            vscode.DiagnosticSeverity.Error
          );

          diagnostic.source = 'njn-validator';
          diagnostics.push(diagnostic);
        }
      }
    }

    diagnosticsCollection.set(document.uri, diagnostics);
  }

  // Validate on document open and changes
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument(doc => {
      if (doc.languageId === 'njn') validateDocument(doc);
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidChangeTextDocument(event => {
      if (event.document.languageId === 'njn') validateDocument(event.document);
    })
  );

  // Validate all open documents on activation
  vscode.workspace.textDocuments.forEach(doc => {
    if (doc.languageId === 'njn') validateDocument(doc);
  });

  // Hover provider for component tooltips
  const hoverProvider = vscode.languages.registerHoverProvider('njn', {
    provideHover(document, position) {
      const range = document.getWordRangeAtPosition(position);
      if (!range) return;

      const word = document.getText(range);
      const line = document.lineAt(position.line).text;

      // Check if this is a meta tag type
      const metaMatch = line.match(/^\s*meta\s+(\w+)/);
      if (metaMatch && metaMatch[1] === word) {
        const metaTag = META_TAGS[word];
        if (metaTag) {
          const markdown = new vscode.MarkdownString(
            `**Meta Tag:** \`${word}\`\n\n${metaTag.description}\n\n**Example:**\n\`\`\`\n${metaTag.example}\n\`\`\``
          );
          return new vscode.Hover(markdown);
        }
      }

      // Check for component
      const component = COMPONENTS[word];
      if (component) {
        const fieldInfo = component.fields.length > 0
          ? '\n\n**Parameters:**\n' + component.fields.map(f =>
              `- \`${f.name}\` (${f.type})${f.default !== undefined ? ` = ${f.default}` : ''}${f.optional ? ' (optional)' : ''}`
            ).join('\n')
          : '\n\n*No parameters*';

        const markdown = new vscode.MarkdownString(
          `**${component.type}** component\n\n${component.description}${fieldInfo}`
        );
        return new vscode.Hover(markdown);
      }
    }
  });

  // Completion provider for smart autocomplete
  const completionProvider = vscode.languages.registerCompletionItemProvider('njn', {
    provideCompletionItems(document, position) {
      const linePrefix = document.lineAt(position).text.substr(0, position.character);

      // Check if we're at a position where we should suggest components
      const tokens = linePrefix.trim().split(/\s+/);
      const lastToken = tokens[tokens.length - 1];

      const completions = [];

      for (const [name, def] of Object.entries(COMPONENTS)) {
        const allOptional = def.fields.length > 0 && def.fields.every(f => f.optional);

        if (allOptional) {
          // Create two completion items for components with all optional fields

          // 1. Tag version (no parameters)
          const tagCompletion = new vscode.CompletionItem(
            `${name} (tag)`,
            vscode.CompletionItemKind.Keyword
          );
          tagCompletion.detail = `${def.type} (as tag)`;
          tagCompletion.documentation = new vscode.MarkdownString(
            def.description + '\n\n*Used as tag (no parameters)*'
          );
          tagCompletion.insertText = name;
          tagCompletion.sortText = `0_${name}`; // Sort before the version with params
          completions.push(tagCompletion);

          // 2. Full version (with parameters)
          const fullCompletion = new vscode.CompletionItem(
            `${name} (with params)`,
            vscode.CompletionItemKind.Keyword
          );
          fullCompletion.detail = `${def.type} (with parameters)`;
          const defaultValues = def.fields.map(f => {
            if (f.default !== undefined) {
              return f.type === 'string' ? `"${f.default}"` : f.default;
            }
            return f.type === 'number' ? '0' : f.type === 'boolean' ? 'false' : '""';
          }).join(' ');
          fullCompletion.insertText = `${name} ${defaultValues}`;
          fullCompletion.documentation = new vscode.MarkdownString(
            def.description + `\n\n**Parameters:** \`${defaultValues}\``
          );
          fullCompletion.sortText = `1_${name}`; // Sort after the tag version
          completions.push(fullCompletion);

        } else {
          // Regular completion for components with required fields
          const completion = new vscode.CompletionItem(name, vscode.CompletionItemKind.Keyword);
          completion.detail = def.type;
          completion.documentation = new vscode.MarkdownString(def.description);

          // Build default value string
          if (def.fields.length > 0) {
            const defaultValues = def.fields.map(f => {
              if (f.default !== undefined) {
                return f.type === 'string' ? `"${f.default}"` : f.default;
              }
              return f.type === 'number' ? '0' : f.type === 'boolean' ? 'false' : '""';
            }).join(' ');

            completion.insertText = `${name} ${defaultValues}`;
            completion.documentation.appendMarkdown(
              `\n\n**Default values:** \`${defaultValues}\``
            );
          } else {
            completion.insertText = name;
            completion.documentation.appendMarkdown('\n\n*No parameters required*');
          }

          completions.push(completion);
        }
      }

      return completions;
    }
  });

  context.subscriptions.push(hoverProvider);
  context.subscriptions.push(completionProvider);
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
