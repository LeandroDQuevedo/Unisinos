# gerar_figuras.py
# Gera as figuras do relatório da Aula 11 a partir de resultados/saida_completa.txt
# e resultados/arvore_pequeno.json.   Uso: python3 figuras/gerar_figuras.py
import json
import os
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, Polygon

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SAIDA = open(os.path.join(RAIZ, "resultados", "saida_completa.txt"), encoding="utf-8").read()
ARVORE = json.load(open(os.path.join(RAIZ, "resultados", "arvore_pequeno.json"), encoding="utf-8"))
DEST = os.path.join(RAIZ, "figuras")

# Paleta (ordem fixa): guloso, PD, D&C, variante
AZUL, LARANJA, VERDE, AMARELO = "#2a78d6", "#eb6834", "#1baf7a", "#eda100"
VERMELHO = "#e34948"
TINTA, TINTA2, GRADE, FUNDO = "#0b0b0b", "#52514e", "#dddcd6", "#ffffff"
AZUL_CLARO, VERDE_CLARO, CINZA_CLARO, LARANJA_CLARO = "#e3eefb", "#dff4ec", "#f1f0ec", "#fde9df"

plt.rcParams.update({
    "font.family": "DejaVu Sans", "font.size": 9, "text.color": TINTA,
    "axes.edgecolor": GRADE, "axes.labelcolor": TINTA2, "xtick.color": TINTA2,
    "ytick.color": TINTA2, "axes.spines.top": False, "axes.spines.right": False,
    "figure.facecolor": FUNDO, "axes.facecolor": FUNDO, "savefig.dpi": 200,
})


def caixa(ax, x, y, w, h, texto, cor=CINZA_CLARO, borda=TINTA2, tam=8, negrito=False, lw=0.9):
    ax.add_patch(FancyBboxPatch((x - w / 2, y - h / 2), w, h,
                                boxstyle="round,pad=0.0,rounding_size=0.06",
                                fc=cor, ec=borda, lw=lw, zorder=2))
    ax.text(x, y, texto, ha="center", va="center", fontsize=tam,
            fontweight="bold" if negrito else "normal", zorder=3, linespacing=1.25)


def seta(ax, a, b, cor=TINTA2, estilo="-|>", lw=0.9, ls="-", rad=0.0):
    ax.add_patch(FancyArrowPatch(a, b, arrowstyle=estilo, mutation_scale=9, lw=lw,
                                 color=cor, ls=ls, zorder=1,
                                 connectionstyle=f"arc3,rad={rad}", shrinkA=0, shrinkB=0))


def salvar(fig, nome):
    fig.savefig(os.path.join(DEST, nome), bbox_inches="tight", pad_inches=0.08)
    plt.close(fig)
    print("ok", nome)


# ------------------------------------------------ Figura 1: esquema geral
def fig_esquema():
    fig, ax = plt.subplots(figsize=(6.6, 3.5))
    ax.set_xlim(0, 10); ax.set_ylim(0, 6.2); ax.axis("off")
    caixa(ax, 5, 5.6, 3.6, 0.7, "Problema de tamanho n", AZUL_CLARO, AZUL, 9, True)
    caixa(ax, 2.6, 4.0, 3.0, 0.7, "Subproblema 1\n(tamanho n/2)", CINZA_CLARO, tam=8)
    caixa(ax, 7.4, 4.0, 3.0, 0.7, "Subproblema 2\n(tamanho n/2)", CINZA_CLARO, tam=8)
    caixa(ax, 2.6, 2.3, 3.0, 0.7, "Solução do\nsubproblema 1", VERDE_CLARO, VERDE, 8)
    caixa(ax, 7.4, 2.3, 3.0, 0.7, "Solução do\nsubproblema 2", VERDE_CLARO, VERDE, 8)
    caixa(ax, 5, 0.6, 4.6, 0.7, "Solução do problema original", VERDE_CLARO, VERDE, 9, True)
    seta(ax, (4.2, 5.25), (2.9, 4.36)); seta(ax, (5.8, 5.25), (7.1, 4.36))
    seta(ax, (2.6, 3.64), (2.6, 2.66)); seta(ax, (7.4, 3.64), (7.4, 2.66))
    seta(ax, (2.9, 1.94), (4.2, 0.96)); seta(ax, (7.1, 1.94), (5.8, 0.96))
    ax.text(5, 4.72, "DIVIDIR", ha="center", fontsize=8.5, fontweight="bold", color=AZUL)
    ax.text(5, 3.15, "CONQUISTAR\n(recursão até o caso base)", ha="center", va="center",
            fontsize=8.5, fontweight="bold", color=TINTA2)
    ax.text(5, 1.52, "COMBINAR", ha="center", fontsize=8.5, fontweight="bold", color=VERDE)
    salvar(fig, "fig1_esquema_dc.png")


