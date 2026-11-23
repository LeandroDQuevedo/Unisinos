// main_aula11.cpp
// Aula 11 - Divisão e Conquista aplicada à Árvore Geradora Mínima.
// Compara, sobre as MESMAS entradas das Aulas 9 e 10:
//   - Kruskal guloso (Aula 9, Merge Sort iterativo + Union-Find);
//   - Programação Dinâmica por subconjuntos (Aula 10);
//   - Divisão e Conquista sobre as arestas (dc_arestas.hpp);
//   - Kruskal com Merge Sort recursivo (D&C só na ordenação).
//
// Uso:
//   ./aula11 pequeno       Teste 1: 7 vértices, árvore de recursão completa
//   ./aula11 medio         Teste 2: n=1.000, m=10.000
//   ./aula11 grande        Teste 3: n=1.000.000, m=10.000.000
//   ./aula11 tres          os três paradigmas nos grafos da Aula 10 (7, 14, 20)
//   ./aula11 verificacao   1.000 grafos aleatórios + casos-limite
//   ./aula11 escala        tempo x tamanho (m = 10n)
//   ./aula11 densidade     n = 2.000, de esparso a completo
//   ./aula11 otimizacao    caso base ampliado e execução paralela
//   ./aula11 todos         tudo acima, em sequência
//   ./aula11 arquivo CAMINHO | aleatorio N M [SEED]

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "dc_arestas.hpp"
#include "dp_subconjuntos.hpp"
#include "grafos.hpp"
#include "kruskal.hpp"
#include "mergesort_recursivo.hpp"
#include "prim.hpp"

using std::int64_t;
using std::string;
using std::vector;

// ------------------------------------------------------------- utilitários

static void titulo(const string& texto) {
    std::printf("\n%s\n%s\n%s\n", string(76, '=').c_str(), texto.c_str(),
                string(76, '=').c_str());
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
    std::printf("%-34s: mediana %.6f ms | min %.6f | max %.6f | desvio %.6f (%zu exec.)\n",
                rotulo, mediana(t), *std::min_element(t.begin(), t.end()),
                *std::max_element(t.begin(), t.end()), desvio(t), t.size());
}

static double cronometrar(const std::function<void()>& f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

static string nomeVertice(int v, const vector<string>& rotulos) {
    return rotulos.empty() ? std::to_string(v) : rotulos[v];
}

static string textoAresta(const Aresta& a, const vector<string>& rotulos) {
    return nomeVertice(a.u, rotulos) + "-" + nomeVertice(a.v, rotulos) + "(" +
           std::to_string(a.peso) + ")";
}

static string textoLista(const vector<Aresta>& v, const vector<string>& rotulos) {
    if (v.empty()) return "{}";
    string s = "{";
    for (size_t i = 0; i < v.size(); ++i) s += (i ? ", " : "") + textoAresta(v[i], rotulos);
    return s + "}";
}

// n-1 arestas e todos os vértices alcançáveis => árvore geradora.
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

static bool mesmasArestas(const vector<Aresta>& a, const vector<Aresta>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].peso != b[i].peso || a[i].u != b[i].u || a[i].v != b[i].v) return false;
    return true;
}

static string caminhoEntrada(const string& arquivo) {
    for (const char* prefixo : {"entradas/", "../entradas/"}) {
        string c = string(prefixo) + arquivo;
        if (std::ifstream(c)) return c;
    }
    return "entradas/" + arquivo;
}

// Memória auxiliar do Kruskal: cópia da entrada + vetor auxiliar do Merge
// Sort + vetores pai e rank do Union-Find.
static int64_t bytesKruskal(int n, size_t m) {
    return static_cast<int64_t>(2 * m * sizeof(Aresta) + 2 * static_cast<size_t>(n) * sizeof(int));
}

static string textoBytes(int64_t b) {
    char buf[64];
    if (b < 1024) std::snprintf(buf, sizeof buf, "%lld B", (long long)b);
    else if (b < 1024 * 1024) std::snprintf(buf, sizeof buf, "%.1f KB", b / 1024.0);
    else std::snprintf(buf, sizeof buf, "%.1f MB", b / (1024.0 * 1024.0));
    return buf;
}

