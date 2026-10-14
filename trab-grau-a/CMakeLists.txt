cmake_minimum_required(VERSION 3.16)
project(VooRasante CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

# ---------------------------------------------------------------------------
# Dependencias
# ---------------------------------------------------------------------------
# A stb_image esta vendorizada em vendor/, entao o projeto nao depende de rede
# pra ela. Apenas GLFW e GLM sao baixadas.
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

if(WIN32)
    set(OPENGL_LIBS opengl32)
elseif(APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    set(OPENGL_LIBS ${OpenGL_LIBRARY})
else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIBS ${OPENGL_gl_LIBRARY})
endif()

add_compile_options(-Wno-pragmas)

# ---------------------------------------------------------------------------
# ALVO 1: game_core -- as REGRAS DO JOGO
# ---------------------------------------------------------------------------
# Este alvo NAO tem a GLAD nem a GLFW no include path. Isso e de proposito:
# escrever #include <glad/glad.h> em qualquer arquivo de src/game/ vira erro
# de compilacao, e nao uma questao de disciplina. A unica dependencia e a GLM,
# que e matematica pura e nao fala com a placa de video.
add_library(game_core STATIC
    src/core/config.cpp
    src/core/sprites.cpp
    src/game/tilemap.cpp
    src/game/entities.cpp
    src/game/player.cpp
    src/game/weapons.cpp
    src/game/enemies.cpp
    src/game/collision.cpp
    src/game/world.cpp
)
target_include_directories(game_core PUBLIC src)
target_link_libraries(game_core PUBLIC glm::glm)

# ---------------------------------------------------------------------------
# ALVO 2: voo_rasante -- RENDERIZACAO, JANELA e main
# ---------------------------------------------------------------------------
add_executable(voo_rasante
    src/main.cpp
    src/stb_impl.cpp
    vendor/glad.c
    src/render/shader.cpp
    src/render/atlas.cpp
    src/render/renderer.cpp
    src/render/layers.cpp
    src/render/hud.cpp
    src/render/radar.cpp
    src/platform/window.cpp
    src/platform/input.cpp
)
target_include_directories(voo_rasante PRIVATE src vendor vendor/glad)
target_link_libraries(voo_rasante PRIVATE game_core glfw ${OPENGL_LIBS})

# Caminho do projeto gravado em tempo de configuracao. Com isso os assets e os
# shaders sao encontrados independente do diretorio de onde o jogo foi aberto,
# seja pelo terminal, pelo VS Code ou pelo Finder.
target_compile_definitions(voo_rasante PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

message(STATUS "Executavel: voo_rasante (raiz do projeto: ${CMAKE_CURRENT_SOURCE_DIR})")

# ---------------------------------------------------------------------------
# ALVO 3: simtest -- prova pratica da separacao
# ---------------------------------------------------------------------------
# Roda a simulacao do jogo SEM abrir janela e sem linkar OpenGL nenhum.
# Isto so e possivel porque toda a regra do jogo esta em game_core. Serve de
# teste automatizado e e a demonstracao mais direta de que a fronteira entre
# gameplay e renderizacao e real, e nao apenas uma convencao de pastas.
add_executable(simtest src/simtest.cpp)
target_link_libraries(simtest PRIVATE game_core)
