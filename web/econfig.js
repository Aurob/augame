



//textureGroupPart roomtiles tile88 
// id 18 test texTest position 0 0 0 shape 1 1 1 color 1 1 1 renderPriority 3 texture roomtiles 0.5 1.0 0.5 0 .1 .1
// id 0 test Room position 0 10 -1 shape 15 15 1 color 0.5 0.5 0.5 1.0 renderPriority 1 collidable interior
// id 1 test Door position 2 25 0.1 shape 1 0.1 1 color 0.6 0.3 0.1 renderPriority 2 collidable interiorPortal 0 -1 inside 0 associated 0
// id 2 test Wall position 0 10 0.5 shape 15 1 2 color 0.1 0.2 0.4 1.0 renderPriority 1 collidable inside 0
// id 3 test Wall position 5 12 0.5 shape 1 6 2 color 0.05 0.1 0.2 1.0 renderPriority 1 collidable inside 0
// id 4 test Wall position 0 24 0.5 shape 15 1 2 color 0.1 0.2 0.4 1.0 renderPriority 1 inside 0
// id 5 test Other position 0 12 2.1 shape 1 1 1 color 0.1 0.2 0.3 1.0 renderPriority 1 collidable inside 0
// id 6 test Room position 9 10 0.2 shape 6 6 0 color 0.15 0.15 0.15 1.0 renderPriority 1 collidable interior inside 0
// id 7 test Door position 12 16 0 shape 1 0.1 1 color 0.6 0.3 0.1 renderPriority 2 collidable interiorPortal 6 0 inside 0 texture door
// id 8 test Wall position 9 15 0.1 shape 6 1 2 color 0.1 0.2 0.4 1.0 renderPriority 1 collidable inside 0 true
// id 9 test Other position -17 12 0 shape 1 1 0.1 color 0.1 0.3 0.8 renderPriority 0 collidable moveable movement 1 10 0 0 0 0 1 1 0.1 hoverable interactable
// id 10 test Table position 12 11 0.1 shape 1.5 1.5 1 color 0.5 0.25 0.1 1.0 renderPriority 1 collidable texture table 1.0 1.0 1 1 11 11 inside 6
// id 11 test Chair1 position 13 13 -0.1 shape 0.75 0.75 0.5 color 0.7 0.4 0.2 renderPriority 0 texture chair1 1.0 1.0 0 0 1 1 inside 6
// id 12 test Chair2 position 14 12 -0.1 shape 0.75 0.75 0.5 color 0.7 0.4 0.2 renderPriority 0 texture chair1 1.0 1.0 0 0 1 1 inside 6
// id 14 test Bed position 9 14 0.1 shape 2 2 1 color 0.4 0.3 0.2 1.0 renderPriority 1 texture bed1 1.0 1.0 0 0 1 1 inside 6
// id 13 test Shelf position 9 8 0.2 shape 2 2 2 color 0.3 0.2 0.1 renderPriority 2 texture shelf1 1.0 1.0 0 0 1 1 inside 6
// id 14 test Wall position 8 6 0 shape 3 3 3 color 0.15 0.15 0.15 1.0 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 15 test Wall position 11 6 0 shape 3 3 3 color 0.15 0.15 0.15 1.0 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 16 test Wall positbion 14 6 0 shape 3 3 3 color 0.15 0.15 0.15 1.0 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 15 test Wall position 8 9 0 shape 1 3 3 color 0.15 0.15 0.15 renderPriority 2 collidable inside 6 texture wall2 1.0 2.0 0 0 1 1
// id 17 test Wall position 8 12 0 shape 1 3 3 color 0.15 0.15 0.15 renderPriority 2 collidable inside 6 texture wall2 1.0 2.0 0 0.5 1 1
// `;

// id 2 test test1 position 0 0 0 shape 15 4 0 color 0.1 0.2 0.3 1.0 renderPriority 0 collidable
// id 3 test test2 position 0 -.1 0 shape 15 .1 4 color 0.1 0.3 0.4 1.0 renderPriority 2 collidable
// id 2 test test1 position 0 -8 4 shape 15 4 1 color 0.1 0.2 0.4 1.0 renderPriority 3 collidable

