#include "backends/graphics_api.h"
#include "main.h"

#include "../ogl_includes.h"
#include <GL/gl.h>
#include <stdbool.h>
#include <sys/types.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// use 0 sentinel for unused textures.
TextureHandle textures[NUM_TEXTURES] = {0};

// just for keeping the same texture bound in the g_load_texture() function.
static TextureHandle curr_bound_texture = 0;

// returns an index into the global textures array.
uint g_load_texture(const char *filepath) {
  int width, height, channels;

  // flip BEFORE we load the first image.
  stbi_set_flip_vertically_on_load(true);

  unsigned char *image_data =
      stbi_load(filepath, &width, &height, &channels, 0);
  if (!image_data) {
    printf("Error loading the image: %s\n", stbi_failure_reason());
    return 0;
  }

  uint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set the texture wrapping/filtering options (modify as needed)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load the image data into the texture
  GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free the image data after it has been loaded into the texture
  stbi_image_free(image_data);

  for (int i = 0; i < NUM_TEXTURES; i++) {
    TextureHandle *t = &textures[i];
    if (*t == 0) {
      *t = textureID;
      glBindTexture(
          GL_TEXTURE_2D,
          curr_bound_texture); // rebind the old texture we were using.
      return i;
    }
  }

  fprintf(stderr, "Too many textures in the textures array.");
  exit(1);
  return 0;
}

uint g_load_cubemap(char *faces[6]) {
  uint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  stbi_set_flip_vertically_on_load(false);

  for (GLuint i = 0; i < 6; i++) {
    int width, height, channels;

    unsigned char *image_data =
        stbi_load(faces[i], &width, &height, &channels, 0);

    if (!image_data) {
      printf("Error loading the image at path \"%s\": %s\n", faces[i],
             stbi_failure_reason());
      return 0;
    }

    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;

    // load the face in by the offset.
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height,
                 0, format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

void g_use_texture(TextureHandle handle, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, handle);
  curr_bound_texture = handle;
}

void g_use_cubemap(TextureHandle handle, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
  curr_bound_texture = handle;
}
