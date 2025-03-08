
var configstr = `
id 1 room position 0.5 -5 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority -1 interior textureGroupPart room1 s115 10 10
id 11 roomwall position 0.5 -11 0 shape 10 2 1 color 0.1 0.2 0.3 1.0 renderPriority 2 inside 1 textureGroupPart room1 s227 10 2
id 11 roomwall5 position 0.5 -9.5 1 shape 10 .1 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 1 collidable
id 11 roomwall2 position 0.5 -1 1 shape 10 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 1 textureGroupPart room1 s227 10 2
id 11 roomwall25 position 0.5 0.5 1 shape 10 .1 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 1 collidable
id 103 l position 2.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s1 1 1 movement 75 1 0 collidable
id 111 d123 position 4.5 -4 0 shape 1 1 0 color 1 1 1 1 inside 1 movement 75 .1 .1 collidable
id 109 r position 7.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s18 1 1 movement 75 1 0 collidable

id 0 player position 5 -3 0 shape 1 1 0 color 0.1 0.2 0.3 1 movement 575 .5 0 collidable

id 121 door1 position 5 -10 0 shape 1 1 0 color 1 1 1 1 interiorPortal -1 1 0 inside 1 collidable
`
configstr = configstr.split("\n").filter(line => line.length > 0);

ECONFIG = {
    "Entities": configstr.map(input => new EntityBuilder().parseInput(input).build())
};



// id 104 l position 3.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s12 1 1 movement 75 1 0 collidable
// id 105 o position 4.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s15 1 1 movement 75 1 0 collidable
// id 107 w position 5.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s23 1 1 movement 75 1 0 collidable
// id 108 o position 6.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s15 1 1 movement 75 1 0 collidable
// id 109 r position 7.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s18 1 1 movement 75 1 0 collidable
// id 110 l position 8.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s12 1 1 movement 75 1 0 collidable
// id 111 d position 9.5 -6 0 shape 1 1 0 color 1 1 1 1 inside 1 textureGroupPart font s4 1 1 movement 75 1 0 collidable