// Tempos (mediana de 'rep' execuções) dos algoritmos sobre a mesma entrada.
struct Tempos {
    vector<double> kruskal, kruskalOrd, kruskalSel;
    vector<double> kruskalMS, kruskalMSOrd;
    vector<double> dc;        // D&C com caso base de 1 aresta
    vector<double> dcBloco;   // D&C com caso base ampliado (blocos de n arestas)
    ResultadoDC rdcBloco;
    ResultadoKruskal rk;
    ResultadoKruskalMS rms;
    ResultadoDC rdc;
};

static Tempos medirTodos(int n, const vector<Aresta>& arestas, int rep,
                         const OpcoesDC& op = {}) {
    Tempos t;
    for (int i = 0; i < rep; ++i) {
        t.rk = kruskal(n, arestas);
        t.kruskal.push_back(t.rk.msTotal());
        t.kruskalOrd.push_back(t.rk.msOrdenacao);
        t.kruskalSel.push_back(t.rk.msSelecao);
        t.rms = kruskalMergeRecursivo(n, arestas);
        t.kruskalMS.push_back(t.rms.kruskal.msTotal());
        t.kruskalMSOrd.push_back(t.rms.kruskal.msOrdenacao);
        t.rdc = agmDivisaoConquista(n, arestas, op);
        t.dc.push_back(t.rdc.ms);
        OpcoesDC bloco = op;
        bloco.limiteCasoBase = n;
        t.rdcBloco = agmDivisaoConquista(n, arestas, bloco);
        t.dcBloco.push_back(t.rdcBloco.ms);
    }
    return t;
}

static void mostrarMetricasDC(int n, int64_t m, const ResultadoDC& r) {
    std::printf("Vertices (n) / arestas (m)       : %d / %lld\n", n, (long long)m);
    std::printf("Arestas na AGM / peso total      : %zu / %lld\n", r.arvore.size(),
                (long long)r.pesoTotal);
    std::printf("Divisoes realizadas              : %lld\n", (long long)r.divisoes);
    std::printf("Chamadas recursivas              : %lld\n", (long long)r.chamadas);
    std::printf("Casos base atingidos             : %lld\n", (long long)r.casosBase);
    std::printf("Profundidade da recursao         : %lld (teto de log2 m = %d)\n",
                (long long)r.profundidade,
                m > 1 ? (int)std::ceil(std::log2((double)m)) : 0);
    std::printf("Arestas que entraram em combinacoes: %lld\n", (long long)r.arestasCombinadas);
    std::printf("Comparacoes de peso (intercalacao): %lld\n", (long long)r.comparacoes);
    std::printf("Arestas descartadas (ciclo)      : %lld\n", (long long)r.descartadas);
    std::printf("Chamadas de find                 : %lld\n", (long long)r.chamadasFind);
    std::printf("Memoria auxiliar                 : %s\n", textoBytes(r.bytesAuxiliares).c_str());
}

static void mostrarPerfilNiveis(const ResultadoDC& r, int64_t m) {
    std::printf("\nPerfil por nivel da arvore de recursao:\n");
    std::printf("%6s %10s %14s %16s %16s %10s\n", "nivel", "chamadas",
                "tam. do bloco", "arestas combin.", "arestas devolv.", "% de m");
    for (size_t d = 0; d < r.chamadasPorNivel.size(); ++d)
        std::printf("%6zu %10lld %14.1f %16lld %16lld %9.1f%%\n", d,
                    (long long)r.chamadasPorNivel[d],
                    (double)m / std::pow(2.0, (double)d),
                    (long long)r.combinadasPorNivel[d],
                    (long long)r.devolvidasPorNivel[d],
                    m ? 100.0 * r.combinadasPorNivel[d] / m : 0.0);
}

// ------------------------------------------------------------------ testes