# ------------------------------- Figura 2: árvore de recursão T(n)=2T(n/2)+cn
def fig_arvore_generica():
    fig, ax = plt.subplots(figsize=(6.6, 3.3))
    ax.set_xlim(0, 10.6); ax.set_ylim(0, 5.4); ax.axis("off")
    niveis = [(4.8, ["cn"], [4.0]),
              (3.6, ["cn/2", "cn/2"], [2.0, 6.0]),
              (2.4, ["cn/4"] * 4, [1.0, 3.0, 5.0, 7.0])]
    for y, rot, xs in niveis:
        for x, r in zip(xs, rot):
            caixa(ax, x, y, 1.1, 0.5, r, AZUL_CLARO, AZUL, 8)
    for (y1, _, xs1), (y2, _, xs2) in zip(niveis, niveis[1:]):
        for i, x in enumerate(xs1):
            seta(ax, (x, y1 - 0.25), (xs2[2 * i], y2 + 0.25), estilo="-")
            seta(ax, (x, y1 - 0.25), (xs2[2 * i + 1], y2 + 0.25), estilo="-")
    for x in [1.0, 3.0, 5.0, 7.0]:
        ax.text(x, 1.62, "⋮", ha="center", va="center", fontsize=11, color=TINTA2)
    for i in range(8):
        caixa(ax, 0.5 + i, 0.75, 0.62, 0.42, "c", VERDE_CLARO, VERDE, 8)
    ax.text(4.0, 0.2, "n casos base (folhas)", ha="center", fontsize=8, color=TINTA2)
    for y, t in [(4.8, "cn"), (3.6, "cn"), (2.4, "cn"), (0.75, "cn")]:
        ax.text(8.5, y, "→  " + t, ha="left", va="center", fontsize=8.5)
    ax.text(8.5, 5.28, "custo por nível", ha="left", fontsize=8, color=TINTA2)
    ax.annotate("", xy=(10.2, 0.6), xytext=(10.2, 5.0),
                arrowprops=dict(arrowstyle="<->", color=TINTA2, lw=0.9))
    ax.text(10.32, 2.8, "log₂ n + 1 níveis", rotation=90, va="center", fontsize=8, color=TINTA2)
    salvar(fig, "fig2_arvore_recorrencia.png")


# --------------------------- grafo pequeno (usado nas figuras 3)
POS = {"A": (0, 2), "B": (2, 2), "C": (4, 2), "D": (1, 1), "E": (3, 1), "F": (2, 0), "G": (4, 0)}
ARESTAS = [("A", "B", 7), ("A", "D", 5), ("B", "C", 8), ("B", "D", 9), ("B", "E", 7),
           ("C", "E", 5), ("D", "E", 15), ("D", "F", 6), ("E", "F", 8), ("E", "G", 9),
           ("F", "G", 11)]


def desenhar_grafo(ax, escolhidas, titulo, grupos=None, cor=VERDE):
    ax.set_xlim(-0.5, 4.5); ax.set_ylim(-0.55, 2.75); ax.axis("off"); ax.set_aspect("equal")
    for u, v, p in ARESTAS:
        (x1, y1), (x2, y2) = POS[u], POS[v]
        dentro = (u, v) in escolhidas
        ax.plot([x1, x2], [y1, y2], color=cor if dentro else GRADE,
                lw=3.0 if dentro else 1.2, zorder=1, solid_capstyle="round")
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        ax.text(mx, my, str(p), ha="center", va="center", fontsize=8,
                fontweight="bold" if dentro else "normal",
                color=TINTA if dentro else TINTA2, zorder=3,
                bbox=dict(boxstyle="round,pad=0.12", fc=FUNDO, ec="none"))
    for nome, (x, y) in POS.items():
        fc = FUNDO
        if grupos:
            fc = AZUL_CLARO if nome in grupos[0] else LARANJA_CLARO
        ax.scatter([x], [y], s=330, fc=fc, ec=TINTA, lw=1.1, zorder=4)
        ax.text(x, y, nome, ha="center", va="center", fontsize=9, fontweight="bold", zorder=5)
    ax.set_title(titulo, fontsize=8.5, pad=4)


