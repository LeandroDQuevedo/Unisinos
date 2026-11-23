// mergesort_recursivo.hpp
// Variante de comparação: o Kruskal guloso da Aula 9 com a ordenação feita
// por um Merge Sort RECURSIVO (top-down), o exemplo clássico de Divisão e
// Conquista (CORMEN et al., 2012, seção 2.3).
//
// Aqui o paradigma aparece só na ORDENAÇÃO das arestas; a construção da
// árvore continua gulosa. Serve para separar duas perguntas:
//   - quanto custa a recursão em relação ao Merge Sort iterativo da Aula 9?
//   - o que muda quando o D&C é aplicado ao problema da AGM em si
//     (dc_arestas.hpp), e não só à ordenação?
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "estruturas.hpp"
#include "kruskal.hpp"

struct MetricasMergeRecursivo {
    std::int64_t chamadas = 0;
    std::int64_t divisoes = 0;
    std::int64_t casosBase = 0;
    std::int64_t profundidade = 0;
    std::int64_t comparacoes = 0;
};

namespace detalhe_ms {

// Ordena v[inicio, fim) por peso crescente.
inline void ordenar(std::vector<Aresta>& v, std::vector<Aresta>& aux,
                    std::size_t inicio, std::size_t fim, int prof,
                    MetricasMergeRecursivo& mt) {
    ++mt.chamadas;
    mt.profundidade = std::max<std::int64_t>(mt.profundidade, prof);

    // CASO BASE: 0 ou 1 elemento já está ordenado.
    if (fim - inicio <= 1) {
        ++mt.casosBase;
        return;
    }
    // DIVIDIR
    ++mt.divisoes;
    std::size_t meio = inicio + (fim - inicio) / 2;
    // CONQUISTAR
    ordenar(v, aux, inicio, meio, prof + 1, mt);
    ordenar(v, aux, meio, fim, prof + 1, mt);
    // COMBINAR: intercala as duas metades ordenadas (estável)
    std::size_t i = inicio, j = meio, k = inicio;
    while (i < meio && j < fim) {
        ++mt.comparacoes;
        if (v[i].peso <= v[j].peso) aux[k++] = v[i++];
        else                        aux[k++] = v[j++];
    }
    while (i < meio) aux[k++] = v[i++];
    while (j < fim)  aux[k++] = v[j++];
    std::copy(aux.begin() + inicio, aux.begin() + fim, v.begin() + inicio);
}

}  // namespace detalhe_ms

inline MetricasMergeRecursivo mergeSortRecursivo(std::vector<Aresta>& arestas) {
    MetricasMergeRecursivo mt;
    std::vector<Aresta> aux(arestas.size());
    detalhe_ms::ordenar(arestas, aux, 0, arestas.size(), 0, mt);
    return mt;
}

struct ResultadoKruskalMS {
    ResultadoKruskal kruskal;        // árvore, peso, tempos, métricas do laço
    MetricasMergeRecursivo ordenacao;
};

// Kruskal guloso com ordenação por Merge Sort recursivo.
inline ResultadoKruskalMS kruskalMergeRecursivo(int n, std::vector<Aresta> arestas) {
    ResultadoKruskalMS r;
    auto t0 = detalhe::Relogio::now();
    r.ordenacao = mergeSortRecursivo(arestas);
    auto t1 = detalhe::Relogio::now();
    detalhe::selecionar<UnionFind>(n, arestas, OpcoesKruskal{}, r.kruskal);
    auto t2 = detalhe::Relogio::now();

    r.kruskal.comparacoesOrdenacao = r.ordenacao.comparacoes;
    r.kruskal.descartadas =
        r.kruskal.examinadas - static_cast<std::int64_t>(r.kruskal.arvore.size());
    r.kruskal.msOrdenacao = detalhe::ms(t0, t1);
    r.kruskal.msSelecao = detalhe::ms(t1, t2);
    if (n > 0 && static_cast<int>(r.kruskal.arvore.size()) != n - 1)
        throw std::runtime_error(
            "O grafo nao e conexo: nao existe arvore geradora.");
    return r;
}