static void imprimirArvoreRecursao(const ResultadoDC& r, const vector<string>& rot) {
    // Sequência das chamadas (pré-ordem = ordem em que foram feitas)
    for (const NoRecursao& no : r.nos) {
        string recuo(static_cast<size_t>(no.profundidade) * 2, ' ');
        std::printf("%s#%d [%zu,%zu) %s  entrada=%s\n", recuo.c_str(), no.id,
                    no.inicio, no.fim, no.casoBase ? "CASO BASE" : "DIVIDE",
                    textoLista(no.entrada, rot).c_str());
    }
    std::printf("\nRetorno das solucoes e combinacoes (de baixo para cima):\n");
    // pós-ordem: um nó só combina depois dos seus dois filhos
    std::function<void(int)> visitar = [&](int id) {
        for (const NoRecursao& f : r.nos)
            if (f.pai == id) visitar(f.id);
        const NoRecursao& no = r.nos[id];
        if (no.casoBase) return;
        std::printf("  #%d combina -> devolve %s", no.id, textoLista(no.saida, rot).c_str());
        if (!no.descartadas.empty())
            std::printf("  | descarta %s", textoLista(no.descartadas, rot).c_str());
        std::printf("\n");
    };
    visitar(0);
}

// Grava a árvore de recursão em JSON (usada para desenhar a figura).
static void gravarArvoreJson(const ResultadoDC& r, const vector<string>& rot,
                             const string& caminho) {
    std::ofstream out(caminho);
    if (!out) return;
    auto lista = [&](const vector<Aresta>& v) {
        string s = "[";
        for (size_t i = 0; i < v.size(); ++i)
            s += string(i ? "," : "") + "\"" + textoAresta(v[i], rot) + "\"";
        return s + "]";
    };
    out << "[\n";
    for (size_t i = 0; i < r.nos.size(); ++i) {
        const NoRecursao& no = r.nos[i];
        out << "  {\"id\":" << no.id << ",\"pai\":" << no.pai << ",\"prof\":"
            << no.profundidade << ",\"inicio\":" << no.inicio << ",\"fim\":" << no.fim
            << ",\"base\":" << (no.casoBase ? "true" : "false")
            << ",\"entrada\":" << lista(no.entrada) << ",\"saida\":" << lista(no.saida)
            << ",\"descartadas\":" << lista(no.descartadas) << "}"
            << (i + 1 < r.nos.size() ? "," : "") << "\n";
    }
    out << "]\n";
}

static void testePequeno() {
    titulo("TESTE 1 - Pequena instancia (7 vertices, 11 arestas): passo a passo");
    GrafoArquivo g = lerArquivo(caminhoEntrada("pequeno.txt"));
    std::printf("Entrada (ordem do arquivo, posicoes 0 a %zu):\n", g.arestas.size() - 1);
    for (size_t i = 0; i < g.arestas.size(); ++i)
        std::printf("  [%2zu] %s\n", i, textoAresta(g.arestas[i], g.rotulos).c_str());

    OpcoesDC op;
    op.registrarArvore = true;
    ResultadoDC r = agmDivisaoConquista(g.n, g.arestas, op);

    std::printf("\nSequencia das chamadas recursivas (recuo = profundidade):\n");
    imprimirArvoreRecursao(r, g.rotulos);
    gravarArvoreJson(r, g.rotulos, "resultados/arvore_pequeno.json");

    std::printf("\nArvore Geradora Minima (Divisao e Conquista): %s\n\n",
                textoLista(r.arvore, g.rotulos).c_str());
    mostrarMetricasDC(g.n, g.arestas.size(), r);
    mostrarPerfilNiveis(r, g.arestas.size());

    ResultadoKruskal rk = kruskal(g.n, g.arestas);
    ResultadoPD rpd = agmProgramacaoDinamica(g.n, g.arestas);
    ResultadoKruskalMS rms = kruskalMergeRecursivo(g.n, g.arestas);
    std::printf("\nConferencia: Kruskal %lld | PD %lld | Kruskal+MergeSort recursivo %lld | Prim %lld\n",
                (long long)rk.pesoTotal, (long long)rpd.pesoTotal,
                (long long)rms.kruskal.pesoTotal, (long long)primPesoTotal(g.n, g.arestas));
    std::printf("Mesmas arestas, na mesma ordem, que o Kruskal: %s\n",
                mesmasArestas(r.arvore, rk.arvore) ? "sim" : "NAO");
    std::printf("Merge Sort recursivo: %lld chamadas, %lld divisoes, profundidade %lld, %lld comparacoes\n",
                (long long)rms.ordenacao.chamadas, (long long)rms.ordenacao.divisoes,
                (long long)rms.ordenacao.profundidade, (long long)rms.ordenacao.comparacoes);
}

