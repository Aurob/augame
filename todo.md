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