def fig_contraexemplo():
    fig, axs = plt.subplots(1, 2, figsize=(6.8, 2.75))
    errada = {("A", "D"), ("A", "B"), ("B", "C"), ("E", "F"), ("E", "G"), ("C", "E")}
    otima = {("A", "D"), ("C", "E"), ("D", "F"), ("A", "B"), ("B", "E"), ("E", "G")}
    desenhar_grafo(axs[0], errada,
                   "Divisão dos vértices (incorreta)\npeso 20 + 17 + 5 = 42",
                   grupos=("ABCD", "EFG"), cor=VERMELHO)
    desenhar_grafo(axs[1], otima, "Árvore geradora mínima\npeso 39",
                   grupos=("ABCD", "EFG"), cor=VERDE)
    fig.subplots_adjust(wspace=0.08)
    salvar(fig, "fig3_contraexemplo_vertices.png")


# ------------------------------------------------ Figura 4: fluxograma
def fig_fluxograma():
    fig, ax = plt.subplots(figsize=(6.6, 7.0))
    ax.set_xlim(0, 10); ax.set_ylim(0, 13.4); ax.axis("off")
    cx = 4.2

    def losango(x, y, w, h, texto):
        ax.add_patch(Polygon([(x, y + h / 2), (x + w / 2, y), (x, y - h / 2), (x - w / 2, y)],
                             closed=True, fc="#fff6dd", ec=AMARELO, lw=1.0, zorder=2))
        ax.text(x, y, texto, ha="center", va="center", fontsize=8, zorder=3)

    caixa(ax, cx, 12.9, 4.6, 0.6, "AGM-DC(arestas[início, fim))", AZUL_CLARO, AZUL, 8.5, True)
    losango(cx, 11.55, 4.2, 1.3, "O trecho tem\n0 ou 1 aresta?")
    caixa(ax, 8.3, 11.55, 2.9, 0.9, "CASO BASE\ndevolve o próprio\ntrecho (já é floresta)", VERDE_CLARO, VERDE, 7.5)
    caixa(ax, cx, 10.0, 4.6, 0.7, "DIVIDIR\nmeio ← (início + fim) / 2", AZUL_CLARO, AZUL, 8)
    caixa(ax, 2.25, 8.6, 3.5, 0.8, "CONQUISTAR\nF₁ ← AGM-DC(arestas[início, meio))", CINZA_CLARO, tam=7.3)
    caixa(ax, 6.15, 8.6, 3.5, 0.8, "CONQUISTAR\nF₂ ← AGM-DC(arestas[meio, fim))", CINZA_CLARO, tam=7.3)
    caixa(ax, cx, 7.15, 5.4, 0.7, "COMBINAR(F₁, F₂)\nUnion-Find vazio; i ← 0, j ← 0, F ← ∅", VERDE_CLARO, VERDE, 8)
    losango(cx, 5.75, 4.9, 1.45, "Restam arestas em\nF₁ ou F₂ e |F| < n − 1?")
    caixa(ax, cx, 4.3, 5.4, 0.7, "e ← aresta mais leve entre F₁[i] e F₂[j]\n(empate: a de F₁); avança o índice", CINZA_CLARO, tam=7.8)
    losango(cx, 2.9, 4.9, 1.3, "find(e.u) ≠ find(e.v)?")
    caixa(ax, 1.5, 1.35, 2.7, 0.8, "unir(e.u, e.v)\nF ← F ∪ {e}", VERDE_CLARO, VERDE, 7.8)
    caixa(ax, 6.9, 1.35, 2.7, 0.8, "descarta e\n(fecharia ciclo)", "#fde4e4", VERMELHO, 7.8)
    caixa(ax, 8.3, 5.75, 2.9, 1.0, "reinicia os vértices\ntocados e devolve F\n(ordenada por peso)", VERDE_CLARO, VERDE, 7.5)

    seta(ax, (cx, 12.6), (cx, 12.2))
    seta(ax, (cx + 2.1, 11.55), (6.85, 11.55)); ax.text(6.45, 11.72, "sim", fontsize=7.5, ha="center")
    seta(ax, (cx, 10.9), (cx, 10.35)); ax.text(cx + 0.3, 10.62, "não", fontsize=7.5)
    seta(ax, (cx - 1.0, 9.65), (2.25, 9.0)); seta(ax, (cx + 1.0, 9.65), (6.15, 9.0))
    seta(ax, (2.25, 8.2), (cx - 1.0, 7.5)); seta(ax, (6.15, 8.2), (cx + 1.0, 7.5))
    seta(ax, (cx, 6.8), (cx, 6.4))
    seta(ax, (cx, 5.1), (cx, 4.65)); ax.text(cx + 0.3, 4.86, "sim", fontsize=7.5)
    seta(ax, (cx + 2.45, 5.75), (6.85, 5.75)); ax.text(6.62, 5.92, "não", fontsize=7.5, ha="center")
    seta(ax, (cx, 3.95), (cx, 3.55))
    # ramos do teste de ciclo
    ax.plot([cx - 2.45, 1.5], [2.9, 2.9], color=TINTA2, lw=0.9); seta(ax, (1.5, 2.9), (1.5, 1.75))
    ax.text(1.75, 3.05, "sim", fontsize=7.5)
    ax.plot([cx + 2.45, 6.9], [2.9, 2.9], color=TINTA2, lw=0.9); seta(ax, (6.9, 2.9), (6.9, 1.75))
    ax.text(6.65, 3.05, "não", fontsize=7.5, ha="right")
    # volta ao laço
    ax.plot([1.5, 1.5, 0.35, 0.35], [0.95, 0.45, 0.45, 5.75], color=TINTA2, lw=0.9)
    ax.plot([6.9, 6.9, 1.5], [0.95, 0.45, 0.45], color=TINTA2, lw=0.9)
    seta(ax, (0.35, 5.75), (cx - 2.45, 5.75))
    salvar(fig, "fig4_fluxograma.png")