static void relatorioInstancia(int n, const vector<Aresta>& arestas, int rep) {
    const int64_t m = static_cast<int64_t>(arestas.size());
    Tempos t = medirTodos(n, arestas, rep);
    mostrarMetricasDC(n, m, t.rdc);
    mostrarPerfilNiveis(t.rdc, m);

    std::printf("\nKruskal guloso (Aula 9): peso %lld, %lld examinadas, %lld descartadas, "
                "%lld comparacoes, %lld finds, memoria %s\n",
                (long long)t.rk.pesoTotal, (long long)t.rk.examinadas,
                (long long)t.rk.descartadas, (long long)t.rk.comparacoesOrdenacao,
                (long long)t.rk.chamadasFind, textoBytes(bytesKruskal(n, arestas.size())).c_str());
    std::printf("Merge Sort recursivo: %lld chamadas, %lld divisoes, profundidade %lld, %lld comparacoes\n",
                (long long)t.rms.ordenacao.chamadas, (long long)t.rms.ordenacao.divisoes,
                (long long)t.rms.ordenacao.profundidade, (long long)t.rms.ordenacao.comparacoes);
    std::printf("Mesmas arestas, na mesma ordem, que o Kruskal: %s\n\n",
                mesmasArestas(t.rdc.arvore, t.rk.arvore) ? "sim" : "NAO");

    mostrarTempos("Kruskal guloso - total", t.kruskal);
    mostrarTempos("  ordenacao (Merge Sort iterativo)", t.kruskalOrd);
    mostrarTempos("  selecao gulosa", t.kruskalSel);
    mostrarTempos("Kruskal + Merge Sort recursivo", t.kruskalMS);
    mostrarTempos("  ordenacao (Merge Sort recursivo)", t.kruskalMSOrd);
    mostrarTempos("Divisao e Conquista nas arestas", t.dc);
    mostrarTempos("D&C com caso base de n arestas", t.dcBloco);
    std::printf("  (caso base de n arestas: %lld chamadas, %lld divisoes, profundidade %lld, "
                "%lld comparacoes, %lld finds, peso %lld)\n",
                (long long)t.rdcBloco.chamadas, (long long)t.rdcBloco.divisoes,
                (long long)t.rdcBloco.profundidade, (long long)t.rdcBloco.comparacoes,
                (long long)t.rdcBloco.chamadasFind, (long long)t.rdcBloco.pesoTotal);
    std::printf("Razao D&C / Kruskal: %.2fx | D&C (base n) / Kruskal: %.2fx | "
                "Merge recursivo / iterativo: %.2fx\n",
                mediana(t.dc) / mediana(t.kruskal),
                mediana(t.dcBloco) / mediana(t.kruskal),
                mediana(t.kruskalMSOrd) / mediana(t.kruskalOrd));
}

static void testeMedio() {
    titulo("TESTE 2 - Media instancia (n=1.000, m=10.000, seed=7)");
    const int n = 1000;
    vector<Aresta> arestas = gerarGrafoConexo(n, 10000, 7);
    relatorioInstancia(n, arestas, 10);
    std::printf("Conferencia com Prim: %lld\n", (long long)primPesoTotal(n, arestas));
}

static void testeGrande() {
    titulo("TESTE 3 - Grande instancia (n=1.000.000, m=10.000.000, seed=2026)");
    const int n = 1000000;
    vector<Aresta> arestas;
    double tg = cronometrar([&] { arestas = gerarGrafoConexo(n, 10000000, 2026); });
    std::printf("Geracao do grafo: %.0f ms (nao entra na medicao)\n", tg);
    relatorioInstancia(n, arestas, 5);
    int64_t pesoPrim = 0;
    double tp = cronometrar([&] { pesoPrim = primPesoTotal(n, arestas); });
    std::printf("Conferencia com Prim: %lld (Prim levou %.0f ms)\n", (long long)pesoPrim, tp);
}

