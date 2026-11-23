# Aula 11 - Divisão e Conquista aplicada à Árvore Geradora Mínima

Trabalho de **Análise e Projeto de Algoritmos** (Unisinos, 2026/2). Continuação das Aulas 9 (Kruskal guloso) e 10 (Programação Dinâmica): o mesmo problema, agora por Divisão e Conquista, em C++17 e sem bibliotecas que resolvam a AGM.

Equipe: Felipe Flores e Leandro Berwanger · Profa. Maria Adelina Raupp Sganzerla

## Estrutura

```
src/
  dc_arestas.hpp           NOVO  AGM por Divisão e Conquista sobre a lista de arestas
  mergesort_recursivo.hpp  NOVO  Kruskal com Merge Sort recursivo (D&C só na ordenação)
  dp_subconjuntos.hpp      PD por subconjuntos (Aula 10), reescrita a partir do relatório
  main_aula11.cpp          NOVO  casos de teste e comparações
  estruturas.hpp, ordenacao.hpp, kruskal.hpp, grafos.hpp, prim.hpp   (Aula 9, sem alteração)
entradas/pequeno.txt       instância de 7 vértices e 11 arestas
resultados/
  saida_completa.txt       saída de "./aula11 todos"
  arvore_pequeno.json      árvore de recursão do teste pequeno
figuras/gerar_figuras.py   gera as figuras do relatório a partir dos resultados
```

## Compilar e executar

```bash
make            # ou: g++ -std=c++17 -O2 -pthread -o aula11 src/main_aula11.cpp
./aula11 pequeno       # Teste 1: árvore de recursão completa
./aula11 medio         # Teste 2: n=1.000, m=10.000
./aula11 grande        # Teste 3: n=1.000.000, m=10.000.000
./aula11 tres          # guloso x PD x D&C nos grafos da Aula 10 (7, 14 e 20 vértices)
./aula11 verificacao   # 1.000 grafos aleatórios + casos-limite
./aula11 escala        # tempo x tamanho
./aula11 densidade     # de esparso a completo
./aula11 otimizacao    # caso base ampliado e subproblemas em paralelo
./aula11 todos         # tudo (cerca de 9 minutos)

./aula11 arquivo entradas/pequeno.txt
./aula11 aleatorio 500 3000 7      # n m [semente]
```

Windows (WinLibs/MinGW): `g++ -std=c++17 -O2 -o aula11.exe src\main_aula11.cpp` e `aula11.exe todos`.

## O algoritmo

```
AGM-DC(arestas[inicio, fim)):
  se o trecho tem 0 ou 1 aresta: devolve o próprio trecho      (caso base)
  meio <- ponto médio do trecho                                 (dividir)
  F1 <- AGM-DC(arestas[inicio, meio))                           (conquistar)
  F2 <- AGM-DC(arestas[meio, fim))
  devolve COMBINAR(F1, F2)                                      (combinar)

COMBINAR(F1, F2):   // as duas florestas chegam ordenadas por peso
  intercala F1 e F2 por peso; cada aresta passa pelo Union-Find:
    liga duas componentes -> fica;  fecharia ciclo -> descartada
```

Correto pela propriedade do ciclo: a aresta descartada é a mais pesada de um ciclo, logo existe uma AGM sem ela. Tempo O(m log n), espaço O(n + m).

Os arquivos em `resultados/` foram gerados em Linux x86-64 (Intel Xeon 2,1 GHz, 2 núcleos, 7,8 GB), g++ 13.3.0, `-std=c++17 -O2`.
