// dc_arestas.hpp
// Árvore Geradora Mínima por DIVISÃO E CONQUISTA sobre o conjunto de arestas.
//
// Ideia (propriedade do ciclo): se uma aresta é a mais pesada de um ciclo
// formado só por arestas de um subconjunto E1 de E, ela também é a mais
// pesada de um ciclo em E e pode ficar fora da AGM. Logo
//
//        AGM(E) = AGM( FGM(E1) U FGM(E2) ),   E = E1 U E2,
//
// onde FGM é a floresta geradora mínima (a metade de um grafo conexo pode
// ser desconexa). O algoritmo:
//
//   DIVIDIR    parte a lista de arestas ao meio (por posição, não por peso);
//   CONQUISTAR resolve cada metade recursivamente: cada chamada devolve a
//              FGM da sua metade, com as arestas em ordem crescente de peso;
//   CASO BASE  lista com 0 ou 1 aresta: ela mesma já é a sua FGM;
//   COMBINAR   intercala as duas florestas por peso (como no Merge Sort) e
//              passa cada aresta pelo Union-Find: quem ligaria dois vértices
//              já conectados fecha ciclo e é descartada.
//
// Cada floresta tem no máximo n-1 arestas, então a combinação nunca recebe
// mais de 2(n-1) arestas, por maior que seja a metade original.
//
// Recorrência: T(m) = 2T(m/2) + O(min(m, n) * alfa(n))  =>  O(m log n).
// Espaço: O(m) da cópia da entrada + O(n) de vetor auxiliar e Union-Find
// + O(log m) de pilha de recursão.
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "estruturas.hpp"
#include "ordenacao.hpp"

// Union-Find com compressão de caminho e união por rank que pode ser
// "desfeito" vértice a vértice. Cada combinação usa a estrutura do zero;
// reiniciar só os vértices tocados custa O(arestas combinadas), enquanto
// criar um Union-Find novo custaria O(n) em cada uma das m-1 combinações.
class UnionFindReutilizavel {
public:
    explicit UnionFindReutilizavel(int n) : pai_(n), rank_(n, 0) {
        for (int i = 0; i < n; ++i) pai_[i] = i;
    }

    int find(int x) {
        ++chamadasFind;
        int raiz = x;
        while (pai_[raiz] != raiz) raiz = pai_[raiz];
        while (pai_[x] != raiz) {          // compressão de caminho
            int proximo = pai_[x];
            pai_[x] = raiz;
            x = proximo;
        }
        return raiz;
    }

    bool unir(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank_[ra] < rank_[rb]) std::swap(ra, rb);
        pai_[rb] = ra;
        if (rank_[ra] == rank_[rb]) ++rank_[ra];
        return true;
    }

    // Devolve o vértice x ao estado inicial (sozinho na sua componente).
    void reiniciar(int x) {
        pai_[x] = x;
        rank_[x] = 0;
    }

    std::int64_t chamadasFind = 0;

private:
    std::vector<int> pai_;
    std::vector<int> rank_;
};

// Um nó da árvore de recursão (registrado só quando pedido, para exibir).
struct NoRecursao {
    int id = 0;
    int pai = -1;                    // id do nó que fez a chamada (-1 = raiz)
    int profundidade = 0;
    std::size_t inicio = 0, fim = 0; // intervalo [inicio, fim) da lista
    bool casoBase = false;
    std::vector<Aresta> entrada;     // arestas recebidas
    std::vector<Aresta> saida;       // floresta devolvida
    std::vector<Aresta> descartadas; // rejeitadas na combinação (ciclo)
};

struct ResultadoDC {
    std::vector<Aresta> arvore;          // n-1 arestas, em ordem de peso
    std::int64_t pesoTotal = 0;
    std::int64_t chamadas = 0;           // chamadas recursivas (nós da árvore)
    std::int64_t divisoes = 0;           // nós internos (cada um divide em 2)
    std::int64_t casosBase = 0;          // folhas
    std::int64_t profundidade = 0;       // nível mais fundo (raiz = 0)
    std::int64_t comparacoes = 0;        // comparações de peso na intercalação
    std::int64_t arestasCombinadas = 0;  // arestas que entraram em combinações
    std::int64_t descartadas = 0;        // rejeitadas por formar ciclo
    std::int64_t chamadasFind = 0;
    std::int64_t bytesAuxiliares = 0;    // memória extra pedida pelo algoritmo
    double ms = 0;
    // Perfil por nível da árvore de recursão (índice = profundidade):
    std::vector<std::int64_t> chamadasPorNivel;    // nós no nível
    std::vector<std::int64_t> combinadasPorNivel;  // arestas que chegaram às combinações
    std::vector<std::int64_t> devolvidasPorNivel;  // arestas que sobreviveram
    std::vector<NoRecursao> nos;         // árvore de recursão (opcional)
};