// Os três paradigmas sobre os grafos usados na Aula 10 (limitados pela PD).
static void tresParadigmas() {
    titulo("TRES PARADIGMAS - grafos da Aula 10 (7, 14 e 20 vertices)");
    struct Caso { const char* nome; int n; int64_t m; uint64_t semente; int rep; int repPD; };
    const vector<Caso> casos = {
        {"Pequeno (arquivo)", 7, 11, 0, 7, 7},
        {"Medio (seed 7)", 14, 45, 7, 7, 7},
        {"Grande para PD (seed 2026)", 20, 100, 2026, 7, 3},
    };
    for (const Caso& c : casos) {
        vector<Aresta> arestas = c.semente == 0
            ? lerArquivo(caminhoEntrada("pequeno.txt")).arestas
            : gerarGrafoConexo(c.n, c.m, c.semente);
        Tempos t = medirTodos(c.n, arestas, c.rep);
        vector<double> tpd;
        ResultadoPD rpd;
        for (int i = 0; i < c.repPD; ++i) {
            rpd = agmProgramacaoDinamica(c.n, arestas);
            tpd.push_back(rpd.ms);
        }
        std::printf("\n%s: n=%d, m=%lld\n", c.nome, c.n, (long long)c.m);
        std::printf("  Peso: Kruskal %lld | PD %lld | D&C %lld | Kruskal+MS rec. %lld\n",
                    (long long)t.rk.pesoTotal, (long long)rpd.pesoTotal,
                    (long long)t.rdc.pesoTotal, (long long)t.rms.kruskal.pesoTotal);
        std::printf("  Kruskal : %lld arestas examinadas, %lld comparacoes | memoria %s\n",
                    (long long)t.rk.examinadas, (long long)t.rk.comparacoesOrdenacao,
                    textoBytes(bytesKruskal(c.n, arestas.size())).c_str());
        std::printf("  PD      : %lld estados, %lld transicoes, %lld consultas de peso | memoria %s\n",
                    (long long)rpd.estados, (long long)rpd.transicoes,
                    (long long)rpd.consultasPeso, textoBytes(rpd.bytesAuxiliares).c_str());
        std::printf("  D&C     : %lld chamadas, %lld divisoes, profundidade %lld, "
                    "%lld arestas combinadas, %lld comparacoes | memoria %s\n",
                    (long long)t.rdc.chamadas, (long long)t.rdc.divisoes,
                    (long long)t.rdc.profundidade, (long long)t.rdc.arestasCombinadas,
                    (long long)t.rdc.comparacoes, textoBytes(t.rdc.bytesAuxiliares).c_str());
        mostrarTempos("  Kruskal guloso", t.kruskal);
        mostrarTempos("  Programacao Dinamica", tpd);
        mostrarTempos("  Divisao e Conquista", t.dc);
        mostrarTempos("  Kruskal + Merge Sort recursivo", t.kruskalMS);
    }
}

