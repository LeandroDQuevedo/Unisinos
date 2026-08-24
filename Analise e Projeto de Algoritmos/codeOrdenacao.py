import time
import random


# ============================================================
#  ALGORITMOS DE ORDENAÇÃO
# ============================================================

def insertionSort(arr):
    for i in range(1, len(arr)):
        chave = arr[i]
        j = i - 1
        while j >= 0 and arr[j] > chave:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = chave
    return arr


def selectionSort(arr):
    n = len(arr)
    for i in range(n):
        menorIdx = i
        for j in range(i + 1, n):
            if arr[j] < arr[menorIdx]:
                menorIdx = j
        arr[i], arr[menorIdx] = arr[menorIdx], arr[i]
    return arr


def quickSort(lista):
    if len(lista) <= 1:
        return lista

    pivo = lista[len(lista) // 2]
    menores = [x for x in lista if x < pivo]
    iguais = [x for x in lista if x == pivo]
    maiores = [x for x in lista if x > pivo]

    return quickSort(menores) + iguais + quickSort(maiores)


def mergeSort(arr):
    if len(arr) <= 1:
        return arr

    mid = len(arr) // 2
    leftHalf = arr[:mid]
    rightHalf = arr[mid:]

    sortedLeft = mergeSort(leftHalf)
    sortedRight = mergeSort(rightHalf)

    return merge(sortedLeft, sortedRight)


def merge(left, right):
    result = []
    i = j = 0

    while i < len(left) and j < len(right):
        if left[i] < right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1

    result.extend(left[i:])
    result.extend(right[j:])

    return result


def shellSort(array):
    n = len(array)
    interval = n // 2
    while interval > 0:
        for i in range(interval, n):
            temp = array[i]
            j = i
            while j >= interval and array[j - interval] > temp:
                array[j] = array[j - interval]
                j -= interval
            array[j] = temp
        interval //= 2
    return array


# Wrappers: recebem uma lista e devolvem uma NOVA lista ordenada,
# sem alterar a lista original (necessário pois DES e ORD reutilizam arrays)
def runInsertionSort(arr):
    copia = list(arr)
    insertionSort(copia)
    return copia


def runSelectionSort(arr):
    copia = list(arr)
    selectionSort(copia)
    return copia


def runQuickSort(arr):
    return quickSort(list(arr))


def runMergeSort(arr):
    return mergeSort(list(arr))


def runShellSort(arr):
    copia = list(arr)
    shellSort(copia)
    return copia


ALGORITMOS = {
    "1": ("Insertion Sort", runInsertionSort),
    "2": ("Selection Sort", runSelectionSort),
    "3": ("Merge Sort", runMergeSort),
    "4": ("Quick Sort", runQuickSort),
    "5": ("Shell Sort", runShellSort),
}


# ============================================================
#  EXECUÇÃO / MEDIÇÃO DE TEMPO
#  O cronômetro só corre em torno da chamada do algoritmo,
#  a geração do array acontece antes e fica de fora da medição.
# ============================================================

def executarAlgoritmo(nome, func, n):
    array = [random.randint(0, 10500) for _ in range(n)]

    # dois testes com o array desordenado
    inicioDES1 = time.perf_counter()
    arrayOrdenado = func(array)
    fimDES1 = time.perf_counter()

    inicioDES2 = time.perf_counter()
    func(array)
    fimDES2 = time.perf_counter()

    # dois testes com o array já ordenado
    inicioORD1 = time.perf_counter()
    func(arrayOrdenado)
    fimORD1 = time.perf_counter()

    inicioORD2 = time.perf_counter()
    func(arrayOrdenado)
    fimORD2 = time.perf_counter()

    tempoDES1 = (fimDES1 - inicioDES1) * 1000
    tempoDES2 = (fimDES2 - inicioDES2) * 1000
    tempoORD1 = (fimORD1 - inicioORD1) * 1000
    tempoORD2 = (fimORD2 - inicioORD2) * 1000

    print(f"\nTempo {nome} DES 1:  {tempoDES1:.3f} ms")
    print(f"Tempo {nome} DES 2:  {tempoDES2:.3f} ms")
    print(f"Tempo {nome} ORD 1:  {tempoORD1:.3f} ms")
    print(f"Tempo {nome} ORD 2:  {tempoORD2:.3f} ms")


# ============================================================
#  INTERFACE
# ============================================================

def main():
    while True:
        print("\n1 - Insertion Sort")
        print("2 - Selection Sort")
        print("3 - Merge Sort")
        print("4 - Quick Sort")
        print("5 - Shell Sort")
        print("0 - Sair")
        opcao = input("Escolha o algoritmo: ").strip()

        if opcao == "0":
            break

        if opcao not in ALGORITMOS:
            print("Opção inválida.")
            continue

        n = int(input("Quantidade de elementos: ").strip())
        nome, func = ALGORITMOS[opcao]
        executarAlgoritmo(nome, func, n)


if __name__ == "__main__":
    main()