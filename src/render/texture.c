#include "texture.h"
#include "cglm/types.h"
#include "defines.h"
#include "main.h"

#include "macros.h"

#include "../ogl_includes.h"
#include "path.h"
#include <GL/gl.h>
#include <stdbool.h>
#include <sys/types.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// just for keeping the same texture bound in the g_load_texture() function.
static uint curr_bound_texture = 0;

uint transparent_tex;
uint black_tex;
uint white_tex;
uint nepeta_tex;

// i think that we can call this function, then call modifiers after and still
// modify the image texture properties properly. so, we don't need any function
// pointer bullshit of "what params should i set on this texture?"
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

  uint textureID =
      g_load_texture_from_buf(image_data, width, height, channels, 0);

  // Free the image data after it has been loaded into the texture
  stbi_image_free(image_data);

  return textureID;
}

uint g_load_texture_from_png_buf(byte *png_buf, int len) {
  int width, height, channels;

  stbi_set_flip_vertically_on_load(true);

  unsigned char *image_data =
      stbi_load_from_memory(png_buf, len, &width, &height, &channels, 0);
  if (!image_data) {
    printf("Error loading the image: %s\n", stbi_failure_reason());
    return 0;
  }

  uint textureID =
      g_load_texture_from_buf(image_data, width, height, channels, 0);

  // Free the image data after it has been loaded into the texture
  stbi_image_free(image_data);

  return textureID;
}

uint g_load_texture_from_buf(byte *img_buf, int width, int height, int channels,
                             int stride) {
  uint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set the texture wrapping/filtering options (modify as needed)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLenum internal_format;
  GLenum format;
  // Load the image data into the texture
  switch (channels) {
  case 1: {
    internal_format = GL_R8;
    format = GL_RED;
  } break;
  case 3: {
    format = GL_RGB;
    internal_format = format;
  } break;
  case 4: {
    format = GL_RGBA;
    internal_format = format;
  } break;
  default: {
    ERROR_NO_ARGS("Invalid channels passed to function, could not find a "
                  "corresponding opengl tex type enum.");
    return 0;
  } break;
  }

  glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
               GL_UNSIGNED_BYTE, img_buf);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Reset row length to zero (the default)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  return textureID;
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

void init_helper_textures() {
  transparent_tex =
      g_load_texture_from_buf((byte *)(vec4){0, 0, 0, 0}, 1, 1, 4, 0);
  black_tex = g_load_texture_from_buf((byte *)(vec4){0, 0, 0, 1}, 1, 1, 4, 0);
  white_tex = g_load_texture_from_buf((byte *)(vec4){1, 1, 1, 1}, 1, 1, 4, 0);
  nepeta_tex = g_load_texture(TEXTURE_PATH("nepeta.jpg"));
}
