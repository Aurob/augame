
const configstr = `
id 9 test Other position 10 10 1 shape 1 1 1 color 0.1 0.3 0.8 renderPriority 4 textureGroupPart roomtiles tile78
id 9 test Other position 10 10 0 shape 1 1 1 color 0.1 0.3 0.8 renderPriority 0 collidable textureGroupPart roomtiles tile88`;

// id 18 test texTest position 0 0 0 shape 1 1 1 color 1 1 1 renderPriority 3 texture roomtiles 0.5 1.0 0.5 0 .1 .1
// id 0 test Room position 0 10 -1 shape 15 15 1 color 0.5 0.5 0.5 renderPriority 1 collidable interior
// id 1 test Door position 2 25 0.1 shape 1 0.1 1 color 0.6 0.3 0.1 renderPriority 2 collidable interiorPortal 0 -1 inside 0 associated 0
// id 2 test Wall position 0 10 0.5 shape 15 1 2 color 0.1 0.2 0.4 renderPriority 1 collidable inside 0
// id 3 test Wall position 5 12 0.5 shape 1 6 2 color 0.05 0.1 0.2 renderPriority 1 collidable inside 0
// id 4 test Wall position 0 24 0.5 shape 15 1 2 color 0.1 0.2 0.4 renderPriority 1 inside 0
// id 5 test Other position 0 12 2.1 shape 1 1 1 color 0.1 0.2 0.3 renderPriority 1 collidable inside 0
// id 6 test Room position 9 10 0.2 shape 6 6 0 color 0.15 0.15 0.15 renderPriority 1 collidable interior inside 0
// id 7 test Door position 12 16 0 shape 1 0.1 1 color 0.6 0.3 0.1 renderPriority 2 collidable interiorPortal 6 0 inside 0 texture door
// id 8 test Wall position 9 15 0.1 shape 6 1 2 color 0.1 0.2 0.4 renderPriority 1 collidable inside 0 true
// id 9 test Other position -17 12 0 shape 1 1 0.1 color 0.1 0.3 0.8 renderPriority 0 collidable moveable movement 1 10 0 0 0 0 1 1 0.1 hoverable interactable
// id 10 test Table position 12 11 0.1 shape 1.5 1.5 1 color 0.5 0.25 0.1 renderPriority 1 collidable texture table 1.0 1.0 1 1 11 11 inside 6
// id 11 test Chair1 position 13 13 -0.1 shape 0.75 0.75 0.5 color 0.7 0.4 0.2 renderPriority 0 texture chair1 1.0 1.0 0 0 1 1 inside 6
// id 12 test Chair2 position 14 12 -0.1 shape 0.75 0.75 0.5 color 0.7 0.4 0.2 renderPriority 0 texture chair1 1.0 1.0 0 0 1 1 inside 6
// id 14 test Bed position 9 14 0.1 shape 2 2 1 color 0.4 0.3 0.2 renderPriority 1 texture bed1 1.0 1.0 0 0 1 1 inside 6
// id 13 test Shelf position 9 8 0.2 shape 2 2 2 color 0.3 0.2 0.1 renderPriority 2 texture shelf1 1.0 1.0 0 0 1 1 inside 6
// id 14 test Wall position 8 6 0 shape 3 3 3 color 0.15 0.15 0.15 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 15 test Wall position 11 6 0 shape 3 3 3 color 0.15 0.15 0.15 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 16 test Wall position 14 6 0 shape 3 3 3 color 0.15 0.15 0.15 renderPriority 1 collidable inside 6 texture wall1 1.0 2.0 0 0 1 1
// id 15 test Wall position 8 9 0 shape 1 3 3 color 0.15 0.15 0.15 renderPriority 2 collidable inside 6 texture wall2 1.0 2.0 0 0 1 1
// id 17 test Wall position 8 12 0 shape 1 3 3 color 0.15 0.15 0.15 renderPriority 2 collidable inside 6 texture wall2 1.0 2.0 0 0.5 1 1
// `;

const inputs = configstr.split("\n");
ECONFIG = {
    "Entities": inputs.map(input => new EntityBuilder().parseInput(input).build())
};
