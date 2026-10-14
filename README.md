# ComputacaoGrafica-2026-02

Repositório da disciplina de Processamento Gráfico (Unisinos, 2026/02).

Cada exercício é um `.cpp` independente com sua própria `main`, e o CMake gera um executável para cada um. O alvo leva o nome da pasta como prefixo e o binário vai para `build/<pasta>/`, para que arquivos de mesmo nome em listas diferentes não conflitem. A partir da pasta `PG2026-2`:

```
cmake -S . -B build                              # apenas na primeira vez
cmake --build build --target Lista1_exercicio3
./build/Lista1/exercicio3
```

No VS Code, abra o `.cpp` e use o botão ▷ (Run C/C++ File). A configuração do ambiente está em [PG2026-2/GettingStarted.md](PG2026-2/GettingStarted.md).

## Lista 1 - Primitivas Gráficas, Shaders & Buffers

Pasta [PG2026-2/src/Lista1](PG2026-2/src/Lista1).

| Exercício | Arquivos |
| --- | --- |
| 1. Dois triângulos (preenchido, contorno, pontos e os três juntos) | `exercicio1a.cpp` a `exercicio1d.cpp` |
| 2. Círculo paramétrico e variações (octágono, pentágono, pac-man, fatia de pizza, estrela) | `exercicio2.cpp`, `exercicio2a.cpp` a `exercicio2e.cpp` |
| 3. Triângulo com cor por vértice (VBO intercalado, dois atributos) | `exercicio3.cpp` |

## Lista 2 - Câmera 2D

Pasta [PG2026-2/src/Lista2](PG2026-2/src/Lista2).

| Exercício | Arquivo |
| --- | --- |
| 1. Projeção ortográfica com `ortho(-10, 10, -10, 10)` | `exercicio1.cpp` |
| 2. Câmera 2D em coordenadas de tela com `ortho(0, 800, 600, 0)` | `exercicio2.cpp` |
| 3. Resposta escrita sobre o posicionamento dos objetos | `exercicio3.md` |
| 4. Triângulo posicionado em um quadrante | `exercicio4.cpp` |
| 5. Um triângulo em cada quadrante, por função parametrizada | `exercicio5.cpp` |
| 6. Triângulo criado na posição do clique do mouse | `exercicio6.cpp` |

## Trabalho Grau A - Voo Rasante

**→ [Documentação completa do trabalho](trab-grau-a/README.md)**

Jogo 2D de avião visto de cima, na pasta [trab-grau-a](trab-grau-a). O avião
fica sempre no centro da tela, o mundo rola por baixo e um radar quadrado no
canto inferior esquerdo mostra o mapa inteiro.

O código é separado em duas camadas, e a fronteira é imposta pelo CMake: a
biblioteca `game_core` tem as regras do jogo e não enxerga OpenGL, então
incluir a GLAD em `src/game/` é erro de compilação. O alvo `simtest` roda a
simulação completa sem abrir janela.

```
cd trab-grau-a
cmake -S . -B build
cmake --build build -j4
./build/voo_rasante
```

### Vídeos

Gravações em [trab-grau-a/apresentacao](trab-grau-a/apresentacao):
[demo de gameplay](trab-grau-a/apresentacao/demo.mov) ·
[estrutura do código](trab-grau-a/apresentacao/estrutura.mov) ·
[VAO e matriz de model](trab-grau-a/apresentacao/vao.mov) ·
[sprites e animação](trab-grau-a/apresentacao/sprite.mov) ·
[camadas e parallax](trab-grau-a/apresentacao/parallax.mov)

Arquitetura, decisões de implementação e checklist de requisitos em
**[trab-grau-a/README.md](trab-grau-a/README.md)**.
