// main.cpp
// Programa principal: executa os casos de teste do algoritmo de Kruskal.
//
// Uso:
//   ./kruskal pequeno               Teste 1, passo a passo
//   ./kruskal medio                 Teste 2
//   ./kruskal grande                Teste 3 (desempenho)
//   ./kruskal verificacao           Kruskal x Prim em 1000 grafos aleatórios
//   ./kruskal escala                tempo para tamanhos crescentes
//   ./kruskal otimizacao            Union-Find otimizado x ingênuo, etc.
//   ./kruskal densidade             grafos esparsos x densos (Kruskal x Prim)
//   ./kruskal todos                 tudo acima, em sequência
//   ./kruskal arquivo CAMINHO       roda num grafo lido de arquivo
//   ./kruskal aleatorio N M [SEED]  roda num grafo aleatório

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX   // impede o windows.h de definir as macros min e max
#endif
#include <windows.h>
#endif

#include "experimento.hpp"
#include "grafos.hpp"
#include "kruskal.hpp"
#include "prim.hpp"

using std::int64_t;
using std::string;
using std::vector;

// ------------------------------------------------------------- utilitários

static void titulo(const string& texto) {
    std::printf("\n%s\n%s\n%s\n", string(72, '=').c_str(), texto.c_str(),
                string(72, '=').c_str());
}

static string nomeVertice(int v, const vector<string>& rotulos) {
    return rotulos.empty() ? std::to_string(v) : rotulos[v];
}

static double mediana(vector<double> v) {
    std::sort(v.begin(), v.end());
    size_t k = v.size();
    return k % 2 ? v[k / 2] : (v[k / 2 - 1] + v[k / 2]) / 2;
}

static double desvio(const vector<double>& v) {
    if (v.size() < 2) return 0;
    double media = 0;
    for (double x : v) media += x;
    media /= v.size();
    double s = 0;
    for (double x : v) s += (x - media) * (x - media);
    return std::sqrt(s / (v.size() - 1));
}

static void mostrarTempos(const char* rotulo, const vector<double>& t) {
    std::printf("%-28s: mediana %.3f ms | min %.3f | max %.3f | desvio %.3f (%zu execucoes)\n",
                rotulo, mediana(t), *std::min_element(t.begin(), t.end()),
                *std::max_element(t.begin(), t.end()), desvio(t), t.size());
}

static void mostrarPassos(const ResultadoKruskal& r, const vector<string>& rotulos,
                          size_t limite = SIZE_MAX) {
    std::printf("%5s  %-14s %6s  %-17s %14s\n", "Passo", "Aresta", "Peso",
                "Decisao", "Peso acumulado");
    for (size_t i = 0; i < r.passos.size() && i < limite; ++i) {
        const Passo& p = r.passos[i];
        string aresta = nomeVertice(p.aresta.u, rotulos) + "-" +
                        nomeVertice(p.aresta.v, rotulos);
        std::printf("%5zu  %-14s %6d  %-17s %14lld\n", i + 1, aresta.c_str(),
                    p.aresta.peso, p.aceita ? "ACEITA" : "DESCARTA (ciclo)",
                    static_cast<long long>(p.pesoAcumulado));
    }
    if (r.passos.size() > limite)
        std::printf("  ... (%zu passos omitidos)\n", r.passos.size() - limite);
}

static void mostrarResumo(int n, int64_t m, const ResultadoKruskal& r) {
    std::printf("Vertices (n)                : %d\n", n);
    std::printf("Arestas (m)                 : %lld\n", (long long)m);
    std::printf("Arestas na AGM              : %zu\n", r.arvore.size());
    std::printf("Peso total da AGM           : %lld\n", (long long)r.pesoTotal);
    std::printf("Arestas examinadas          : %lld (%.1f%% do total)\n",
                (long long)r.examinadas, m ? 100.0 * r.examinadas / m : 0.0);
    std::printf("Arestas descartadas (ciclo) : %lld\n", (long long)r.descartadas);
    std::printf("Comparacoes na ordenacao    : %lld\n", (long long)r.comparacoesOrdenacao);
    std::printf("Chamadas de find            : %lld\n", (long long)r.chamadasFind);
    std::printf("Tempo ordenacao             : %.3f ms\n", r.msOrdenacao);
    std::printf("Tempo selecao gulosa        : %.3f ms\n", r.msSelecao);
    std::printf("Tempo total                 : %.3f ms\n", r.msTotal());
}

// Executa o Kruskal 'rep' vezes e devolve os tempos (ordenação, seleção, total)
struct Medicao {
    ResultadoKruskal ultimo;
    vector<double> ordenacao, selecao, total;
};

