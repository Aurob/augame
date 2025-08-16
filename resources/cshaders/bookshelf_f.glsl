precision mediump float;

varying vec2 vPosition;
uniform float uSeed; // Use a uniform seed for more randomness

// Enhanced random function using seed
float random(vec2 st, float seed) {
    return fract(sin(dot(st.xy + seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    // Normalize position to [0,1]
    vec2 uv = (vPosition + 1.0) * 0.5;
    
    // Background color (dark wood)
    vec3 backgroundColor = vec3(0.3, 0.2, 0.1);
    
    // Number of shelves and books per row
    float numShelves = 6.0;
    float booksPerShelf = 300.0;
    
    // Calculate shelf and book position
    float shelfHeight = 1.0 / numShelves;
    float bookWidth = 1.0 / booksPerShelf;
    
    // Which shelf are we on?
    float shelfIndex = floor(uv.y / shelfHeight);
    float yInShelf = mod(uv.y, shelfHeight);
    
    // Which book are we on?
    float bookIndex = floor(uv.x / bookWidth);
    float xInBook = mod(uv.x, bookWidth);
    
    // Start with background color
    vec3 color = backgroundColor;
    float alpha = 1.0; // Default alpha value for opaque rendering
    
    // Draw horizontal shelf lines (bottom 10% of each shelf)
    if (yInShelf < shelfHeight * 0.1) {
        // Shelf color (lighter wood)
        color = vec3(0.433, 0.21, 0.29);
    }
    // Top shelf is the top of the bookshelf
    else if (shelfIndex == numShelves - 1.0) {
        color = backgroundColor;
    }
    // Bottom shelf (thinner than top)
    else if (shelfIndex == 0.0) {
        if (yInShelf < shelfHeight * 0.5) {
            // Render a solid rectangle at the very bottom of the bottom shelf
            color = backgroundColor;
        } else if (yInShelf < shelfHeight * 0.967) {
            // Shorten the books on the bottom shelf
            vec2 bookSeed = vec2(bookIndex, shelfIndex);
            float drawBook = random(bookSeed, uSeed);
            
            if (drawBook > 0.2) { // 80% chance to draw a book
                // Randomize book width and height
                float bookWidthVariation = random(bookSeed + vec2(2.0, 2.0), uSeed) * 0.8 + 0.2;
                float bookHeightVariation = random(bookSeed + vec2(3.0, 3.0), uSeed) * 0.4 + 0.1; // Shorter books
                
                // Adjust xInBook and yInShelf for book size variation
                xInBook /= bookWidthVariation;
                yInShelf /= bookHeightVariation;
                
                // Basic palette of book colors found in a library
                float colorChoice = random(bookSeed, uSeed);
                vec3 bookColor;
                
                if (colorChoice < 0.2) {
                    bookColor = vec3(0.6, 0.3, 0.2); // Brown
                } else if (colorChoice < 0.4) {
                    bookColor = vec3(0.8, 0.5, 0.4); // Light Brown
                } else if (colorChoice < 0.6) {
                    bookColor = vec3(0.4, 0.4, 0.4); // Gray
                } else if (colorChoice < 0.8) {
                    bookColor = vec3(0.7, 0.7, 0.5); // Beige
                } else {
                    bookColor = vec3(0.5, 0.2, 0.2); // Dark Red
                }
                
                color = bookColor;
                
                // Add book spine lines (vertical separators) with some randomness
                if (random(bookSeed + vec2(4.0, 4.0), uSeed) > 0.5) {
                    if (xInBook < bookWidth * 0.05 || xInBook > bookWidth * 0.95) {
                        color *= 0.7; // Darken the edges for book separation
                    }
                }
                
                // Add some horizontal lines on book spines for detail
                float spineLines = sin(uv.y * 60.0 + bookIndex * 3.0);
                if (spineLines > 0.8) {
                    color *= 0.9;
                }
            } else {
                // Render transparent for empty spaces
                alpha = 0.0;
            }
        } else {
            color = vec3(0.433, 0.21, 0.29);
        }
    }
    // Left and right sides of the bookshelf (very thin)
    else if (uv.x < bookWidth * 0.5 || uv.x > 1.0 - bookWidth * 0.5) {
        color = vec3(0.433, 0.21, 0.29);
    }
    // Draw books (top 90% of each shelf)
    else {
        // Randomly decide whether to draw a book or not
        vec2 bookSeed = vec2(bookIndex, shelfIndex);
        float drawBook = random(bookSeed, uSeed);
        
        if (drawBook > 0.2) { // 80% chance to draw a book
            // Randomize book width and height
            float bookWidthVariation = random(bookSeed + vec2(2.0, 2.0), uSeed) * 0.8 + 0.2;
            float bookHeightVariation = random(bookSeed + vec2(3.0, 3.0), uSeed) * 0.8 + 0.2;
            
            // Adjust xInBook and yInShelf for book size variation
            xInBook /= bookWidthVariation;
            yInShelf /= bookHeightVariation;
            
            // Basic palette of book colors found in a library
            float colorChoice = random(bookSeed, uSeed);
            vec3 bookColor;
            
            if (colorChoice < 0.2) {
                bookColor = vec3(0.6, 0.3, 0.2); // Brown
            } else if (colorChoice < 0.4) {
                bookColor = vec3(0.8, 0.5, 0.4); // Light Brown
            } else if (colorChoice < 0.6) {
                bookColor = vec3(0.4, 0.4, 0.4); // Gray
            } else if (colorChoice < 0.8) {
                bookColor = vec3(0.7, 0.7, 0.5); // Beige
            } else {
                bookColor = vec3(0.5, 0.2, 0.2); // Dark Red
            }
            
            color = bookColor;
            
            // Add book spine lines (vertical separators) with some randomness
            if (random(bookSeed + vec2(4.0, 4.0), uSeed) > 0.5) {
                if (xInBook < bookWidth * 0.05 || xInBook > bookWidth * 0.95) {
                    color *= 0.7; // Darken the edges for book separation
                }
            }
            
            // Add some horizontal lines on book spines for detail
            float spineLines = sin(uv.y * 60.0 + bookIndex * 3.0);
            if (spineLines > 0.8) {
                color *= 0.9;
            }
        } else {
            // Render transparent for empty spaces
            alpha = 0.0;
        }
    }
    
    gl_FragColor = vec4(color, alpha);
}