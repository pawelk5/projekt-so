# Komponenty projektu
## Biblioteka statyczna
Współdzielone funkcje, klasy i stałe znajdują się w [bibliotece statycznej](../lib) dołączanej do każdego programu.

## Konfiguracja
W bibliotece współdzielonej znajduje się plik [Config.hpp](../lib/include/Config/Config.hpp), w którym zdefiniowana jest większość stałych.

## Procesy
Każdy proces jest pochodną klasy [`Process`](../lib/include/Process.hpp), która zapewnia dostęp do głównej pamięci dzielonej [`struct SimulationData`](../lib/include/SimulationData.hpp), zbioru semaforów [`MainSemaphoreArray`](../lib/include/SimulationData.hpp) oraz kolejki komunikatów loggera (w ramach funkcji [`pLogMessage`](../lib/include/Process.hpp)).

### Kierownik (MainProc)
Proces nadrzędny, odpowiedzialny za tworzenie i usuwanie struktur IPC (zbiór semaforów, pamięć współdzielona). Kierownik tworzy inne procesy (kasę, atrakcje, restauracje i klientów).

Kierownik obsługuje sygnały `SIGINT`, `SIGTERM`, `SIGUSR1`, `SIGUSR2` oraz ignoruje `SIGCHLD`.

Kierownik dodatkowo tworzy i zarządza wątkiem odpowiedzialnym za zapisywanie logów do pliku.

### Kasa (CashierProc)
Proces nadzorujący klientów wchodzących i wychodzących z parku. Proces kończy się, gdy park jest zamknięty i wszyscy klienci opuścili park.

Kasa obsługuje sygnały `SIGUSR1` (ewakuacja) oraz `SIGTERM` (jako mechanizm awaryjny, gdy wszyscy klienci opuścili park, ale nie każdy był w stanie wysłać wiadomość do kasy, co pozwala odblokować proces z oczekiwania).

### Atrakcja (AttractionProc)
Proces nadzorujący działanie poszczególnej atrakcji.

Atrakcja obsługuje sygnały `SIGUSR1`, `SIGUSR2` - sygnały zamknięcia i otwarcia atrakcji.

### Restauracja (RestaurantProc)
Proces nadzorujący działanie restauracji. Proces kończy się jeżeli wszyscy klienci opuszczą restaurację.

Restauracja obsługuje sygnały `SIGUSR1`, `SIGUSR2` - sygnały zamknięcia i otwarcia restauracji oraz `SIGINT` - sygnał ewakuacji.

### Klient (ClientProc)
Proces reprezentuje klienta korzystającego z parku i jego atrakcji. Klient może również skorzystać z restauracji bez konieczności wchodzenia do parku.

Aby skorzystać z atrakcji lub wejścia do parku wysyła komunikat do danego procesu i tworzy własną kolejkę wyłącznie na odpowiedzi. Klient posiada mechanizm timeoutu, który powoduje, że w przypadku długiego oczekiwania rezygnuje z aktywności.

Klient obsługuje sygnał `SIGUSR1` - sygnał ewakuacji.

## Wykorzystane mechanizmy IPC
### Zestaw semaforów (SystemV)
Klasa `SemaphoreArray` zarządzająca zbiorem semaforów. Dodatkowo istnieją struktury reprezentujące pojedynczy semafor (`SemaphoreStruct`) oraz `SemaphoreLock`, który pozwala w prosty sposób zablokować zasób (np. pamięć dzieloną) w celu wykonania operacji w sekcji krytycznej.

### Pamięć współdzielona (SystemV)
Klasa `SharedMemory` zarządza pamięcią współdzieloną. Do obiektu tej klasy można przekazać semafor, który ma być wykorzystywany do synchronizacji operacji zapisu i odczytu.

### Kolejki komunikatów (POSIX)
Klasa `MessageQueue` pozwala tworzyć kolejki komunikatów i nimi zarządzać.

Różne typy kolejek są zdefiniowane w plikach nagłówkowych w katalogu [MessageTypes](../lib/include/MessageTypes/), i dodatkowo [istnieją funkcje](../lib/include/PredefinedMQ.hpp), które umożliwiają tworzenie kolejek tych typów.

### Sygnały
Sygnały są obsługiwane przez procesy kierownika, atrakcji, restauracji oraz klienta.

Funkcja do tworzenia handlerów znajduje się w pliku [Utils.hpp](../lib/include/Utils.hpp).

## Wykorzystane mechanizmy synchronizacji wątków
### Semafor POSIX (sem_t)
Wykorzystywany przy inicjalizacji wątku logów `g_loggerInitSem` ([MainProc.cpp](../app/main/src/MainProc/MainProc.cpp), [LoggerService.cpp](../app/main/src/LoggerService/LoggerService.cpp)), pozwala zapewnić odpowiednią kolejność startu symulacji (logger startuje przed stworzeniem innych procesów).

## Logi
Za tworzenie logów odpowiada wątek uruchamiany wewnątrz procesu kierownika.

### Pliki
Program tworzy główny plik z logami oraz dodatkowe pliki odpowiadają typowi procesu:
- main.log - główny plik, zawierający wszystkie logi
- attraction.log - plik zawierający logi procesów atrakcji
- restaurant.log - plik zawierający logi procesu restauracji
- cashier.log - plik zawierający logi procesu kasy
- client.log - plik zawierający logi procesów klientów

Pliki są obiektami klasy [`File`](../lib/include/File.hpp) - warstwa abstrakcji nad `open()` i `close()`.