static Medicao medir(int n, const vector<Aresta>& arestas, int rep,
                     const OpcoesKruskal& op = {}) {
    Medicao med;
    for (int i = 0; i < rep; ++i) {
        med.ultimo = kruskal(n, arestas, op);
        med.ordenacao.push_back(med.ultimo.msOrdenacao);
        med.selecao.push_back(med.ultimo.msSelecao);
        med.total.push_back(med.ultimo.msTotal());
    }
    return med;
}

static double cronometrar(const std::function<void()>& f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// Checagem independente: n-1 arestas e todos os vértices alcançáveis
// implica árvore geradora (conexa e sem ciclos).
static bool ehArvoreGeradora(int n, const vector<Aresta>& arvore) {
    if (n == 0) return arvore.empty();
    if (static_cast<int>(arvore.size()) != n - 1) return false;
    vector<vector<int>> adj(n);
    for (const Aresta& a : arvore) {
        adj[a.u].push_back(a.v);
        adj[a.v].push_back(a.u);
    }
    vector<char> visitado(n, 0);
    vector<int> pilha{0};
    visitado[0] = 1;
    int alcancados = 1;
    while (!pilha.empty()) {
        int x = pilha.back();
        pilha.pop_back();
        for (int y : adj[x])
            if (!visitado[y]) { visitado[y] = 1; ++alcancados; pilha.push_back(y); }
    }
    return alcancados == n;
}

static string caminhoEntrada(const string& arquivo) {
    // Funciona rodando de dentro da pasta do projeto ou da pasta build/
    for (const char* prefixo : {"entradas/", "../entradas/"}) {
        string c = string(prefixo) + arquivo;
        if (std::ifstream(c)) return c;
    }
    return "entradas/" + arquivo;
}

// ------------------------------------------------------------------ testes

static void testePequeno() {
    titulo("TESTE 1 - Pequena instancia (passo a passo)");
    GrafoArquivo g = lerArquivo(caminhoEntrada("pequeno.txt"));
    std::printf("Entrada (ordem do arquivo):\n");
    for (const Aresta& a : g.arestas)
        std::printf("  %s-%s  peso %d\n", g.rotulos[a.u].c_str(),
                    g.rotulos[a.v].c_str(), a.peso);

    OpcoesKruskal op;
    op.registrarPassos = true;
    ResultadoKruskal r = kruskal(g.n, g.arestas, op);

    std::printf("\nDecisoes do algoritmo (arestas ja ordenadas por peso):\n");
    mostrarPassos(r, g.rotulos);
    std::printf("\nArvore Geradora Minima:\n");
    for (const Aresta& a : r.arvore)
        std::printf("  %s-%s  (%d)\n", g.rotulos[a.u].c_str(),
                    g.rotulos[a.v].c_str(), a.peso);
    std::printf("\n");
    mostrarResumo(g.n, g.arestas.size(), r);
    std::printf("Conferencia com Prim        : %lld\n",
                (long long)primPesoTotal(g.n, g.arestas));
}

static void testeMedio() {
    titulo("TESTE 2 - Media instancia (n=1.000, m=10.000, seed=7)");
    const int n = 1000;
    const int64_t m = 10000;
    vector<Aresta> arestas = gerarGrafoConexo(n, m, 7);

    OpcoesKruskal op;
    op.registrarPassos = true;
    ResultadoKruskal r = kruskal(n, arestas, op);
    std::printf("Primeiras 20 decisoes:\n");
    mostrarPassos(r, {}, 20);

    // Mostra também as primeiras decisões de descarte
    std::printf("\nPrimeiros 5 descartes:\n");
    int mostrados = 0;
    for (const Passo& p : r.passos)
        if (!p.aceita && mostrados < 5) {
            std::printf("  %d-%d peso %d (u e v ja estavam conectados)\n",
                        p.aresta.u, p.aresta.v, p.aresta.peso);
            ++mostrados;
        }

    size_t aceitas = 0;
    for (const Passo& p : r.passos) aceitas += p.aceita;
    std::printf("\nTotal de decisoes: %zu (%zu aceitas, %zu descartadas)\n",
                r.passos.size(), aceitas, r.passos.size() - aceitas);
    std::printf("Aresta mais pesada da AGM: %d (peso maximo possivel: 1000)\n\n",
                r.arvore.back().peso);
    mostrarResumo(n, m, r);
    std::printf("Conferencia com Prim        : %lld\n",
                (long long)primPesoTotal(n, arestas));
    Medicao med = medir(n, arestas, 10);
    mostrarTempos("Tempo total (10 execucoes)", med.total);
}

static void testeGrande() {
    titulo("TESTE 3 - Grande instancia (n=1.000.000, m=10.000.000, seed=2026)");
    const int n = 1000000;
    const int64_t m = 10000000;
    vector<Aresta> arestas;
    double tg = cronometrar([&] { arestas = gerarGrafoConexo(n, m, 2026); });
    std::printf("Geracao do grafo: %.0f ms (nao entra na medicao)\n", tg);
    Medicao med = medir(n, arestas, 5);
    mostrarResumo(n, m, med.ultimo);
    mostrarTempos("Ordenacao", med.ordenacao);
    mostrarTempos("Selecao gulosa", med.selecao);
    mostrarTempos("Total", med.total);
    int64_t pesoPrim = 0;
    double tp = cronometrar([&] { pesoPrim = primPesoTotal(n, arestas); });
    std::printf("Conferencia com Prim        : %lld (Prim levou %.0f ms)\n",
                (long long)pesoPrim, tp);
}

static void verificacao() {
    const int quantidade = 1000;
    titulo("VERIFICACAO - Kruskal x Prim em 1000 grafos aleatorios");
    Sorteador rng(123);
    int falhas = 0, comEmpates = 0;
    for (int i = 0; i < quantidade; ++i) {
        int n = static_cast<int>(rng.entre(1, 80));
        int64_t maxM = static_cast<int64_t>(n) * (n - 1) / 2;
        int64_t m = n > 1 ? rng.entre(n - 1, maxM) : 0;
        // Metade dos grafos com pesos de 1 a 10: muitos empates
        int pesoMax = (i % 2 == 0) ? 10 : 1000;
        if (pesoMax == 10) ++comEmpates;
        vector<Aresta> arestas = gerarGrafoConexo(n, m, i, 1, pesoMax);
        ResultadoKruskal r = kruskal(n, arestas);
        bool ok = r.pesoTotal == primPesoTotal(n, arestas) &&
                  ehArvoreGeradora(n, r.arvore);
        if (!ok) {
            ++falhas;
            std::printf("  FALHA no grafo %d: n=%d m=%lld\n", i, n, (long long)m);
        }
    }
    std::printf("Grafos testados: %d (%d com muitos pesos empatados) | falhas: %d\n",
                quantidade, comEmpates, falhas);
    std::printf("Cada caso confere: mesmo peso total que o Prim, n-1 arestas,\n"
                "e todos os vertices alcancaveis pela arvore (logo, sem ciclos).\n");

    // Caso de erro: grafo desconexo deve ser recusado
    try {
        kruskal(4, {{1, 0, 1}, {2, 2, 3}});
        std::printf("Grafo desconexo: NAO detectado (erro!)\n");
    } catch (const std::exception& e) {
        std::printf("Grafo desconexo detectado corretamente: %s\n", e.what());
    }
    // Casos-limite
    std::printf("Grafo com 1 vertice: peso %lld, %zu arestas\n",
                (long long)kruskal(1, {}).pesoTotal, kruskal(1, {}).arvore.size());
    ResultadoKruskal iguais = kruskal(4, {{5, 0, 1}, {5, 1, 2}, {5, 2, 3}, {5, 3, 0}, {5, 0, 2}});
    std::printf("Todos os pesos iguais (n=4): peso %lld (esperado 15)\n",
                (long long)iguais.pesoTotal);
}

static void escala() {
    titulo("ESCALA - tempo x tamanho (grafos com m = 10n, mediana de 5 execucoes)");
    std::printf("%10s %11s %13s %11s %10s %12s %11s %10s\n", "n", "m",
                "ordenacao ms", "selecao ms", "total ms", "ns/aresta",
                "examinadas", "% exam.");
    for (int n : {1000, 10000, 100000, 1000000}) {
        int64_t m = 10LL * n;
        vector<Aresta> arestas = gerarGrafoConexo(n, m, n);
        Medicao med = medir(n, arestas, 5);
        double tot = mediana(med.total);
        std::printf("%10d %11lld %13.3f %11.3f %10.3f %12.2f %11lld %9.1f%%\n", n,
                    (long long)m, mediana(med.ordenacao), mediana(med.selecao),
                    tot, tot * 1e6 / m, (long long)med.ultimo.examinadas,
                    100.0 * med.ultimo.examinadas / m);
    }
}

static void otimizacao() {
    titulo("OTIMIZACAO 1 - Union-Find otimizado x ingenuo (caso adverso)");
    std::printf("Grafo 'estrela': toda aresta liga ao vertice 0 e o peso cresce\n"
                "com o indice. Sem rank/compressao, a arvore do Union-Find vira\n"
                "uma lista e cada find percorre todos os vertices ja unidos.\n\n");
    std::printf("%8s %22s %20s %10s\n", "n", "selecao otimizada ms",
                "selecao ingenua ms", "razao");
    for (int n : {5000, 10000, 20000, 40000}) {
        vector<Aresta> estrela;
        for (int i = 1; i < n; ++i) estrela.push_back({i, i, 0});
        OpcoesKruskal ingenuo;
        ingenuo.unionFindOtimizado = false;
        double rapido = mediana(medir(n, estrela, 3).selecao);
        double lento = mediana(medir(n, estrela, 3, ingenuo).selecao);
        std::printf("%8d %22.3f %20.3f %9.0fx\n", n, rapido, lento, lento / rapido);
    }

    std::printf("\nCaso tipico: grafos aleatorios com m = 10n\n");
    std::printf("%8s %10s %22s %20s %10s\n", "n", "m", "selecao otimizada ms",
                "selecao ingenua ms", "razao");
    for (int nt : {5000, 10000, 20000, 40000}) {
        vector<Aresta> g = gerarGrafoConexo(nt, 10LL * nt, 99);
        OpcoesKruskal ingenuo;
        ingenuo.unionFindOtimizado = false;
        double rapido = mediana(medir(nt, g, 3).selecao);
        double lento = mediana(medir(nt, g, 3, ingenuo).selecao);
        std::printf("%8d %10lld %22.3f %20.3f %9.0fx\n", nt, 10LL * nt, rapido,
                    lento, lento / rapido);
    }

    const int n = 200000;
    const int64_t m = 2000000;
    vector<Aresta> arestas = gerarGrafoConexo(n, m, 99);

    titulo("OTIMIZACAO 2 - Parada antecipada (n=200.000, m=2.000.000)");
    for (bool usar : {true, false}) {
        OpcoesKruskal op;
        op.paradaAntecipada = usar;
        Medicao med = medir(n, arestas, 5, op);
        std::printf("  %s: examinou %8lld arestas, selecao %.3f ms\n",
                    usar ? "com parada" : "sem parada",
                    (long long)med.ultimo.examinadas, mediana(med.selecao));
    }

    titulo("OTIMIZACAO 3 - Merge Sort proprio x std::sort (m=2.000.000, so referencia)");
    vector<double> tMerge, tStd;
    for (int i = 0; i < 5; ++i) {
        vector<Aresta> copia = arestas;
        tMerge.push_back(cronometrar([&] { mergeSort(copia); }));
        copia = arestas;
        tStd.push_back(cronometrar([&] {
            std::sort(copia.begin(), copia.end(),
                      [](const Aresta& a, const Aresta& b) { return a.peso < b.peso; });
        }));
    }
    std::printf("  Merge Sort proprio: %.3f ms (mediana)\n", mediana(tMerge));
    std::printf("  std::sort (introsort): %.3f ms (mediana)\n", mediana(tStd));
}

static void densidade() {
    titulo("DENSIDADE - mesmo n=2.000, numero de arestas crescente");
    const int n = 2000;
    std::printf("%10s %10s %12s %10s %12s %9s\n", "m", "densidade",
                "Kruskal ms", "Prim ms", "examinadas", "% exam.");
    for (int64_t m : {2000LL, 10000LL, 50000LL, 200000LL, 1000000LL, 1999000LL}) {
        vector<Aresta> arestas = gerarGrafoConexo(n, m, static_cast<uint64_t>(m));
        Medicao med = medir(n, arestas, 5);
        vector<double> tp;
        for (int i = 0; i < 5; ++i)
            tp.push_back(cronometrar([&] { primPesoTotal(n, arestas); }));
        double dens = m / (n * (n - 1) / 2.0);
        std::printf("%10lld %9.1f%% %12.3f %10.3f %12lld %8.1f%%\n", (long long)m,
                    100 * dens, mediana(med.total), mediana(tp),
                    (long long)med.ultimo.examinadas,
                    100.0 * med.ultimo.examinadas / m);
    }
}

static void hipotese() {
    titulo("HIPOTESE - por que o Union-Find ingenuo e lento no grafo aleatorio?");
    std::printf("Hipotese: sem rank, a uniao pendura uma raiz na outra sem olhar o\n"
                "tamanho. A componente gigante acaba pendurada sob vertices isolados\n"
                "e ganha um nivel de profundidade a cada uniao; cada find precisa\n"
                "subir esses niveis. Tempos com instrumentacao (contadores ligados).\n");
    const vector<VarianteUF> variantes = {
        {"ingenuo (v sob u)", false, false, false},
        {"ingenuo invertido (u sob v)", false, false, true},
        {"so uniao por rank", true, false, false},
        {"so compressao", false, true, false},
        {"otimizado (rank + compressao)", true, true, false},
    };
    auto rodar = [&](const char* nome, int n, vector<Aresta> arestas) {
        mergeSort(arestas);
        std::printf("\n%s\n", nome);
        std::printf("%-30s %11s %9s %13s %9s %16s\n", "variante", "tempo ms",
                    "finds", "passos/find", "altura", "maior sob menor");
        for (const VarianteUF& v : variantes) {
            vector<double> tempos;
            ResultadoExperimento r{};
            for (int i = 0; i < 3; ++i) {
                r = rodarVariante(n, arestas, v);
                tempos.push_back(r.ms);
            }
            std::printf("%-30s %11.3f %9lld %13.2f %9d %15.1f%%\n", v.nome.c_str(),
                        mediana(tempos), (long long)r.chamadas,
                        (double)r.passos / r.chamadas, r.altura,
                        100.0 * r.maiorSobMenor / r.unioes);
        }
    };
    const int n = 20000;
    rodar("Grafo aleatorio: n=20.000, m=200.000 (mesmo da OTIMIZACAO 1)", n,
          gerarGrafoConexo(n, 10LL * n, 99));
    vector<Aresta> estrela;
    for (int i = 1; i < n; ++i) estrela.push_back({i, i, 0});
    rodar("Grafo estrela: n=20.000 (caso adverso da OTIMIZACAO 1)", n, estrela);
}

static void rodarArquivo(const string& caminho) {
    titulo("Grafo lido de " + caminho);
    GrafoArquivo g = lerArquivo(caminho);
    OpcoesKruskal op;
    op.registrarPassos = g.arestas.size() <= 100;
    ResultadoKruskal r = kruskal(g.n, g.arestas, op);
    if (op.registrarPassos) mostrarPassos(r, g.rotulos);
    std::printf("\nArestas da AGM:\n");
    for (const Aresta& a : r.arvore)
        std::printf("  %s-%s  (%d)\n", g.rotulos[a.u].c_str(),
                    g.rotulos[a.v].c_str(), a.peso);
    mostrarResumo(g.n, g.arestas.size(), r);
}

static void rodarAleatorio(int n, int64_t m, uint64_t semente) {
    titulo("Grafo aleatorio n=" + std::to_string(n) + ", m=" + std::to_string(m) +
           ", seed=" + std::to_string(semente));
    vector<Aresta> arestas = gerarGrafoConexo(n, m, semente);
    OpcoesKruskal op;
    op.registrarPassos = m <= 100;
    ResultadoKruskal r = kruskal(n, arestas, op);
    if (op.registrarPassos) mostrarPassos(r, {});
    mostrarResumo(n, m, r);
}

static void ambiente() {
#if defined(__clang__)
    std::printf("Compilador: clang %s", __clang_version__);
#elif defined(__GNUC__)
    std::printf("Compilador: g++ %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    std::printf("Compilador: MSVC %d", _MSC_VER);
#endif
    std::printf(" | C++ padrao %ld\n", static_cast<long>(__cplusplus));
}

static void uso() {
    std::printf(
        "Uso:\n"
        "  kruskal pequeno | medio | grande | verificacao | escala |\n"
        "          otimizacao | densidade | todos\n"
        "  kruskal arquivo CAMINHO\n"
        "  kruskal aleatorio N M [SEED]\n");
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    string modo = argc > 1 ? argv[1] : "todos";
    const std::map<string, std::function<void()>> testes = {
        {"pequeno", testePequeno}, {"medio", testeMedio},
        {"grande", testeGrande},   {"verificacao", verificacao},
        {"escala", escala},        {"otimizacao", otimizacao},
        {"densidade", densidade},  {"hipotese", hipotese},
    };
    try {
        ambiente();
        if (modo == "todos") {
            for (const char* t : {"pequeno", "medio", "grande", "verificacao",
                                  "escala", "otimizacao", "densidade", "hipotese"})
                testes.at(t)();
        } else if (modo == "arquivo" && argc > 2) {
            rodarArquivo(argv[2]);
        } else if (modo == "aleatorio" && argc > 3) {
            rodarAleatorio(std::stoi(argv[2]), std::stoll(argv[3]),
                           argc > 4 ? std::stoull(argv[4]) : 42);
        } else if (testes.count(modo)) {
            testes.at(modo)();
        } else {
            uso();
            return 1;
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Erro: %s\n", e.what());
        return 1;
    }
    return 0;
}
