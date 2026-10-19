# Algoritmo de Kruskal: Árvore Geradora Mínima

Trabalho de **Análise e Projeto de Algoritmos** (Unisinos, 2026/2): implementação do algoritmo guloso de Kruskal em C++17, sem bibliotecas que resolvam a AGM.

Equipe: Felipe Flores e Leandro Berwanger · Profa. Maria Adelina Raupp Sganzerla

Os arquivos em `resultados/` foram gerados num AMD Ryzen 7 5700X com 16 GB de RAM, Windows 64 bits, g++ 16.2.0 (WinLibs), com `-std=c++17 -O2`.

## Estrutura

```
src/
  estruturas.hpp   Aresta, Union-Find otimizado e Union-Find ingênuo (comparação)
  ordenacao.hpp    Merge Sort próprio (bottom-up, estável)
  kruskal.hpp      O algoritmo guloso, com métricas e registro passo a passo
  grafos.hpp       Gerador de grafos conexos aleatórios e leitor de arquivo
  prim.hpp         Prim (heap), usado só para verificar o resultado do Kruskal
  experimento.hpp  Union-Find instrumentado para o teste da hipótese
  main.cpp         Casos de teste
entradas/
  pequeno.txt      Instância pequena (7 vértices, 11 arestas)
resultados/
  saida_completa.txt   Saída de "./kruskal todos"
```

## Como compilar

Linux / macOS / WSL / MinGW:

```bash
make
# ou
g++ -std=c++17 -O2 -Wall -o kruskal src/main.cpp
```

Windows com Visual Studio (Developer Command Prompt):

```bat
cl /std:c++17 /O2 /EHsc /Fe:kruskal.exe src\main.cpp
```

## Como executar

```bash
./kruskal pequeno        # Teste 1: passo a passo
./kruskal medio          # Teste 2: n=1.000, m=10.000
./kruskal grande         # Teste 3: n=1.000.000, m=10.000.000
./kruskal verificacao    # Kruskal x Prim em 1000 grafos aleatórios
./kruskal escala         # tempo para tamanhos crescentes
./kruskal otimizacao     # Union-Find otimizado x ingênuo, parada antecipada, ordenação
./kruskal densidade      # grafos esparsos x densos (Kruskal x Prim)
./kruskal hipotese       # por que o Union-Find ingênuo é lento (5 variantes)
./kruskal todos          # tudo, em sequência (cerca de 1 min e meio)

./kruskal arquivo entradas/pequeno.txt   # qualquer grafo em arquivo
./kruskal aleatorio 500 3000 7           # grafo aleatório: n m [semente]
```

O teste grande usa cerca de 600 MB de RAM no pico.

No Windows, troque `./kruskal` por `kruskal.exe`. Para guardar a saída num arquivo:

```bash
./kruskal todos > resultados/saida_completa.txt
```

## Formato do arquivo de entrada

```
# comentários começam com #
n m
u v peso
u v peso
...
```

Os vértices podem ser números ou nomes (A, B, C...). O grafo precisa ser conexo.
