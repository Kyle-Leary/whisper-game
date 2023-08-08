CFLAGS +=  -c -Wall

INCLUDES :=  -I. -Isrc 

# ignore the whole backend folder, and only add in the backend subfolder.
ALL_BACKEND := src/backends

# The final target (change this to your desired target name)
TARGET:=walk