# --------------------------- Figura 5: árvore de recursão do teste pequeno
def curto(lista):
    return [s.replace("(", " ").replace(")", "") for s in lista]


def fig_arvore_pequeno():
    nos = {n["id"]: n for n in ARVORE}
    filhos = {i: [n["id"] for n in ARVORE if n["pai"] == i] for i in nos}
    folhas = [n["id"] for n in ARVORE if n["base"]]
    x = {f: i for i, f in enumerate(folhas)}

    def pos(i):
        if i not in x:
            x[i] = sum(pos(c) for c in filhos[i]) / len(filhos[i])
        return x[i]
    pos(0)
    prof_max = max(n["prof"] for n in ARVORE)
    alt = 1.55

    fig, ax = plt.subplots(figsize=(7.2, 6.6))
    ax.set_xlim(-0.7, len(folhas) - 0.3); ax.set_ylim(-1.15, prof_max * alt + 0.75); ax.axis("off")

    def y(i):
        return (prof_max - nos[i]["prof"]) * alt

    for n in ARVORE:
        if n["pai"] >= 0:
            p = n["pai"]
            ax.plot([x[p], x[n["id"]]], [y(p) - 0.42, y(n["id"]) + 0.42], color=GRADE, lw=1.3, zorder=1)
    for n in ARVORE:
        i = n["id"]
        if n["base"]:
            a = curto(n["entrada"])[0].split(" ")
            caixa(ax, x[i], y(i), 0.86, 0.72, f"#{i}\n{a[0]}\n{a[1]}", CINZA_CLARO, TINTA2, 6.6)
        else:
            saida = curto(n["saida"])
            desc = curto(n["descartadas"])
            larg = {0: 5.4, 1: 3.9, 2: 2.05, 3: 1.8}[n["prof"]]
            if n["prof"] >= 2:
                linhas = [f"#{i}   [{n['inicio']}, {n['fim']})"]
                if len(saida) <= 2:
                    linhas += ["devolve", ", ".join(saida)]
                else:
                    linhas += ["devolve " + ", ".join(saida[:2]) + ",", ", ".join(saida[2:])]
            else:
                linhas = [f"#{i}   posições [{n['inicio']}, {n['fim']})   {n['fim'] - n['inicio']} arestas",
                          "devolve " + ", ".join(saida)]
            alt_caixa = 0.3 + 0.2 * (len(linhas) + (1 if desc else 0))
            caixa(ax, x[i], y(i), larg, alt_caixa, "", AZUL_CLARO if i else VERDE_CLARO,
                  AZUL if i else VERDE, 6.8, lw=1.1)
            topo = y(i) + alt_caixa / 2 - 0.17
            for k, l in enumerate(linhas):
                ax.text(x[i], topo - 0.2 * k, l, ha="center", va="center", fontsize=6.7,
                        fontweight="bold" if k == 0 else "normal", zorder=3)
            if desc:
                ax.text(x[i], topo - 0.2 * len(linhas), "descarta " + ", ".join(desc),
                        ha="center", va="center", fontsize=6.7, color="#b3261e", zorder=3)
    # setas-guia laterais
    xe = -0.55
    ax.annotate("", xy=(xe, 0.0), xytext=(xe, prof_max * alt),
                arrowprops=dict(arrowstyle="-|>", color=AZUL, lw=1.2))
    ax.text(xe - 0.1, prof_max * alt / 2, "DIVIDIR (descida)", rotation=90, va="center", ha="right",
            fontsize=7.5, color=AZUL, fontweight="bold")
    xd = len(folhas) - 0.45
    ax.annotate("", xy=(xd, prof_max * alt), xytext=(xd, 0.0),
                arrowprops=dict(arrowstyle="-|>", color=VERDE, lw=1.2))
    ax.text(xd + 0.1, prof_max * alt / 2, "COMBINAR (retorno)", rotation=270, va="center", ha="left",
            fontsize=7.5, color=VERDE, fontweight="bold")
    ax.text((len(folhas) - 1) / 2, -0.9,
            "Caixas cinza: casos base (uma aresta).  Caixas azuis: divisões, com a floresta devolvida após a combinação.\n"
            "Caixa verde (#0): problema original e solução final (peso 39).",
            ha="center", va="center", fontsize=6.8, color=TINTA2)
    salvar(fig, "fig5_arvore_recursao_pequeno.png")


