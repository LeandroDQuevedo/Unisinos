// experimento.hpp
// Experimento da hipótese levantada durante o desenvolvimento:
// "O Union-Find ingênuo fica lento porque pendura uma raiz na outra sem
//  olhar o tamanho: a componente gigante acaba pendurada sob vértices
//  isolados, ganhando um nível de profundidade a cada união."
//
// Variantes do Union-Find comparadas, todas instrumentadas para contar
// quantos ponteiros o find percorre (passos) e a altura final da floresta:
//   - ingênuo           : raiz de v pendurada sob raiz de u (o do programa)
//   - ingênuo invertido : raiz de u pendurada sob raiz de v
//   - só rank           : união por rank, SEM compressão de caminho
//   - só compressão     : compressão de caminho, SEM união por rank
//   - otimizado         : as duas (o usado pelo Kruskal)
#pragma once

#include <chrono>
#include <cstdint>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "estruturas.hpp"

struct VarianteUF {
    std::string nome;
    bool porRank;      // união por rank
    bool compressao;   // compressão de caminho
    bool invertido;    // sem rank: pendura u sob v em vez de v sob u
};

class UnionFindInstrumentado {
public:
    UnionFindInstrumentado(int n, const VarianteUF& v)
        : pai_(n), rank_(n, 0), var_(v) {
        std::iota(pai_.begin(), pai_.end(), 0);
    }

    int find(int x) {
        ++chamadas;
        int raiz = x;
        while (pai_[raiz] != raiz) { raiz = pai_[raiz]; ++passos; }
        if (var_.compressao)
            while (pai_[x] != raiz) { int p = pai_[x]; pai_[x] = raiz; x = p; }
        return raiz;
    }

    bool unir(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (var_.porRank) {
            if (rank_[ra] < rank_[rb]) std::swap(ra, rb);
            pai_[rb] = ra;
            if (rank_[ra] == rank_[rb]) ++rank_[ra];
        } else if (var_.invertido) {
            pai_[ra] = rb;
        } else {
            pai_[rb] = ra;
        }
        // Métrica da hipótese: a união pendurou a árvore MAIOR sob a menor?
        // (tamanho calculado à parte, só para medir, não influencia a união)
        int pendurada = (pai_[rb] == ra) ? rb : ra;
        int quemFica = (pendurada == rb) ? ra : rb;
        if (tamanho(pendurada) > tamanho(quemFica)) ++maiorSobMenor;
        tam_[quemFica] = tamanho(quemFica) + tamanho(pendurada);
        ++unioes;
        return true;
    }

    // Altura da floresta: maior número de ponteiros de um nó até a raiz.
    int altura() const {
        const int n = static_cast<int>(pai_.size());
        std::vector<int> prof(n, -1);
        int maior = 0;
        std::vector<int> pilha;
        for (int i = 0; i < n; ++i) {
            int x = i;
            while (prof[x] < 0 && pai_[x] != x) { pilha.push_back(x); x = pai_[x]; }
            if (pai_[x] == x) prof[x] = 0;
            int d = prof[x];
            while (!pilha.empty()) { prof[pilha.back()] = ++d; pilha.pop_back(); }
            if (prof[i] > maior) maior = prof[i];
        }
        return maior;
    }

    std::int64_t chamadas = 0, passos = 0, unioes = 0, maiorSobMenor = 0;

private:
    int tamanho(int raiz) {
        if (tam_.empty()) tam_.assign(pai_.size(), 1);
        return tam_[raiz];
    }
    std::vector<int> pai_, rank_, tam_;
    VarianteUF var_;
};

struct ResultadoExperimento {
    double ms;
    std::int64_t chamadas, passos, unioes, maiorSobMenor;
    int altura;
};

// Roda o laço do Kruskal com a variante dada, sobre arestas JÁ ordenadas.
inline ResultadoExperimento rodarVariante(int n, const std::vector<Aresta>& ordenadas,
                                          const VarianteUF& v) {
    UnionFindInstrumentado uf(n, v);
    int aceitas = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (const Aresta& a : ordenadas)
        if (uf.unir(a.u, a.v) && ++aceitas == n - 1) break;
    auto t1 = std::chrono::steady_clock::now();
    return {std::chrono::duration<double, std::milli>(t1 - t0).count(),
            uf.chamadas, uf.passos, uf.unioes, uf.maiorSobMenor, uf.altura()};
}