var configstr = `
    id 1 room position 0 -10 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority -1 collidable interior
    id 0 player position 4.5 -1 0 shape 1 1 1 color 0.1 0.2 0.3 .3 movement 75 1000 0 0 0 0 10 1 0 moveable collidable renderPriority 0 inside 1
    id 3 slime1 position 4.5 -3 0 shape .5 .125 .25 color 0.4 0.2 0.3 1.0 texture slime 0 0 0.047 0.11 0.031 0.038 ui hello 1 0 -200 renderPriority -2 inside 1 draggable 1 hoverable interactable
    id 3 trumpet position 9 -8 0 shape 1 1 0 color 0.4 0.2 0.3 1.0 renderPriority 3 inside 1 interactable hoverable draggable 1 tone R 4n -25 fsharp.wav texture trumpet

    id 4 chair position 6 -6 0 shape 1 1 1 color 0.4 0.2 0.3 1.0 renderPriority 3 inside 1 textureGroupPart interiors s590
    id 4 chair position 6 -6 -1 shape 1 1 1 color 0.4 0.2 0.3 1.0 renderPriority -1 inside 1 textureGroupPart interiors s606

    id 6 piano position 9 -5 0 shape 1 1 1 color 0.1 0.2 0.3 1.0 renderPriority 0 collidable inside 1 hoverable interactable texture props 0 0 0.577 0.043 0.0113 0.035
    id 7 pianokey position 9.2 -5.4 1 shape .325 0.15 1 color 1 1 1 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone C4 4n -25
    id 7 pianokey position 9.2 -5.25 1 shape .1625 0.15 1 color 1 1 1 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone D4 4n -25
    id 7 pianokey position 9.3625 -5.25 1 shape .1625 0.15 1 color 0 0 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone Eb4 4n -25
    id 7 pianokey position 9.2 -5.1 1 shape .325 0.15 1 color 1 1 1 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone E4 4n -25
    id 7 pianokey position 9.2 -4.95 1 shape .1625 0.15 1 color 1 1 1 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone F4 4n -25
    id 7 pianokey position 9.3625 -4.95 1 shape .1625 0.15 1 color 0 0 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable tone Gb4 4n -25
    id 7 pianokey position 9.2 -4.8 1 shape .325 0.15 1 color 1 1 1 1.0 renderPriority -1 collidable inside 1 hoverable interactable tone G4 4n -25
    id 7 pianokey position 9.2 -4.65 1 shape .1625 0.15 0 color 1 1 1 1.0 renderPriority -1 collidable inside 1 hoverable interactable tone A4 4n -25
    id 7 pianokey position 9.3625 -4.65 1 shape .1625 0.15 0 color 0 0 0 1.0 renderPriority -1 collidable inside 1 hoverable interactable tone Bb4 4n -25
    id 7 pianokey position 9.2 -4.5 1 shape .325 0.15 0 color 1 1 1 1.0 renderPriority -1 collidable inside 1 hoverable interactable tone B4 4n -25

    id 9 test1 position 4.5 -10 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s215 inside 1
    id 9 test1 position 3.5 -9 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s240 inside 1
    id 9 test1 position 4.5 -7 1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 3 textureGroupPart tileset1 s241 collidable inside 1
    id 9 test1 position 5.5 -9 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s242 inside 1
    id 9 test1 position 4.5 -8 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s267 inside 1
    id 2 room2 position 0 -10 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority 0 collidable interior
    id 10 door position 4.5 -7 0 shape 1 0.4 1 color 0.0 0.0 0.0 1.0 renderPriority 2 collidable interiorPortal 1 2 inside 1 associated 0

    id 3 room3 position 0 -10 0 shape 10 10 0 color 0.4 0.4 0.4 1.0 renderPriority 0 collidable interior
    id 11 door position 4.5 -4 0 shape 1 0.4 1 color 0.0 0.0 0.0 1.0 renderPriority 0 collidable interiorPortal 2 3 inside 2 associated 0
    id 9 test1 position 4.5 -6 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 0 textureGroupPart tileset1 s215 inside 2
    id 9 test1 position 3.5 -5 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 0 textureGroupPart tileset1 s240 inside 2
    id 9 test1 position 4.5 -3 1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 0 textureGroupPart tileset1 s241 collidable inside 2
    id 9 test1 position 5.5 -5 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 0 textureGroupPart tileset1 s242 inside 2
    id 9 test1 position 4.5 -4 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 0 textureGroupPart tileset1 s267 inside 2

    id 4 room4 position 0 -10 0 shape 10 10 0 color 0.3 0.3 0.3 1.0 renderPriority 0 collidable interior
    id 12 door position 4.5 -1 0 shape 1 0.4 1 color 0.0 0.0 0.0 1.0 renderPriority 2 collidable interiorPortal 3 4 inside 3 associated 0
    id 9 test1 position 4.5 -3 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s215 inside 3
    id 9 test1 position 3.5 -2 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s240 inside 3
    id 9 test1 position 4.5 0 1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority 3 textureGroupPart tileset1 s241 collidable inside 3
    id 9 test1 position 5.5 -2 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s242 inside 3
    id 9 test1 position 4.5 -1 -1 shape 1 1 1 color 0.1 0.3 0.8 1.0 renderPriority -1 textureGroupPart tileset1 s267 inside 3
    
    id 5 room5 position 0 -10 0 shape 10 10 0 color 0.2 0.2 0.2 1.0 renderPriority 0 collidable interior
`.split("\n").filter(line => line.length > 0);


