// ordenacao.hpp
// Merge Sort implementado do zero para ordenar as arestas por peso.
//
// Por que Merge Sort:
//   - O(E log E) garantido, sem o pior caso O(n^2) do Quick Sort.
//   - É estável: arestas de mesmo peso mantêm a ordem de entrada, o que
//     deixa a execução do Kruskal determinística e fácil de acompanhar.
//   - Custo: um vetor auxiliar de E arestas (O(E) de memória extra).
#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "estruturas.hpp"

// Mescla v[inicio, meio) e v[meio, fim), já ordenados, em aux[inicio, fim).
inline void mesclar(const std::vector<Aresta>& v, std::vector<Aresta>& aux,
                    std::size_t inicio, std::size_t meio, std::size_t fim,
                    std::int64_t& comparacoes) {
    std::size_t i = inicio, j = meio, k = inicio;
    while (i < meio && j < fim) {
        ++comparacoes;
        // "<=" garante a estabilidade: no empate vem o da esquerda
        if (v[i].peso <= v[j].peso) aux[k++] = v[i++];
        else                        aux[k++] = v[j++];
    }
    while (i < meio) aux[k++] = v[i++];   // copia o que sobrou
    while (j < fim)  aux[k++] = v[j++];
}

// Ordena 'arestas' em ordem crescente de peso (no próprio vetor).
// Versão bottom-up (iterativa): mescla blocos de tamanho 1, 2, 4, 8...
// Retorna o número de comparações de peso realizadas (métrica).
inline std::int64_t mergeSort(std::vector<Aresta>& arestas) {
    std::int64_t comparacoes = 0;
    const std::size_t n = arestas.size();
    std::vector<Aresta> aux(n);
    std::vector<Aresta>* origem = &arestas;
    std::vector<Aresta>* destino = &aux;

    for (std::size_t largura = 1; largura < n; largura *= 2) {
        for (std::size_t inicio = 0; inicio < n; inicio += 2 * largura) {
            std::size_t meio = std::min(inicio + largura, n);
            std::size_t fim = std::min(inicio + 2 * largura, n);
            mesclar(*origem, *destino, inicio, meio, fim, comparacoes);
        }
        std::swap(origem, destino);   // troca os papéis dos vetores
    }
    if (origem != &arestas) arestas.swap(aux);   // resultado no vetor original
    return comparacoes;
}
