// kruskal.hpp
// Algoritmo de Kruskal para Árvore Geradora Mínima (AGM / MST).
//
// Entrada: número de vértices n e lista de arestas (peso, u, v) de um grafo
//          não direcionado, ponderado e conexo.
// Saída:   as n-1 arestas da AGM e o seu peso total.
//
// Estratégia gulosa: percorrer as arestas da MAIS LEVE para a MAIS PESADA e
// aceitar cada uma que não feche ciclo com as já escolhidas.
#pragma once

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "estruturas.hpp"
#include "ordenacao.hpp"

struct Passo {
    bool aceita;                 // true = entrou na árvore; false = ciclo
    Aresta aresta;
    std::int64_t pesoAcumulado;
};

struct ResultadoKruskal {
    std::vector<Aresta> arvore;      // arestas aceitas, na ordem de escolha
    std::int64_t pesoTotal = 0;
    std::int64_t examinadas = 0;     // quantas arestas o laço chegou a olhar
    std::int64_t descartadas = 0;    // rejeitadas por formar ciclo
    std::int64_t comparacoesOrdenacao = 0;
    std::int64_t chamadasFind = 0;
    double msOrdenacao = 0;          // milissegundos
    double msSelecao = 0;            // laço guloso + union-find
    std::vector<Passo> passos;       // registro passo a passo (opcional)

    double msTotal() const { return msOrdenacao + msSelecao; }
};

struct OpcoesKruskal {
    bool registrarPassos = false;    // guardar cada decisão para exibir
    bool unionFindOtimizado = true;  // compressão de caminho + rank
    bool paradaAntecipada = true;    // parar ao atingir n-1 arestas
};

namespace detalhe {

using Relogio = std::chrono::steady_clock;

inline double ms(Relogio::time_point a, Relogio::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

// Laço guloso, genérico no tipo de Union-Find (otimizado ou ingênuo).
template <typename UF>
void selecionar(int n, const std::vector<Aresta>& ordenadas,
                const OpcoesKruskal& op, ResultadoKruskal& r) {
    UF conjuntos(n);   // cada vértice começa sozinho na sua componente
    r.arvore.reserve(n > 0 ? n - 1 : 0);

    for (const Aresta& a : ordenadas) {
        ++r.examinadas;
        // unir() devolve false se u e v já estão na mesma componente,
        // isto é, se a aresta fecharia um ciclo -> descarta
        if (conjuntos.unir(a.u, a.v)) {
            r.arvore.push_back(a);
            r.pesoTotal += a.peso;
            if (op.registrarPassos) r.passos.push_back({true, a, r.pesoTotal});
            // Condição de parada: a árvore com n-1 arestas está completa
            if (op.paradaAntecipada &&
                static_cast<int>(r.arvore.size()) == n - 1)
                break;
        } else if (op.registrarPassos) {
            r.passos.push_back({false, a, r.pesoTotal});
        }
    }
    r.chamadasFind = conjuntos.chamadasFind;
}

}  // namespace detalhe

// Recebe as arestas por valor: a cópia é ordenada e o vetor do chamador
// fica intacto (permite repetir a medição sobre a mesma entrada).
inline ResultadoKruskal kruskal(int n, std::vector<Aresta> arestas,
                                const OpcoesKruskal& op = {}) {
    ResultadoKruskal r;

    // 1) Organização inicial: ordenar as arestas por peso crescente
    auto t0 = detalhe::Relogio::now();
    r.comparacoesOrdenacao = mergeSort(arestas);
    auto t1 = detalhe::Relogio::now();

    // 2) e 3) Seleção gulosa com detecção de ciclo via Union-Find
    if (op.unionFindOtimizado)
        detalhe::selecionar<UnionFind>(n, arestas, op, r);
    else
        detalhe::selecionar<UnionFindIngenuo>(n, arestas, op, r);
    auto t2 = detalhe::Relogio::now();

    r.descartadas = r.examinadas - static_cast<std::int64_t>(r.arvore.size());
    r.msOrdenacao = detalhe::ms(t0, t1);
    r.msSelecao = detalhe::ms(t1, t2);

    if (n > 0 && static_cast<int>(r.arvore.size()) != n - 1)
        throw std::runtime_error(
            "O grafo nao e conexo: nao existe arvore geradora.");
    return r;
}
