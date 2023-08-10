CFLAGS += -Wall

INCLUDES +=  -I. -Isrc -Ideps/wjson/api -Ideps -Ideps/stb

# ignore the whole backend folder, and only add in the backend subfolder.
ALL_BACKEND := src/backends

# The final target (change this to your desired target name)
TARGET:=whisper