static void verificacao() {
    const int quantidade = 1000;
    titulo("VERIFICACAO - D&C x Kruskal x Prim x PD em 1000 grafos aleatorios");
    Sorteador rng(123);
    int falhas = 0, comEmpates = 0, conferidosPD = 0, arvoresIdenticas = 0;
    for (int i = 0; i < quantidade; ++i) {
        int n = static_cast<int>(rng.entre(1, 80));
        int64_t maxM = static_cast<int64_t>(n) * (n - 1) / 2;
        int64_t m = n > 1 ? rng.entre(n - 1, maxM) : 0;
        int pesoMax = (i % 2 == 0) ? 10 : 1000;   // metade com muitos empates
        if (pesoMax == 10) ++comEmpates;
        vector<Aresta> arestas = gerarGrafoConexo(n, m, i, 1, pesoMax);

        ResultadoKruskal rk = kruskal(n, arestas);
        ResultadoDC rdc = agmDivisaoConquista(n, arestas);
        ResultadoKruskalMS rms = kruskalMergeRecursivo(n, arestas);
        OpcoesDC corte;  corte.limiteCasoBase = (i % 3 == 0) ? 16 : n;
        OpcoesDC par;    par.niveisParalelos = 1;
        ResultadoDC rcorte = agmDivisaoConquista(n, arestas, corte);
        ResultadoDC rpar = agmDivisaoConquista(n, arestas, par);

        bool ok = rdc.pesoTotal == rk.pesoTotal &&
                  rdc.pesoTotal == primPesoTotal(n, arestas) &&
                  ehArvoreGeradora(n, rdc.arvore) &&
                  rms.kruskal.pesoTotal == rk.pesoTotal &&
                  rcorte.pesoTotal == rk.pesoTotal && ehArvoreGeradora(n, rcorte.arvore) &&
                  rpar.pesoTotal == rk.pesoTotal && ehArvoreGeradora(n, rpar.arvore) &&
                  rdc.divisoes == std::max<int64_t>(m - 1, 0) &&
                  rdc.chamadas == std::max<int64_t>(2 * m - 1, 1);
        if (n <= 12) {   // a PD só é viável nos grafos pequenos
            ++conferidosPD;
            ok = ok && agmProgramacaoDinamica(n, arestas).pesoTotal == rdc.pesoTotal;
        }
        if (mesmasArestas(rdc.arvore, rk.arvore)) ++arvoresIdenticas;
        if (!ok) {
            ++falhas;
            std::printf("  FALHA no grafo %d: n=%d m=%lld\n", i, n, (long long)m);
        }
    }
    std::printf("Grafos testados: %d (%d com muitos pesos empatados) | falhas: %d\n",
                quantidade, comEmpates, falhas);
    std::printf("Conferidos tambem com a PD (n <= 12): %d\n", conferidosPD);
    std::printf("D&C devolveu exatamente as mesmas arestas do Kruskal, na mesma ordem: %d de %d\n",
                arvoresIdenticas, quantidade);
    std::printf("Cada caso confere: mesmo peso de Kruskal e Prim, n-1 arestas, todos os\n"
                "vertices alcancaveis, m-1 divisoes e 2m-1 chamadas; variantes com caso\n"
                "base ampliado (16 ou n arestas) e paralela (2 tarefas) com o mesmo peso.\n");

    // Casos-limite
    try {
        agmDivisaoConquista(4, {{1, 0, 1}, {2, 2, 3}});
        std::printf("Grafo desconexo: NAO detectado (erro!)\n");
    } catch (const std::exception& e) {
        std::printf("Grafo desconexo detectado corretamente: %s\n", e.what());
    }
    ResultadoDC um = agmDivisaoConquista(1, {});
    std::printf("Grafo com 1 vertice: peso %lld, %zu arestas, %lld chamada(s)\n",
                (long long)um.pesoTotal, um.arvore.size(), (long long)um.chamadas);
    std::printf("Todos os pesos iguais (n=4): peso %lld (esperado 15)\n",
                (long long)agmDivisaoConquista(4, {{5, 0, 1}, {5, 1, 2}, {5, 2, 3}, {5, 3, 0}, {5, 0, 2}}).pesoTotal);
    std::printf("Pesos negativos (n=4): peso %lld (esperado -9)\n",
                (long long)agmDivisaoConquista(4, {{-4, 0, 1}, {3, 1, 2}, {-5, 2, 3}, {0, 3, 0}, {2, 0, 2}}).pesoTotal);
    std::printf("Arestas paralelas (n=3): peso %lld (esperado 3)\n",
                (long long)agmDivisaoConquista(3, {{9, 0, 1}, {1, 0, 1}, {7, 1, 2}, {2, 1, 2}, {5, 0, 2}}).pesoTotal);
}

