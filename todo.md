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