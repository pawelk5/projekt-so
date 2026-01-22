## Schemat komunikacji
### Logger
- Logger po inicjalizacji wykonuje Signal na semaforze [`g_loggerInitSem`](../app/main/src/LoggerService/LoggerService.cpp), na którym czeka MainProc

- Logger w pętli oczekuje na wiadomość na kolejce komunikatów.

- Komunikat z `senderPID == -1` kończy działanie loggera (specjalny typ wiadomości wysyłany przez MainProc w trakcie zamykania symulacji)

### Klient (wchodzenie do parku)
- Klient wysyła wiadomość typu `EnterPark` do kolejki komunikatów kasy i wykonuje Signal na semaforze [`CashierEvent`](../lib/include/SimulationData.hpp) (semafor zliczający zdarzenia kasy)
- Kasa odbiera wiadomość, zapisuje do własnego wektora z zapytaniami o wejście do parku (segreguje osobno klientów ze statusem VIP i tych bez)
- Klient tworzy własną kolejkę komunikatów na odpowiedź i próbuje odebrać wiadomość (z timeoutem)
- Jeżeli jest miejsce w parku to kasa ([CashierProc](../app/cashier/src/CashierProc/CashierProc.hpp)) próbuje wysłać wiadomość z pozwoleniem na wejście do parku (najpierw z wektora z klientami VIP), dopóki nie zapełni się park lub wszyscy klienci w kolejce nie zostaną obsłużeni

Komunikacja klient-kasa na kolejce odpowiedzi klienta:
- Kasa próbuje klientowi wysłać strukturę z pozwoleniem na wejście
- Klient dopiero wchodzi do parku po odesłaniu wiadomości z potwierdzeniem (`ACK`)

Dodatkowo niezależnie od tego, czy klientowi uda się wejść do parku kolejka komunikatów jest usuwana.

### Klient (wychodzenie z parku)
- Klient wysyła wiadomość `ExitPark` do kolejki komunikatów kasy i wykonuje Signal na semaforze [`CashierEvent`](../lib/include/SimulationData.hpp) (semafor zliczający zdarzenia kasy)
- Kasa próbuje wysłać strukturę `Bill` (reprezentującą rachunek)
- Klient odsyła wiadomość z potwierdzeniem (`ACK`)

- Jeżeli klient nie jest w stanie stworzyć kolejki odpowiedzi lub nastąpi inny błąd przy przesyłaniu rachunku **klient wychodzi bez otrzymania rachunku, a park odnotowuje wyjście klienta z parku**, co zapisuje do loggera

Typy struktur są opisane w pliku [RegisterMQ.hpp](../lib/include/MessageTypes/RegisterMQ.hpp) oraz [ClientMQ](../lib/include/MessageTypes/ClientMQ.hpp).

### Klient (wchodzenie do atrakcji)
- Klient wysyła wiadomość `EnterAttraction` do kolejki komunikatów atrakcji i wykonuje Signal na semaforze zliczającym zdarzenia atrakcji (ID od `AttractionEvent1` do `RestaurantEvent`)
- Klient otrzymuje odpowiedź typu `AttractionEntryPermit`, który zawiera flagę, czy klient może wejść do atrakcji oraz numer semafora wyjściowego z atrakcji
- Jeżeli klient ma pozwolenie na wejście to wykonuje Wait na wybranym semaforze
- Jeżeli klient wychodzi przed zamknięciem wagonika/końcem tury w atrakcji wysyła wiadomość `ExitAttraction`

### Klient (wychodzenie z atrakcji)
- Jeżeli klient wychodzi przed czasem będzie próbował wysłać wiadomość o wyjściu do procesu atrakcji, ale nie będzie tworzył własnej kolejki do czekania na potwierdzenie otrzymania wiadomości przez procesu atrakcji
- Jeżeli klient otrzyma sygnał `SIGUSR1` i znajduje się w parku lub restauracji, będzie próbował opuścić park (wraz z poinformowaniem atrakcji/kasy)

### Atrakcja (w tym restauracja)
- Atrakcja działa w pętli dopóki park jest otwarty
- Każda atrakcja ma semafor ([od `AttractionEvent1` do `RestaurantEvent`](../lib/include/SimulationData.hpp)), który zlicza zdarzenia (iteracja następuje wyłącznie wtedy, kiedy nastąpiło zdarzenie)
- Każda atrakcja ma przynajmniej jeden obiekt [`AttractionHandler`](../lib/include/AttractionHandler.hpp) - wiele obiektów tej klasy oznacza, że atrakcja działa na zasadzie wielu wagoników
- Obiekt `AttractionHandler` śledzi liczbę klientów i czas od rozpoczęcia działania oraz zarządza semaforem, na którym czekają wszyscy klienci po wejściu do atrakcji ([semafory od `AttractionHandler1` do `RestaurantHandler`](../lib/include/SimulationData.hpp))

### Kasa
- Kasa działa w pętli dopóki park jest otwarty lub są w nim klienci (czeka aż opuszczą park)
- Kasa ma semafor `CashierEvent`, który zlicza zdarzenia (iteracja następuje wyłącznie wtedy, kiedy nastąpiło zdarzenie)

### Kierownik
- Tworzy pamięć współdzieloną i zbiór semaforów oraz jest odpowiedzialny za usunięcie tych struktur
- Tworzy i uruchamia wątek loggera
- Kierownik obsługuje sygnały `SIGUSR1` i `SIGUSR2`, które przerywają i kontynuują generowanie klientów
- Jeżeli generowanie klientów jest przerwane, to kierownik czeka na semaforze ([`MainPause`](../lib/include/SimulationData.hpp))

### Sygnał zamknięcia parku
- Kierownik otrzymuje sygnał `SIGINT` lub `SIGTERM`, ustawia flagę `isOpen` w pamięci dzielonej na `false`
- Kierownik wysyła sygnał `SIGUSR1` do kasy i `SIGINT` do restauracji
- Kasa wysyła `SIGUSR1` do wszystkich klientów znajdujących się w parku
- Restauracja wysyła `SIGUSR1` do wszystkich klientów znajdujących się w restauracji

**Kasa i restauracja czekają aż wszyscy klienci opuszczą park (wyślą wiadomość o opuszczeniu).**