static void escala() {
    titulo("ESCALA - tempo x tamanho (grafos com m = 10n, mediana de 5 execucoes)");
    std::printf("%9s %10s | %11s %11s %11s %12s | %8s %8s %8s %8s | %7s %8s\n", "n", "m",
                "Kruskal ms", "Kr+MSrec ms", "D&C ms", "D&C base n", "ns/a Kr",
                "ns/a MS", "ns/a DC", "ns/a DCn", "DC/Kr", "DCn/Kr");
    for (int n : {1000, 10000, 100000, 1000000}) {
        int64_t m = 10LL * n;
        vector<Aresta> arestas = gerarGrafoConexo(n, m, n);
        Tempos t = medirTodos(n, arestas, 5);
        double k = mediana(t.kruskal), s = mediana(t.kruskalMS), d = mediana(t.dc),
               b = mediana(t.dcBloco);
        std::printf("%9d %10lld | %11.3f %11.3f %11.3f %12.3f | %8.1f %8.1f %8.1f %8.1f | %6.2fx %7.2fx\n",
                    n, (long long)m, k, s, d, b, k * 1e6 / m, s * 1e6 / m,
                    d * 1e6 / m, b * 1e6 / m, d / k, b / k);
    }
}

static void densidade() {
    titulo("DENSIDADE - mesmo n=2.000, numero de arestas crescente (mediana de 5)");
    const int n = 2000;
    std::printf("%9s %8s | %10s %10s %11s %9s | %13s %13s | %7s %8s\n", "m", "densid.",
                "Kruskal ms", "D&C ms", "D&C base n", "Prim ms", "compar. Kr.",
                "compar. D&C", "DC/Kr", "DCn/Kr");
    for (int64_t m : {2000LL, 10000LL, 50000LL, 200000LL, 1000000LL, 1999000LL}) {
        vector<Aresta> arestas = gerarGrafoConexo(n, m, static_cast<uint64_t>(m));
        Tempos t = medirTodos(n, arestas, 5);
        vector<double> tp;
        for (int i = 0; i < 5; ++i)
            tp.push_back(cronometrar([&] { primPesoTotal(n, arestas); }));
        double dens = m / (n * (n - 1) / 2.0);
        std::printf("%9lld %7.1f%% | %10.3f %10.3f %11.3f %9.3f | %13lld %13lld | %6.2fx %7.2fx\n",
                    (long long)m, 100 * dens, mediana(t.kruskal), mediana(t.dc),
                    mediana(t.dcBloco), mediana(tp),
                    (long long)t.rk.comparacoesOrdenacao, (long long)t.rdc.comparacoes,
                    mediana(t.dc) / mediana(t.kruskal),
                    mediana(t.dcBloco) / mediana(t.kruskal));
    }
}

static void tabelaCasoBase(const char* nome, int n, const vector<Aresta>& arestas) {
    double tk = mediana(medirTodos(n, arestas, 5).kruskal);
    std::printf("\n%s\nKruskal guloso de referencia: %.3f ms\n", nome, tk);
    std::printf("%12s %12s %8s %14s %14s %10s %12s\n", "limite base", "chamadas",
                "profund.", "comparacoes", "finds", "D&C ms", "D&C/Kruskal");
    for (int64_t limite : {(int64_t)1, (int64_t)16, (int64_t)64, (int64_t)n / 16,
                           (int64_t)n / 4, (int64_t)n, (int64_t)4 * n}) {
        OpcoesDC op;
        op.limiteCasoBase = limite;
        vector<double> t;
        ResultadoDC r;
        for (int i = 0; i < 5; ++i) {
            r = agmDivisaoConquista(n, arestas, op);
            t.push_back(r.ms);
        }
        std::printf("%12lld %12lld %8lld %14lld %14lld %10.3f %11.2fx\n",
                    (long long)limite, (long long)r.chamadas, (long long)r.profundidade,
                    (long long)r.comparacoes, (long long)r.chamadasFind, mediana(t),
                    mediana(t) / tk);
    }
}

