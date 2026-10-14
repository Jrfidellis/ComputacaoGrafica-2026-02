// Unica unidade de traducao que gera o codigo da stb. Nao tem mais nada aqui
// de proposito: se estes defines aparecerem num header, cada .cpp que o
// incluir gera uma copia das funcoes e o linker acusa simbolo duplicado.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