struct OpcoesDC {
    bool registrarArvore = false;  // guardar cada nó da recursão
    std::int64_t limiteCasoBase = 1;  // blocos até este tamanho são caso base
    int niveisParalelos = 0;       // 0 = sequencial; k = 2^k tarefas
};

namespace detalhe_dc {

// Estado de trabalho de uma linha de execução: vetor auxiliar, Union-Find e
// contadores. Na versão paralela cada thread tem o seu.
struct Contexto {
    Contexto(int n, std::size_t capacidadeAux)
        : conjuntos(n), aux(capacidadeAux) {}
    UnionFindReutilizavel conjuntos;
    std::vector<Aresta> aux;
    std::int64_t chamadas = 0, divisoes = 0, casosBase = 0, profundidade = 0;
    std::int64_t comparacoes = 0, arestasCombinadas = 0, descartadas = 0;
    std::vector<std::int64_t> chamadasPorNivel = std::vector<std::int64_t>(64, 0);
    std::vector<std::int64_t> combinadasPorNivel = std::vector<std::int64_t>(64, 0);
    std::vector<std::int64_t> devolvidasPorNivel = std::vector<std::int64_t>(64, 0);
};

class Resolvedor {
public:
    Resolvedor(int n, std::vector<Aresta>& arestas, const OpcoesDC& op,
               std::vector<NoRecursao>* nos)
        : n_(n), a_(arestas), op_(op), nos_(nos) {}

