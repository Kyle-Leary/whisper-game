#pragma once

// heavily abusing the macro concat in C here.

#define ASSET_PATH "assets/"

// concat the macro, better than making functions that
// sprintf and malloc the path at runtime. faster and easier.
#define TEXTURE_PATH(base_path) ASSET_PATH "textures/" base_path
#define SOUND_PATH(base_path) ASSET_PATH "sounds/" base_path
// don't need to use the .pp at the end. is this too magical??
#define SHADER_PATH(base_path) ASSET_PATH "shaders/" base_path ".pp"
#define MODEL_PATH(base_path) ASSET_PATH "models/" base_path

// general purpose resources, like random area file-formats that we'll parse and
// use on the fly.
#define RESOURCE_PATH ASSET_PATH "resources/"

#define CONFIG_PATH(base_path) ASSET_PATH "config/" base_path

#define AREA_PATH(base_path) "src/areas/instances/" base_path
