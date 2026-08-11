import time
import random

def quick_sort(lista):

    if len(lista) <= 1:
        return lista

    pivo = lista[len(lista) // 2]
    menores = [x for x in lista if x < pivo]
    iguais = [x for x in lista if x == pivo]
    maiores = [x for x in lista if x > pivo]

    return quick_sort(menores) + iguais + quick_sort(maiores)

def prime_numbers(lista):

    listadeprimo = []
    for num in lista:
        if num < 2:
            continue
        prime = True
        for i in range(2,num-1):
            if num % i == 0:
                prime = False
                break
        if prime:
            listadeprimo.append(num)
    return listadeprimo


inicioGeracao = time.time()
array = []
for i in range(100000):
    array.append(random.randint(0, 15000))
fimGeracao = time.time()

inicioPrimosDES = time.time()
prime_numbers(array)
fimPrimosDES = time.time()

inicioOrdenarDES = time.time()
array = quick_sort(array)
fimOrdenarDES = time.time()

inicioOrdenarORD = time.time()
array = quick_sort(array)
fimOrdenarORD = time.time()

inicioPrimosORD = time.time()
prime_numbers(array)
fimPrimosORD = time.time()

tempoGeracao = fimGeracao - inicioGeracao
tempoOrdernarDES = fimOrdenarDES - inicioOrdenarDES
tempoOrdernarORD = fimOrdenarORD - inicioOrdenarORD
tempoPrimosDES = fimPrimosDES - inicioPrimosDES
tempoPrimosORD = fimPrimosORD - inicioPrimosORD


print(f"\nTempo geração:  {tempoGeracao * 1000:.3f} ms", 
      f"\n\nTempo Ordernar DES:  {tempoOrdernarDES * 1000:.3f} ms", 
      f"\nTempo Primos DES:  {tempoPrimosDES * 1000:.3f} ms",
      f"\n\nTempo Ordernar ORD:  {tempoOrdernarORD * 1000:.3f} ms", 
    f"\nTempo Primos ORD:  {tempoPrimosORD * 1000:.3f} ms")