    // Resolve a_[inicio, fim). Ao voltar, a floresta geradora mínima desse
    // trecho está em a_[inicio, inicio + k), em ordem crescente de peso.
    // Devolve k.
    std::size_t resolver(std::size_t inicio, std::size_t fim, int prof,
                         int idPai, Contexto& ctx) {
        ++ctx.chamadas;
        ++ctx.chamadasPorNivel[prof];
        ctx.profundidade = std::max<std::int64_t>(ctx.profundidade, prof);
        int id = registrarEntrada(inicio, fim, prof, idPai);

        // ---- CASO BASE: 0 ou 1 aresta já é a própria floresta mínima.
        // (Com limiteCasoBase > 1, blocos pequenos são resolvidos direto
        // por um Kruskal local: ordenação por inserção + Union-Find.)
        std::size_t tamanho = fim - inicio;
        if (tamanho <= 1 ||
            tamanho <= static_cast<std::size_t>(op_.limiteCasoBase)) {
            ++ctx.casosBase;
            std::size_t k = tamanho <= 1 ? tamanho
                                         : resolverDireto(inicio, fim, ctx);
            ctx.devolvidasPorNivel[prof] += static_cast<std::int64_t>(k);
            registrarSaida(id, inicio, k, true);
            return k;
        }

        // ---- DIVIDIR: duas metades da lista de arestas.
        ++ctx.divisoes;
        std::size_t meio = inicio + tamanho / 2;

        // ---- CONQUISTAR: as metades não compartilham arestas nem
        // resultados, então podem ser resolvidas em qualquer ordem, ou ao
        // mesmo tempo.
        std::size_t kEsq, kDir;
        if (prof < op_.niveisParalelos && nos_ == nullptr) {
            Contexto outro(n_, ctx.aux.size());
            std::thread tarefa([&] {
                kEsq = resolver(inicio, meio, prof + 1, id, outro);
            });
            kDir = resolver(meio, fim, prof + 1, id, ctx);
            tarefa.join();
            somar(ctx, outro);
        } else {
            kEsq = resolver(inicio, meio, prof + 1, id, ctx);
            kDir = resolver(meio, fim, prof + 1, id, ctx);
        }

        // ---- COMBINAR: intercalação por peso + filtro de ciclos.
        std::size_t k = combinar(inicio, kEsq, meio, kDir, id, ctx);
        ctx.combinadasPorNivel[prof] += static_cast<std::int64_t>(kEsq + kDir);
        ctx.devolvidasPorNivel[prof] += static_cast<std::int64_t>(k);
        registrarSaida(id, inicio, k, false);
        return k;
    }

private:
    // Junta as florestas a_[inicio, inicio+kEsq) e a_[meio, meio+kDir),
    // ambas ordenadas por peso, numa única floresta ordenada.
    std::size_t combinar(std::size_t inicio, std::size_t kEsq,
                         std::size_t meio, std::size_t kDir, int id,
                         Contexto& ctx) {
        std::size_t i = inicio, fimEsq = inicio + kEsq;
        std::size_t j = meio, fimDir = meio + kDir;
        std::size_t k = 0;
        const std::size_t completa = n_ > 0 ? static_cast<std::size_t>(n_ - 1) : 0;
        ctx.arestasCombinadas += static_cast<std::int64_t>(kEsq + kDir);

        while ((i < fimEsq || j < fimDir) && k < completa) {
            // Próxima aresta mais leve entre as duas florestas. O "<="
            // mantém a estabilidade: no empate vem a da esquerda, isto é,
            // a que aparecia antes na entrada.
            bool daEsquerda;
            if (i < fimEsq && j < fimDir) {
                ++ctx.comparacoes;
                daEsquerda = a_[i].peso <= a_[j].peso;
            } else {
                daEsquerda = i < fimEsq;
            }
            const Aresta& c = daEsquerda ? a_[i++] : a_[j++];

            if (ctx.conjuntos.unir(c.u, c.v)) {
                ctx.aux[k++] = c;        // liga duas componentes: fica
            } else {
                ++ctx.descartadas;       // fecharia ciclo: sai de vez
                if (nos_) (*nos_)[id].descartadas.push_back(c);
            }
        }
        // Se a árvore completou n-1 arestas, o resto das florestas é
        // necessariamente mais pesado e fecharia ciclo (parada antecipada).
        ctx.descartadas += static_cast<std::int64_t>((fimEsq - i) + (fimDir - j));
        if (nos_) {
            for (std::size_t p = i; p < fimEsq; ++p) (*nos_)[id].descartadas.push_back(a_[p]);
            for (std::size_t p = j; p < fimDir; ++p) (*nos_)[id].descartadas.push_back(a_[p]);
        }

        // Devolve ao estado inicial apenas os vértices que foram tocados.
        for (std::size_t p = inicio; p < fimEsq; ++p) {
            ctx.conjuntos.reiniciar(a_[p].u);
            ctx.conjuntos.reiniciar(a_[p].v);
        }
        for (std::size_t p = meio; p < fimDir; ++p) {
            ctx.conjuntos.reiniciar(a_[p].u);
            ctx.conjuntos.reiniciar(a_[p].v);
        }
        std::copy(ctx.aux.begin(), ctx.aux.begin() + k, a_.begin() + inicio);
        return k;
    }

    // Caso base ampliado (otimização opcional): o bloco é resolvido direto
    // por um Kruskal local. Blocos curtos usam ordenação por inserção; os
    // maiores usam o Merge Sort iterativo da Aula 9. As duas são estáveis.
    std::size_t resolverDireto(std::size_t inicio, std::size_t fim,
                               Contexto& ctx) {
        if (fim - inicio <= 32) {
            for (std::size_t p = inicio + 1; p < fim; ++p) {
                Aresta x = a_[p];
                std::size_t q = p;
                while (q > inicio && a_[q - 1].peso > x.peso) {
                    a_[q] = a_[q - 1];
                    --q;
                }
                a_[q] = x;
            }
        } else {
            std::vector<Aresta> bloco(a_.begin() + inicio, a_.begin() + fim);
            ctx.comparacoes += mergeSort(bloco);
            std::copy(bloco.begin(), bloco.end(), a_.begin() + inicio);
        }
        std::size_t k = 0;
        for (std::size_t p = inicio; p < fim; ++p)
            if (ctx.conjuntos.unir(a_[p].u, a_[p].v)) ctx.aux[k++] = a_[p];
            else ++ctx.descartadas;
        for (std::size_t p = inicio; p < fim; ++p) {
            ctx.conjuntos.reiniciar(a_[p].u);
            ctx.conjuntos.reiniciar(a_[p].v);
        }
        std::copy(ctx.aux.begin(), ctx.aux.begin() + k, a_.begin() + inicio);
        return k;
    }

