
var configstr = `
    id 1 room position 0.5 -10 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority -1 collidable interior textureGroupPart room1 s115 10 10
    id 11 roomwall position 0.5 -12.5 0 shape 10 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 1 textureGroupPart room1 s227 10 2
    id 11 roomwall2 position 0.5 -1.5 1 shape 10 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 1 textureGroupPart room1 s227 10 2

    id 2 room2 position 3.5 -25 0 shape 4 15 0 color 0.5 0.5 0.5 1.0 renderPriority 0 collidable interior textureGroupPart room1 s220 4 15
    id 12 roomwall2 position 3.5 -11.6 .7 shape 4 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 2 textureGroupPart room1 s295 4 2
    id 13 roomwall2 position 3.5 -26.8 .7 shape 4 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 2 textureGroupPart room1 s295 4 2
    
    id 3 room3 position 3.5 -28.8 0 shape 4 4 0 color 0.5 0.5 0.5 1.0 renderPriority 0 collidable interior textureGroupPart room1 s115 4 4
    id 4 room4 position 0.5 0 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority 0 collidable interior textureGroupPart room1 s115 10 10
    id 5 room5 position 0.5 7 0 shape 10 10 0 color 0.5 0.5 0.5 1.0 renderPriority 0 collidable interior textureGroupPart room1 s183 10 10

    id 10 doorC position 4.725 -10.8 0.1 shape 1.5 1 1 color 0.0 0.0 0.0 1.0 renderPriority 1 collidable interiorPortal 1 2 0 inside 1 texture doors 0 0 0 0 1 0.5
    id 11 doorB position 4.725 -25.8 0.2 shape 1.5 1 1 color 0.0 0.0 0.0 1.0 renderPriority 1 collidable interiorPortal 2 3 0 inside 2 texture doors 0 0 0 0 1 0.5
    id 12 doorA position 4.725 -.8 0.1 shape 1.5 1 1 color 0.0 0.0 0.0 1.0 renderPriority 1 collidable interiorPortal 1 4 1 inside 3 texture doors 0 0 0 0 1 0.5

    id 0 player position 5 -3 0 shape 1 1 1 color 0.1 0.2 0.3 .3 movement 75 1000 0 0 0 0 10 1 0 moveable collidable renderPriority 0 inside 1
    
    id 55 room4ladder position 5 6 0.1 shape 1 1 0 color 0.1 0.2 0.3 1.0 renderPriority 0 collidable inside 4 texture ladders 0 0 0 0 1 .244 interiorPortal 4 5 0 inside 4
    id 56 room5ladder position 5 6 1 shape 1 4 0 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 5 texture ladders 0 0 0 .244 1 1 inside 5

    id 11 piano2 position 4.5 -29 0 shape 2 2 1 color 0.1 0.2 0.3 1.0 renderPriority 0 inside 3 hoverable interactable texture instruments 0 0 0.6875 0.1764 0.125 0.1875
    id 12 piano2col position 4.5 -28.15 0 shape 2 .25 1 color 0.1 0.2 0.3 0.0 renderPriority 0 collidable inside 3

    id 13 p2key2 position 4.875 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone C4 4n -25
    
    id 14 p2key position 4.937 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone D4 4n -25
    id 14 p2key position 4.937 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable 
    
    id 15 p2key2 position 4.999 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone E4 4n -25
    
    id 16 p2key position 5.061 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 17 p2key position 5.061 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone F4 4n -25
    
    id 18 p2key2 position 5.123 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone G4 4n -25
    
    id 19 p2key position 5.185 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 20 p2key position 5.185 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone A4 4n -25
    
    id 21 p2key2 position 5.247 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone B4 4n -25

    id 22 p2key position 5.309 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 23 p2key position 5.309 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone C5 4n -25

    id 24 p2key2 position 5.371 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone D5 4n -25

    id 25 p2key position 5.433 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 26 p2key position 5.433 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone E5 4n -25

    id 27 p2key2 position 5.495 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone F5 4n -25

    id 28 p2key position 5.557 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 29 p2key position 5.557 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone G5 4n -25

    id 30 p2key2 position 5.619 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone A5 4n -25

    id 31 p2key position 5.681 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 32 p2key position 5.681 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone B5 4n -25

    id 33 p2key2 position 5.743 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone C6 4n -25

    id 34 p2key position 5.805 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 35 p2key position 5.805 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone D6 4n -25

    id 36 p2key2 position 5.867 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone E6 4n -25

    id 37 p2key position 5.929 -28.235 0 shape .06 .115 1 color 0 0 0 0.8 renderPriority 0 inside 3 hoverable interactable
    id 38 p2key position 5.929 -28.115 0 shape .06 .115 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone F6 4n -25

    id 39 p2key2 position 5.991 -28.23 0 shape .06 .23 1 color 1 1 1 0.8 renderPriority 0 inside 3 hoverable interactable tone G6 4n -25


    id 33 rgb1 position 4.65 -11.9 2 shape .25 .25 .25 color 1 0 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable
    id 34 rgb2 position 5 -11.9 2 shape .25 .25 .25 color 0 0 1 1.0 renderPriority 2 collidable inside 1 hoverable interactable
    id 35 rgb3 position 5.35 -11.9 2 shape .25 .25 .25 color 0 1 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable
    id 36 rgb4 position 5.65 -11.9 2 shape .25 .25 .25 color 1 1 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable
    id 37 rgb5 position 5.95 -11.9 2 shape .25 .25 .25 color 1 0.5 0 1.0 renderPriority 2 collidable inside 1 hoverable interactable

    id 101 explode1 position 5 12 0 shape .5 .5 .5 color 0.1 0.2 0.3 1 movement 75 1000 0 0 0 0 10 1 0 collidable renderPriority 0 inside 5 interactable hoverable draggable 10
    `.split("\n").filter(line => line.length > 0);

ECONFIG = {
    "Entities": configstr.map(input => new EntityBuilder().parseInput(input).build())
};
