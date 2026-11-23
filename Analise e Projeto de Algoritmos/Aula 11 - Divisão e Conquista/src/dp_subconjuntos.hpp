// dp_subconjuntos.hpp
// Árvore Geradora Mínima por PROGRAMAÇÃO DINÂMICA sobre subconjuntos de
// vértices (formulação Bottom-Up descrita no relatório da Aula 10).
//
// Estado:      dp[S] = menor custo de uma árvore geradora do subgrafo
//              induzido pelos vértices de S (S é uma máscara de bits).
// Casos base:  dp[{v}] = 0.
// Recorrência: dp[S] = min, para v em S, de
//                         dp[S \ {v}] + min, para u em S \ {v}, de w(v, u)
//              (v entra como folha, ligada ao vizinho mais barato).
// Resultado:   dp[V]; INF significa que o grafo é desconexo.
//
// Custo: O(n^2 * 2^n) de tempo e O(2^n + n^2) de memória. Só serve para
// grafos pequenos; por segurança a função recusa n > 22.
#pragma once

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "estruturas.hpp"

struct ResultadoPD {
    std::vector<Aresta> arvore;
    std::int64_t pesoTotal = 0;
    std::int64_t estados = 0;         // subconjuntos não vazios tabulados
    std::int64_t transicoes = 0;      // pares (S, v) avaliados
    std::int64_t consultasPeso = 0;   // leituras da matriz w
    std::int64_t bytesAuxiliares = 0;
    double ms = 0;
};

inline ResultadoPD agmProgramacaoDinamica(int n, const std::vector<Aresta>& arestas) {
    if (n < 1 || n > 22)
        throw std::invalid_argument("PD por subconjuntos: n deve estar entre 1 e 22");
    const std::int64_t INF = INT64_MAX / 4;
    ResultadoPD r;
    auto t0 = std::chrono::steady_clock::now();

    // Matriz de pesos: menor peso entre cada par (INF = sem aresta)
    // (guardada num vetor contíguo: w(v, u) está em w[v * n + u])
    std::vector<std::int64_t> w(static_cast<std::size_t>(n) * n, INF);
    for (const Aresta& a : arestas)
        if (a.u != a.v && a.peso < w[a.u * n + a.v])
            w[a.u * n + a.v] = w[a.v * n + a.u] = a.peso;

    const std::uint32_t total = 1u << n;
    std::vector<std::int64_t> dp(total, INF);
    std::vector<std::int8_t> ultimoVertice(total, -1), seuVizinho(total, -1);

    // Máscaras em ordem crescente: S sem um bit é sempre menor que S, logo
    // dp[S \ {v}] já está preenchido quando S é processado.
    for (std::uint32_t S = 1; S < total; ++S) {
        ++r.estados;
        if ((S & (S - 1)) == 0) {      // caso base: um único vértice
            dp[S] = 0;
            continue;
        }
        for (int v = 0; v < n; ++v) {
            if (!(S >> v & 1)) continue;
            std::uint32_t resto = S ^ (1u << v);
            if (dp[resto] >= INF) continue;     // resto desconexo: inviável
            ++r.transicoes;
            // vizinho mais barato de v dentro do resto
            std::int64_t melhor = INF;
            int melhorU = -1;
            const std::int64_t* linha = &w[static_cast<std::size_t>(v) * n];
            for (int u = 0; u < n; ++u) {
                if (!(resto >> u & 1)) continue;
                ++r.consultasPeso;
                if (linha[u] < melhor) { melhor = linha[u]; melhorU = u; }
            }
            if (melhor < INF && dp[resto] + melhor < dp[S]) {
                dp[S] = dp[resto] + melhor;
                ultimoVertice[S] = static_cast<std::int8_t>(v);
                seuVizinho[S] = static_cast<std::int8_t>(melhorU);
            }
        }
    }

    if (dp[total - 1] >= INF)
        throw std::runtime_error("O grafo nao e conexo: nao existe arvore geradora.");
    r.pesoTotal = dp[total - 1];

    // Reconstrução: desfaz as decisões a partir do conjunto completo
    for (std::uint32_t S = total - 1; (S & (S - 1)) != 0;) {
        int v = ultimoVertice[S], u = seuVizinho[S];
        r.arvore.push_back({static_cast<int>(w[v * n + u]), u, v});
        S ^= 1u << v;
    }
    r.ms = std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - t0).count();
    r.bytesAuxiliares = static_cast<std::int64_t>(
        total * (sizeof(std::int64_t) + 2 * sizeof(std::int8_t)) +
        static_cast<std::size_t>(n) * n * sizeof(std::int64_t));
    return r;
}