function addToneEntity(id, position, shape, tone) {
    const [posX, posY, posZ] = position;
    const [shapeX, shapeY, shapeZ] = shape;
    const [note, duration, volume] = tone;

    const entityString = `
        id ${id} toneobj position ${posX} ${posY} ${posZ} shape ${shapeX} ${shapeY} ${shapeZ} color 0.1 0.2 0.3 1.0 collidable renderPriority 4 inside 1 hoverable interactable tone ${note} ${duration} ${volume}
    `.trim();

    configstr.push(entityString);
}

const baseX = 4.5;
const baseY = -5;
const notes = ["C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5", "D5", "E5", "F5", "G5", "A5", "B5", "C6"];
const durations = ["8n", "4n", "2n", "1n", "16n", "32n"];
const volumes = [-20, -18, -16, -14, -12, -10, -8, -6, -4, -2, -12];

function getRandomElement(arr) {
    return arr[Math.floor(Math.random() * arr.length)];
}

// for (let i = 0; i < 10; i++) {
//     for (let j = 10; j >= 0; j--) {
//         const id = 13 + i * 10 + j;
//         const posX = baseX + j * 0.12;
//         const posY = baseY + i * 0.12;
//         const posZ = 1/2 * i;
//         const note = getRandomElement(notes);
//         const duration = .1; //getRandomElement(durations);
//         const volume = getRandomElement(volumes);
//         addToneEntity(id, [posX, posY, posZ], [0.1, 0.1, 0], [note, duration, volume]);
//     }
// }

// Example usage:
// addToneEntity(28, [4.5, -5.8, 0], [0.25, 0.25, 0], ["C4", "8n", -20]);


// id 13 toneobj position 4.6 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone D4 8n -20
// id 14 toneobj position 4.7 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone E4 8n -20
// id 15 toneobj position 4.8 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone F4 8n -20
// id 16 toneobj position 4.9 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone G4 8n -20
// id 17 toneobj position 5.0 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone A4 8n -20
// id 18 toneobj position 5.1 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone B4 8n -20
// id 19 toneobj position 5.2 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone C5 8n -20
// id 20 toneobj position 5.3 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone D5 8n -20
// id 21 toneobj position 5.4 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone E5 8n -20
// id 22 toneobj position 5.5 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone F5 8n -20
// id 23 toneobj position 5.6 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone G5 8n -20
// id 24 toneobj position 5.7 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone A5 8n -20
// id 25 toneobj position 5.8 -5 0 shape .1 .5 .1 color 0.9 0.9 0.9 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone B5 8n -20
// id 26 toneobj position 5.9 -5 0 shape .1 .5 .1 color 0.1 0.2 0.3 1.0 renderPriority 4 collidable inside 1 hoverable interactable tone C6 8n -20

ECONFIG = {
    "Entities": configstr.map(input => new EntityBuilder().parseInput(input).build())
};