static void otimizacao() {
    titulo("OTIMIZACAO 1 - Caso base ampliado (Kruskal local em blocos de ate L arestas)");
    const int n = 200000;
    const int64_t m = 2000000;
    vector<Aresta> arestas = gerarGrafoConexo(n, m, 99);
    tabelaCasoBase("Grafo esparso: n=200.000, m=2.000.000 (seed 99)", n, arestas);
    {
        vector<Aresta> denso = gerarGrafoConexo(2000, 1999000, 1999000);
        tabelaCasoBase("Grafo completo: n=2.000, m=1.999.000 (o mesmo do teste DENSIDADE)",
                       2000, denso);
    }
    {
        vector<Aresta> grande = gerarGrafoConexo(1000000, 10000000, 2026);
        tabelaCasoBase("Grande instancia: n=1.000.000, m=10.000.000 (seed 2026)",
                       1000000, grande);
    }

    titulo("OTIMIZACAO 2 - Subproblemas em paralelo (n=200.000, m=2.000.000, mediana de 5)");
    std::printf("Nucleos disponiveis nesta maquina: %u\n", std::thread::hardware_concurrency());
    std::printf("%12s %18s %10s %10s %12s\n", "limite base", "niveis paralelos",
                "tarefas", "D&C ms", "aceleracao");
    for (int64_t limite : {(int64_t)1, (int64_t)n}) {
        double base = 0;
        for (int niveis : {0, 1, 2}) {
            OpcoesDC op;
            op.niveisParalelos = niveis;
            op.limiteCasoBase = limite;
            vector<double> t;
            int64_t peso = 0;
            for (int i = 0; i < 5; ++i) {
                ResultadoDC r = agmDivisaoConquista(n, arestas, op);
                t.push_back(r.ms);
                peso = r.pesoTotal;
            }
            if (niveis == 0) base = mediana(t);
            std::printf("%12lld %18d %10d %10.3f %11.2fx   (peso %lld)\n",
                        (long long)limite, niveis, 1 << niveis, mediana(t),
                        base / mediana(t), (long long)peso);
        }
    }
}

static void rodarGrafo(int n, const vector<Aresta>& arestas, const vector<string>& rot) {
    OpcoesDC op;
    op.registrarArvore = arestas.size() <= 32;
    ResultadoDC r = agmDivisaoConquista(n, arestas, op);
    if (op.registrarArvore) imprimirArvoreRecursao(r, rot);
    if (arestas.size() <= 200)
        std::printf("\nAGM: %s\n", textoLista(r.arvore, rot).c_str());
    mostrarMetricasDC(n, arestas.size(), r);
    std::printf("Tempo D&C: %.3f ms | Kruskal guloso: peso %lld\n", r.ms,
                (long long)kruskal(n, arestas).pesoTotal);
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
        "  aula11 pequeno | medio | grande | tres | verificacao | escala |\n"
        "         densidade | otimizacao | todos\n"
        "  aula11 arquivo CAMINHO\n"
        "  aula11 aleatorio N M [SEED]\n");
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    string modo = argc > 1 ? argv[1] : "todos";
    const std::map<string, std::function<void()>> testes = {
        {"pequeno", testePequeno},   {"medio", testeMedio},
        {"grande", testeGrande},     {"tres", tresParadigmas},
        {"verificacao", verificacao}, {"escala", escala},
        {"densidade", densidade},    {"otimizacao", otimizacao},
    };
    try {
        ambiente();
        if (modo == "todos") {
            for (const char* t : {"pequeno", "medio", "grande", "tres",
                                  "verificacao", "escala", "densidade", "otimizacao"})
                testes.at(t)();
        } else if (modo == "arquivo" && argc > 2) {
            titulo(string("Grafo lido de ") + argv[2]);
            GrafoArquivo g = lerArquivo(argv[2]);
            rodarGrafo(g.n, g.arestas, g.rotulos);
        } else if (modo == "aleatorio" && argc > 3) {
            int n = std::stoi(argv[2]);
            int64_t m = std::stoll(argv[3]);
            uint64_t semente = argc > 4 ? std::stoull(argv[4]) : 42;
            titulo("Grafo aleatorio n=" + std::to_string(n) + ", m=" +
                   std::to_string(m) + ", seed=" + std::to_string(semente));
            rodarGrafo(n, gerarGrafoConexo(n, m, semente), {});
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
