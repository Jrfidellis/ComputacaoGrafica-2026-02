# ComputacaoGrafica-2026-02

Repositório da disciplina de Processamento Gráfico (Unisinos, 2026/02).

## Lista 1 - Introdução à OpenGL Moderna

A entrega da Lista de Exercícios 1 (Primitivas Gráficas, Shaders & Buffers) está na pasta [PG2026-2/src/Lista1](PG2026-2/src/Lista1). Cada exercício é um arquivo `.cpp` independente com sua própria função `main`, e o CMake do projeto gera um executável para cada um.

| Exercício | Arquivos |
|-----------|----------|
| 1. Dois triângulos (preenchido, contorno, pontos e os três juntos) | `exercicio1a.cpp` a `exercicio1d.cpp` |
| 2. Círculo pela equação paramétrica e variações (octágono, pentágono, pac-man, fatia de pizza, estrela) | `exercicio2.cpp`, `exercicio2a.cpp` a `exercicio2e.cpp` |
| 3. Triângulo com cor por vértice (VBO intercalado, dois atributos no vertex shader) | `exercicio3.cpp` |

O enunciado completo está em [aulas/Lista1.pdf](aulas/Lista1.pdf). As instruções de configuração do ambiente e compilação estão em [PG2026-2/README.md](PG2026-2/README.md) e [PG2026-2/GettingStarted.md](PG2026-2/GettingStarted.md).

Para compilar e rodar um exercício, a partir da pasta `PG2026-2`:

```
cmake --build build --target exercicio3
./build/exercicio3
```
