# Testy
## Testy jednostkowe (uruchamiane przy budowaniu)
Test sprawdzający poszczególnych funkcji.
- Sprawdzanie, czy klient spełnia wymagania atrakcji (`MeetsAttractionCriteria`)
- Sprawdzanie zamiany `std::string` na `std::array<char, N>`

## Ewakuacja
Test sprawdzający działanie sygnału ewakuacji/odbierania sygnału ewakuacji przez klientów.

- Klienci wchodzą do atrakcji
- W trakcie działania atrakcji pracownicy przerywają pracę (sygnał `SIGSTOP`)
- Wysłanie sygnału `SIGTERM` lub `SIGINT` do kierownika

**Polecenia:**
```bash
pkill --signal SIGSTOP -f '^park-attraction$'
pkill --signal SIGTERM -x 'park-main'
```

**Zakończenie:**
```bash
pkill --signal SIGCONT -f '^park-attraction$'
```

**Oczekiwanie:**
- Klienci natychmiast opuszczają atrakcje i próbują wyjść przez kasę.
- Kierownik kończy pracę dopiero po wysłaniu `SIGCONT` do atrakcji

## Brak deadlocków przy niewystarczających zasobach systemowych
Test sprawdzający działanie wchodzenia/wychodzenia klientów przy

- Klienci wchodzą do parku/atrakcji
- Po wystąpieniu komunikatów o przekroczeniu maksymalnej liczby kolejek komunikatów wysłanie sygnału `SIGTERM` lub `SIGINT` do kierownika

**Przygotowanie:**

Dodanie w [`Config.hpp`](../lib/include/Config/Config.hpp)
```cpp
#define RUN_TEST2
```
przed
```cpp
#ifdef RUN_TEST2
```

**Polecenia:**

Dodatkowe ograniczenie liczby kolejek komunikatów (wymaga uprawnień roota):
```bash
sysctl fs.mqueue.queues_max=128
```
```bash
pkill --signal SIGTERM -x 'park-main'
```

**Oczekiwania:**
- Klienci opuszczają wszystkie atrakcje i park

## Brak możliwości wyjścia z parku, gdy kasa i restauracja są zapauzowane
Test sprawdzający działanie wchodzenia/wychodzenia klientów przy

- Klienci wchodzą do parku/atrakcji
- Po wejściu klientów uruchomieniu atrakcji wysłanie `SIGSTOP` do kasy i restauracji
- Wysłanie `SIGTERM` lub `SIGINT` do kierownika

**Polecenia:**
Wysłanie sygnałów:
```bash
pkill --signal SIGSTOP -x 'park-cashier'
pkill --signal SIGSTOP -x 'park-restaurant'
pkill --signal SIGTERM -x 'park-main'
```

Kontynuacja kasy i restauracji:
```bash
pkill --signal SIGCONT -x 'park-cashier'
pkill --signal SIGCONT -x 'park-restaurant'
```

**Oczekiwania:**
- Klienci otrzymają sygnał ewakuacji
- Klienci nie opuszczają restauracji/parku dopóki nie zostaną kontynuowane procesy kasy i restauracji