    static void somar(Contexto& a, const Contexto& b) {
        a.chamadas += b.chamadas;
        a.divisoes += b.divisoes;
        a.casosBase += b.casosBase;
        a.profundidade = std::max(a.profundidade, b.profundidade);
        a.comparacoes += b.comparacoes;
        a.arestasCombinadas += b.arestasCombinadas;
        a.descartadas += b.descartadas;
        a.conjuntos.chamadasFind += b.conjuntos.chamadasFind;
        for (std::size_t i = 0; i < a.chamadasPorNivel.size(); ++i) {
            a.chamadasPorNivel[i] += b.chamadasPorNivel[i];
            a.combinadasPorNivel[i] += b.combinadasPorNivel[i];
            a.devolvidasPorNivel[i] += b.devolvidasPorNivel[i];
        }
    }

    int registrarEntrada(std::size_t inicio, std::size_t fim, int prof, int idPai) {
        if (!nos_) return -1;
        NoRecursao no;
        no.id = static_cast<int>(nos_->size());
        no.pai = idPai;
        no.profundidade = prof;
        no.inicio = inicio;
        no.fim = fim;
        no.entrada.assign(a_.begin() + inicio, a_.begin() + fim);
        nos_->push_back(no);
        return no.id;
    }

    void registrarSaida(int id, std::size_t inicio, std::size_t k, bool base) {
        if (!nos_) return;
        (*nos_)[id].casoBase = base;
        (*nos_)[id].saida.assign(a_.begin() + inicio, a_.begin() + inicio + k);
    }

    int n_;
    std::vector<Aresta>& a_;
    const OpcoesDC& op_;
    std::vector<NoRecursao>* nos_;
};

}  // namespace detalhe_dc

// Recebe as arestas por valor: a cópia é reorganizada pelo algoritmo e o
// vetor do chamador fica intacto (mesma convenção do kruskal()).
inline ResultadoDC agmDivisaoConquista(int n, std::vector<Aresta> arestas,
                                       const OpcoesDC& op = {}) {
    ResultadoDC r;
    auto t0 = std::chrono::steady_clock::now();

    // Nenhuma chamada devolve mais que n-1 arestas (é uma floresta), então
    // o vetor auxiliar precisa de no máximo min(m, n) posições.
    std::size_t capacidade = std::min<std::size_t>(
        arestas.size(), static_cast<std::size_t>(std::max(n, 1)));
    detalhe_dc::Contexto ctx(n, capacidade);
    detalhe_dc::Resolvedor resolvedor(n, arestas, op,
                                      op.registrarArvore ? &r.nos : nullptr);

    std::size_t k = resolvedor.resolver(0, arestas.size(), 0, -1, ctx);

    r.arvore.assign(arestas.begin(), arestas.begin() + k);
    for (const Aresta& a : r.arvore) r.pesoTotal += a.peso;
    r.ms = std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - t0).count();

    r.chamadas = ctx.chamadas;
    r.divisoes = ctx.divisoes;
    r.casosBase = ctx.casosBase;
    r.profundidade = ctx.profundidade;
    r.comparacoes = ctx.comparacoes;
    r.arestasCombinadas = ctx.arestasCombinadas;
    r.descartadas = ctx.descartadas;
    r.chamadasFind = ctx.conjuntos.chamadasFind;
    std::size_t niveis = static_cast<std::size_t>(ctx.profundidade) + 1;
    r.chamadasPorNivel.assign(ctx.chamadasPorNivel.begin(), ctx.chamadasPorNivel.begin() + niveis);
    r.combinadasPorNivel.assign(ctx.combinadasPorNivel.begin(), ctx.combinadasPorNivel.begin() + niveis);
    r.devolvidasPorNivel.assign(ctx.devolvidasPorNivel.begin(), ctx.devolvidasPorNivel.begin() + niveis);
    // cópia da entrada + vetor auxiliar + dois vetores do Union-Find
    r.bytesAuxiliares = static_cast<std::int64_t>(
        arestas.size() * sizeof(Aresta) + capacidade * sizeof(Aresta) +
        2 * static_cast<std::size_t>(n) * sizeof(int));

    if (n > 0 && static_cast<int>(r.arvore.size()) != n - 1)
        throw std::runtime_error(
            "O grafo nao e conexo: nao existe arvore geradora.");
    return r;
}
