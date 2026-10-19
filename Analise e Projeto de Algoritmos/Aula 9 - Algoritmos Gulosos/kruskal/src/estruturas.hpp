// estruturas.hpp
// Estruturas de dados usadas pelo algoritmo de Kruskal:
//   - Aresta: (peso, u, v) de um grafo não direcionado.
//   - UnionFind: conjuntos disjuntos com compressão de caminho e união por rank.
//   - UnionFindIngenuo: versão sem otimizações, usada só para comparação.
#pragma once

#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

// Uma aresta não direcionada entre os vértices u e v.
// 12 bytes por aresta: vetores contíguos de Aresta são amigáveis ao cache.
struct Aresta {
    int peso;
    int u;
    int v;
};

// Conjuntos disjuntos (Disjoint Set Union) otimizados.
//
// Cada vértice começa num conjunto próprio. O Kruskal pergunta à estrutura:
// "u e v já estão na mesma componente?" Se sim, a aresta u-v fecharia um
// ciclo e é descartada.
//
// Otimizações:
//   - Compressão de caminho: no find, todo nó visitado passa a apontar
//     direto para a raiz, achatando a árvore.
//   - União por rank: a árvore mais baixa é pendurada na mais alta, o que
//     limita a altura a O(log n).
// Juntas dão custo amortizado O(alfa(n)) por operação (alfa = inversa da
// função de Ackermann; na prática alfa(n) <= 4).
class UnionFind {
public:
    explicit UnionFind(int n) : pai_(n), rank_(n, 0) {
        std::iota(pai_.begin(), pai_.end(), 0);   // cada vértice é sua raiz
    }

    // Retorna o representante (raiz) do conjunto de x.
    int find(int x) {
        ++chamadasFind;
        int raiz = x;
        while (pai_[raiz] != raiz) raiz = pai_[raiz];
        // Compressão de caminho (iterativa: sem risco de estouro de pilha)
        while (pai_[x] != raiz) {
            int proximo = pai_[x];
            pai_[x] = raiz;
            x = proximo;
        }
        return raiz;
    }

    // Une os conjuntos de a e b. Retorna false se já estavam unidos.
    bool unir(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank_[ra] < rank_[rb]) std::swap(ra, rb);   // ra é a mais alta
        pai_[rb] = ra;
        if (rank_[ra] == rank_[rb]) ++rank_[ra];
        return true;
    }

    std::int64_t chamadasFind = 0;   // métrica

private:
    std::vector<int> pai_;
    std::vector<int> rank_;   // limite superior da altura de cada árvore
};

// Conjuntos disjuntos SEM compressão de caminho e SEM união por rank.
// Serve só de base de comparação: aqui as árvores podem virar "listas" e o
// find custa O(n) no pior caso.
class UnionFindIngenuo {
public:
    explicit UnionFindIngenuo(int n) : pai_(n) {
        std::iota(pai_.begin(), pai_.end(), 0);
    }

    int find(int x) {
        ++chamadasFind;
        while (pai_[x] != x) x = pai_[x];
        return x;
    }

    bool unir(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        pai_[rb] = ra;   // sempre pendura b em a, sem critério
        return true;
    }

    std::int64_t chamadasFind = 0;

private:
    std::vector<int> pai_;
};