# --------------------------- leitura dos resultados
def secao(inicio, fim=None):
    a = SAIDA.index(inicio)
    b = SAIDA.index(fim, a) if fim else len(SAIDA)
    return SAIDA[a:b]


def perfil(sec):
    linhas = []
    for l in sec.splitlines():
        m = re.match(r"\s*(\d+)\s+(\d+)\s+([\d.]+)\s+(\d+)\s+(\d+)\s+([\d.]+)%", l)
        if m:
            linhas.append([float(v) for v in m.groups()])
    return linhas


# --------------------------- Figura 6: perfil por nível (teste médio)
def fig_perfil():
    dados = perfil(secao("TESTE 2", "TESTE 3"))
    niveis = [int(d[0]) for d in dados][:-1]
    comb = [d[3] for d in dados][:-1]
    m = 10000
    fig, ax = plt.subplots(figsize=(6.6, 2.9))
    cores = [VERDE if c < 0.995 * m and i < 6 else "#9fd9c4" for i, c in enumerate(comb)]
    ax.bar(niveis, comb, width=0.72, color=cores, zorder=2)
    ax.axhline(m, color=TINTA2, lw=0.9, ls="--", zorder=1)
    ax.text(13.45, m * 1.02, "m = 10.000 arestas (um nível do Merge Sort)", ha="right", va="bottom",
            fontsize=7.5, color=TINTA2)
    for i in range(4):
        ax.text(niveis[i], comb[i] + 180, f"{int(comb[i]):,}".replace(",", "."), ha="center",
                fontsize=7.2)
    ax.set_xticks(niveis)
    ax.set_xlabel("nível da árvore de recursão (0 = problema original)")
    ax.set_ylabel("arestas que chegam às combinações")
    ax.set_ylim(0, m * 1.17)
    ax.yaxis.set_major_formatter(lambda v, _: f"{int(v):,}".replace(",", "."))
    ax.grid(axis="y", color=GRADE, lw=0.6, zorder=0); ax.set_axisbelow(True)
    from matplotlib.patches import Patch
    ax.legend(handles=[Patch(color=VERDE, label="níveis em que o filtro de ciclos reduz o trabalho"),
                       Patch(color="#9fd9c4", label="níveis em que quase nada é descartado")],
              frameon=False, fontsize=7.5, loc="upper center", bbox_to_anchor=(0.5, 1.2), ncol=2)
    salvar(fig, "fig6_perfil_niveis.png")


