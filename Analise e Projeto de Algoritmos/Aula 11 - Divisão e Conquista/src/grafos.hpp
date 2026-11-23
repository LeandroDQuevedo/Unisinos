// grafos.hpp
// Geração aleatória e leitura de grafos de entrada.
//
// Estratégia de randomização (gerarGrafoConexo):
//   1. Embaralha os vértices e liga cada um a um vértice anterior sorteado.
//      Isso cria uma árvore aleatória e GARANTE que o grafo é conexo (o
//      Kruskal só produz árvore geradora em grafo conexo).
//   2. Acrescenta arestas extras sorteadas, sem laços (u == v) e sem
//      arestas repetidas, até atingir a quantidade pedida.
//   3. Pesos inteiros uniformes em [pesoMin, pesoMax].
//   4. Embaralha a lista final, para que a ordem de entrada não ajude a
//      ordenação.
// Usa std::mt19937_64 com semente fixa e sorteio/embaralhamento próprios
// (as distribuições da biblioteca padrão variam entre compiladores), então
// a mesma semente gera o mesmo grafo em qualquer máquina.
#pragma once

#include <cstdint>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "estruturas.hpp"

class Sorteador {
public:
    explicit Sorteador(std::uint64_t semente) : gerador_(semente) {}

    // Inteiro uniforme em [lo, hi] (viés desprezível para intervalos pequenos)
    std::int64_t entre(std::int64_t lo, std::int64_t hi) {
        std::uint64_t faixa = static_cast<std::uint64_t>(hi - lo) + 1;
        return lo + static_cast<std::int64_t>(gerador_() % faixa);
    }

    // Embaralhamento de Fisher-Yates
    template <typename T>
    void embaralhar(std::vector<T>& v) {
        for (std::size_t i = v.size(); i > 1; --i) {
            std::size_t j = static_cast<std::size_t>(entre(0, i - 1));
            std::swap(v[i - 1], v[j]);
        }
    }

private:
    std::mt19937_64 gerador_;
};

inline std::vector<Aresta> gerarGrafoConexo(int n, std::int64_t m,
                                            std::uint64_t semente = 42,
                                            int pesoMin = 1, int pesoMax = 1000) {
    const std::int64_t maxArestas = static_cast<std::int64_t>(n) * (n - 1) / 2;
    if (n < 1 || m < n - 1 || m > maxArestas)
        throw std::invalid_argument("m deve estar entre n-1 e n(n-1)/2");

    Sorteador rng(semente);
    std::unordered_set<std::uint64_t> existentes;
    existentes.reserve(static_cast<std::size_t>(m) * 2);
    std::vector<Aresta> arestas;
    arestas.reserve(static_cast<std::size_t>(m));

    auto chave = [n](int u, int v) {
        if (u > v) std::swap(u, v);
        return static_cast<std::uint64_t>(u) * n + v;
    };
    auto adicionar = [&](int u, int v) {
        if (u == v || !existentes.insert(chave(u, v)).second) return false;
        arestas.push_back({static_cast<int>(rng.entre(pesoMin, pesoMax)), u, v});
        return true;
    };

    // Passo 1: árvore aleatória (garante conexidade)
    std::vector<int> ordem(n);
    for (int i = 0; i < n; ++i) ordem[i] = i;
    rng.embaralhar(ordem);
    for (int i = 1; i < n; ++i)
        adicionar(ordem[i], ordem[rng.entre(0, i - 1)]);

    // Passo 2: arestas extras aleatórias
    if (m > maxArestas / 2) {
        // Grafo denso: sorteia entre os pares que faltam (evita muitas
        // rejeições por aresta repetida)
        std::vector<std::pair<int, int>> faltantes;
        for (int u = 0; u < n; ++u)
            for (int v = u + 1; v < n; ++v)
                if (!existentes.count(chave(u, v))) faltantes.push_back({u, v});
        rng.embaralhar(faltantes);
        for (std::size_t i = 0;
             static_cast<std::int64_t>(arestas.size()) < m; ++i)
            adicionar(faltantes[i].first, faltantes[i].second);
    } else {
        while (static_cast<std::int64_t>(arestas.size()) < m)
            adicionar(static_cast<int>(rng.entre(0, n - 1)),
                      static_cast<int>(rng.entre(0, n - 1)));
    }

    rng.embaralhar(arestas);
    return arestas;
}

// Grafo lido de arquivo texto.
//   primeira linha: n m
//   próximas m linhas: u v peso
// Vértices podem ser números ou rótulos (ex.: A, B, C).
// Linhas vazias ou iniciadas por '#' são ignoradas.
struct GrafoArquivo {
    int n = 0;
    std::vector<Aresta> arestas;
    std::vector<std::string> rotulos;   // rotulos[i] = nome do vértice i
};

inline GrafoArquivo lerArquivo(const std::string& caminho) {
    std::ifstream arq(caminho);
    if (!arq) throw std::runtime_error("Nao foi possivel abrir " + caminho);

    GrafoArquivo g;
    std::unordered_map<std::string, int> indice;
    auto idVertice = [&](const std::string& nome) {
        auto it = indice.find(nome);
        if (it != indice.end()) return it->second;
        int id = static_cast<int>(g.rotulos.size());
        indice[nome] = id;
        g.rotulos.push_back(nome);
        return id;
    };

    std::string linha;
    long long m = -1;
    while (std::getline(arq, linha)) {
        std::istringstream ss(linha);
        std::string a;
        if (!(ss >> a) || a[0] == '#') continue;
        if (m < 0) {                       // cabeçalho "n m"
            g.n = std::stoi(a);
            ss >> m;
            continue;
        }
        std::string b;
        int peso;
        if (!(ss >> b >> peso)) throw std::runtime_error("Linha invalida: " + linha);
        g.arestas.push_back({peso, idVertice(a), idVertice(b)});
    }
    if (static_cast<long long>(g.arestas.size()) != m)
        throw std::runtime_error("Quantidade de arestas diferente do cabecalho");
    if (static_cast<int>(g.rotulos.size()) != g.n)
        throw std::runtime_error("Quantidade de vertices diferente do cabecalho");
    return g;
}
