## TODO
 - ~~If there is no Player entity then an error occurs~~
 - generalize the screen calculation stuff
 - Only update shapes if a zoom update occurred
 - ~~Add text/ui component~~
 
 ## Entity Configuration Changes
 - Rename the Id component to metadata
 - Use the name value in metadata as an entity lookup in EntityConfig.hpp
  - Check if the metadata component was included in the config json
   - check if there already exists an entity with Metadata with a name that matches
    - If there is, remove that entity and recreate it?
     - Or just update it?
   - also don't allow multiple entities with the same name?
    - this might be emergent 

- Player texture is currently hardcoded, make it component based

- Remove the Vector2f and Vector3f structs and instead use the physics vec2/vec3

- Need to be able to make interactable/draggable entities ignore collisions while dragging
 - but also allow collisions while dragging when needed

 - Interiors don't collide with entities that are not inside it

- currently if an entity is inside another it need to be defined after the Interior entity in the config
 - this is not ideal, should be able to define them in any order

- Right now only simple rectangle shapes are supported
 - need to support more circles as well, since the physics lib supports circle collisions
 - Also wnat to be able to make shapes like polygons
   - but maybe a simpler thing to do would be to allow grouping of multiple shapes into a single entity

- Text scale doesn't do anything

- Scenebuilder upgrades:
  - add start menu and pause menu editor
  - add Interactable and Hoverable
  - add select group for moving and blueprinting
  - add zoom
  - add option to hide labels on canvas
    - or just specific labels, like id
  - force unique ids, duplicate id.names are allowed
  - add show/hide buttons for all UI
  - make grid snap global and not on edit panel

*- Add a "winning" finish state
 - like a pause menu where clicking takes you back to the main menu
 - track stats for each run and display on the start menu
- add winning state, ie. anything greater than 1, since 1 is default playstate
  - doesn't have to be 2, there can be multiple increments of state

- add client side network connection
 - add an npc with access to an llm api

- "equip" items

- story idea
  - inspired by later levels in Halo 1
    - gameplay familiarity is contrasted by the "flood"
      - "flood" is hypersensitive
    - leave behind debris to simulate prior events
    - scary waves of danger
      - but ample ammo to pick up
    - minimap showing entity location
    - entities that "die", but can also "reanimate"
    - elevators
    - omnipresent entity that can control player 

~~- make locked doors "red" and unlocked "green"~~
- ~~don't allow interactions in rooms the player isn't in~~
  - ~~dragging an entity through a door maintains that interaction~~
  - issue now is that dragged entities still trigger collision checks in other rooms
    - i.e locked doors with keys, the keys can be dragged over the position of the door unlocking it, even if in a different room


- Add scene browser
- Add Scene component and use like Inside
  - specify that entities are in certain Scenes and should utilize a specific registry
  - swap the registry when changing scenes
    - but would have to get rid of basically all globals not pertaining to the world size/spacing
- Add a way to change the scene at runtime
  - if this works then add the scenebuilder on the same page as the game
    - try and get realtime map editing to work