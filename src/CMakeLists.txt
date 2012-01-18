
add_library(nbt nbt.c endianness.c)
target_link_libraries(nbt z)

add_executable(kouzan chunk.c main.c utils.c)
target_link_libraries(kouzan nbt ${GLUT_LIBRARY} ${OPENGL_LIBRARY})
