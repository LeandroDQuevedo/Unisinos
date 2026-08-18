import time
import random

def insertionSort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1

        while j >= 0 and key < arr[j]:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key

def selectionSort(arr):
    n = len(arr)
    for i in range(n - 1):
      
        min_idx = i
        
        for j in range(i + 1, n):
            if arr[j] < arr[min_idx]:

                min_idx = j
        
        arr[i], arr[min_idx] = arr[min_idx], arr[i]


inicioGeracao = time.time()
array = []
for i in range(100000):
    array.append(random.randint(0, 10500))
fimGeracao = time.time()
arrayInsertion = array.copy()
arraySelection = array.copy()

inicioInsertionSortDES = time.perf_counter()
insertionSort(arrayInsertion)
fimInsertionSortDES = time.perf_counter()

inicioSelectionSortDES = time.perf_counter()
selectionSort(arraySelection)
fimSelectionSortDES = time.perf_counter()

inicioInsertionSortORD = time.perf_counter()
insertionSort(arrayInsertion)
fimInsertionSortORD = time.perf_counter()

inicioSelectionSortORD = time.perf_counter()
selectionSort(arraySelection)
fimSelectionSortORD = time.perf_counter()

tempoGeracao = fimGeracao - inicioGeracao
tempoInsertionSortDES = fimInsertionSortDES - inicioInsertionSortDES
tempoInsertionSortORD = fimInsertionSortORD - inicioInsertionSortORD
tempoSelectionSortDES = fimSelectionSortDES - inicioSelectionSortDES
tempoSelectionSortORD = fimSelectionSortORD - inicioSelectionSortORD


print(f"\nTempo InsertionSort DES:  {tempoInsertionSortDES * 1000:.3f} ms", 
      f"\nTempo InsertionSort ORD:  {tempoInsertionSortORD * 1000:.3f} ms",
      f"\n\nTempo SelectionSort DES:  {tempoSelectionSortDES * 1000:.3f} ms", 
    f"\nTempo SelectionSort ORD:  {tempoSelectionSortORD * 1000:.3f} ms")