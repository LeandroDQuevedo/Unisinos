// prim.hpp
// Algoritmo de Prim (com heap binário), usado SOMENTE para verificação.
//
// Prim é um algoritmo independente que também encontra uma AGM. O peso da
// AGM é único (mesmo quando há várias árvores mínimas), então Kruskal e
// Prim precisam chegar ao mesmo peso total. Se isso acontece em centenas de
// grafos aleatórios, há forte evidência de que o Kruskal está correto.
//
// std::priority_queue é apenas uma fila de prioridade genérica; não
// resolve o problema da AGM.
#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

#include "estruturas.hpp"

inline std::int64_t primPesoTotal(int n, const std::vector<Aresta>& arestas) {
    if (n == 0) return 0;
    std::vector<std::vector<std::pair<int, int>>> adj(n);   // (peso, vizinho)
    for (const Aresta& a : arestas) {
        adj[a.u].push_back({a.peso, a.v});
        adj[a.v].push_back({a.peso, a.u});
    }
    std::vector<char> naArvore(n, 0);
    using Item = std::pair<int, int>;   // (peso da aresta de ligação, vértice)
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> heap;
    heap.push({0, 0});
    std::int64_t total = 0;
    int adicionados = 0;
    while (!heap.empty() && adicionados < n) {
        auto [peso, v] = heap.top();
        heap.pop();
        if (naArvore[v]) continue;
        naArvore[v] = 1;
        total += peso;
        ++adicionados;
        for (auto [pw, w] : adj[v])
            if (!naArvore[w]) heap.push({pw, w});
    }
    if (adicionados != n) throw std::runtime_error("Grafo nao conexo");
    return total;
}