# --------------------------- Figura 7: três paradigmas (7, 14, 20)
def fig_tres():
    sec = secao("TRES PARADIGMAS", "VERIFICACAO")
    def med(rot):
        return [float(v) for v in re.findall(re.escape(rot) + r"\s*: mediana ([\d.]+) ms", sec)]
    k, pd, dc = med("  Kruskal guloso"), med("  Programacao Dinamica"), med("  Divisao e Conquista")
    fig, ax = plt.subplots(figsize=(6.6, 3.0))
    xs = range(3)
    w = 0.25
    for desloc, vals, cor, nome in [(-w, k, AZUL, "Guloso (Kruskal)"), (0, pd, LARANJA, "Programação Dinâmica"),
                                    (w, dc, VERDE, "Divisão e Conquista")]:
        ax.bar([x + desloc for x in xs], vals, width=w - 0.03, color=cor, label=nome, zorder=2)
        for x, v in zip(xs, vals):
            t = f"{v:.4f}" if v < 0.01 else (f"{v:.2f}" if v < 100 else f"{v:.0f}")
            ax.text(x + desloc, v * 1.25, t.replace(".", ","), ha="center", fontsize=6.8)
    ax.set_yscale("log"); ax.set_ylim(1e-4, 1e5)
    ax.set_xticks(list(xs)); ax.set_xticklabels(["n = 7, m = 11", "n = 14, m = 45", "n = 20, m = 100"])
    ax.set_ylabel("tempo mediano (ms, escala log)")
    ax.grid(axis="y", color=GRADE, lw=0.6, zorder=0); ax.set_axisbelow(True)
    ax.legend(frameon=False, fontsize=7.8, loc="upper left", ncol=3, handlelength=1.2,
              columnspacing=1.2)
    salvar(fig, "fig7_tres_paradigmas.png")
    return k, pd, dc


# --------------------------- Figura 8: escala (ns por aresta)
def fig_escala():
    sec = secao("ESCALA", "DENSIDADE")
    linhas = []
    for l in sec.splitlines():
        m = re.match(r"\s*(\d+)\s+(\d+) \|\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+) \|", l)
        if m:
            linhas.append([float(v) for v in m.groups()])
    ms = [l[1] for l in linhas]
    series = [("Guloso (Merge Sort iterativo)", 2, AZUL, "o"),
              ("Guloso + Merge Sort recursivo", 3, AMARELO, "s"),
              ("D&C, caso base de 1 aresta", 4, VERDE, "^"),
              ("D&C, caso base de n arestas", 5, VERDE, "v")]
    fig, ax = plt.subplots(figsize=(6.6, 3.0))
    for nome, col, cor, marc in series:
        ys = [l[col] * 1e6 / l[1] for l in linhas]
        ax.plot(ms, ys, color=cor, lw=2, marker=marc, ms=6, label=nome,
                ls="--" if marc == "v" else "-", mfc=FUNDO if marc == "v" else cor, zorder=3)
        ax.text(ms[-1] * 1.25, ys[-1] + {2: -14, 3: 14}.get(col, 0), f"{ys[-1]:.0f}",
                va="center", fontsize=7.5)
    ax.set_xscale("log"); ax.set_xlim(7e3, 2.6e7); ax.set_ylim(0, 700)
    ax.set_xticks(ms); ax.set_xticklabels(["10 mil", "100 mil", "1 milhão", "10 milhões"])
    ax.minorticks_off()
    ax.set_xlabel("número de arestas m (grafos com m = 10n)")
    ax.set_ylabel("tempo por aresta (ns)")
    ax.grid(axis="y", color=GRADE, lw=0.6, zorder=0)
    ax.legend(frameon=False, fontsize=7.6, loc="upper left")
    salvar(fig, "fig8_escala.png")


if __name__ == "__main__":
    fig_esquema()
    fig_arvore_generica()
    fig_contraexemplo()
    fig_fluxograma()
    fig_arvore_pequeno()
    fig_perfil()
    fig_tres()
    fig_escala()
