# Temat 9 – Park rozrywki.

W pewnej miejscowości znajduje się park rozrywki dostępny w godzinach od Tp do Tk. Klienci w
różnym wieku (od 1 roku do 90 lat) przychodzą do parku w losowych momentach czasu. W parku
rozrywki w danym momencie czasu może znajdować się co najwyżej N osób. Wejście do parku
odbywa się po opłaceniu biletu wstępu w kasie (K), dzieci poniżej 2 roku życia nie płacą za bilet.
Osoba odwiedzająca park rozrywki kupuje bilet czasowy (2h,4h,6h,1D), który upoważnia ją do
korzystania z dowolnej atrakcji zgodnie z regulaminem przez pewien określony czas Ti. Jeżeli limit
czasu Ti zostanie przekroczony, klient przy wyjściu musi dopłacić różnicę proporcjonalnie do
pierwotnego limitu (dopłata o 100% wyższa niż podstawowa cena). Istnieje pewna liczba osób VIP
(ok. 1% ) – posiadająca wcześniej wykupiony karnet, które mogą bezpłatne wejść do parku z
pominięciem kolejki oczekujących. Osoby VIP nie mają limitu czasu przebywania w parku rozrywki –
mogą wyjść w dowolnym momencie. Kasa rejestruje wszystkie wchodzące osoby (ID procesu/wątku)
oraz wszystkie osoby wychodzące.

Dostępne atrakcje wraz z dodatkowymi ograniczeniami:
- Wodna bitwa (A1)– wiek brak ograniczeń, wzrost 120 - 140 cm z opiekunem, powyżej 140
cm bez opiekuna, otwarte w sezonie letnim, jednorazowo 20 osób, czas 30min.;
- Magiczna pompa (A2) - wiek brak ograniczeń, wzrost 100 - 120 cm z opiekunem, powyżej
120 cm bez opiekuna, otwarte w sezonie letnim, jednorazowo 12 osób, czas 30min.;
- Wyprawa do groty (A3) – wiek od 2 - 13 lat z opiekunem, wzrost od 120 cm, otwarte w sezonie
letnim, jednorazowo 16 osób, czas 25 min.;
- Zatoka bambusowa (A4) – wiek bez ograniczeń, wzrost 110 - 135 cm z opiekunem od 135
cm bez opiekuna, otwarte w sezonie letnim, jednorazowo 18 osób, czas 35min;
- Smocza przygoda (A5) – wiek od 2 - 13 lat z opiekunem, wzrost od 130 cm jednorazowo 14
osób, czas 20min;
- Cudowne koło (A6) – wiek od 0 - 13 lat z opiekunem, wzrost od 120 cm jednorazowo 8 osób,
czas 20min;
- Karuzela (A7) – wiek od 2 - 13 lat z opiekunem, wzrost 130 - 190 cm jednorazowo 12 osób,
czas 15min;
- Kolejka mała (A8) – wiek brak ograniczeń, wzrost 100 - 120 cm z opiekunem, od 120 cm bez
opiekuna, jednorazowo 24 osoby, czas 15min;
- Kolejka górska (A9) – wiek od 4 - 12 lat z opiekunem, wzrost od 120 cm, 5 wagoników po 4
osoby, czas 35min.;
- Kolejka smocza (A10) – wiek od 4 - 13 lat z opiekunem, wzrost od 120 cm;
- Mega Roller Coaster (A11) – wiek brak ograniczeń, wzrost 140 - 195 cm, jednorazowo 24
osoby, czas 30min.;
- Ławka obrotowa (A12) - wiek brak ograniczeń, wzrost 140 - 195 cm, jednorazowo 18 osób,
czas 25min.;
- Kosmiczny wzmacniacz (A13) - wiek brak ograniczeń, wzrost 140 - 195 cm, jednorazowo 2
kapsuły po 4 osoby, czas 20min.;
- Dom potwora (A14) – wiek od 4 - 12 lat z opiekunem, wzrost od 130 cm, jednorazowo 3
wagoniki po 4 osoby, czas 20min.;
- Samochodziki (A15) - wiek brak ograniczeń, wzrost od 120 cm, jednorazowo 10
samochodzików po 2 osoby, czas 15min.;
- Przygoda w dżungli (A16) – wiek brak ograniczeń, wzrost 120 - 140 cm z opiekunem, 140 -
195 cm bez opiekuna, 5 łodzi po 9 osób, czas 35min.;
- Restauracja (A17) – dostępna dla wszystkich, maksymalna liczba osób 50, czas losowy z
zakresu od 5min. do 60 min., do restauracji można wejść z zewnątrz oraz od strony parku
rozrywki, nie można wejść do parku lub wyjść z parku przez restaurację ;

Przy każdej atrakcji znajduje się pracownik parku – na polecenie (synał1) danego pracownika
natychmiast wszyscy muszą opuścić atrakcję, którą on nadzoruje (w tym czasie mogą udać się na
inne działające atrakcje lub czekać). Po wydaniu kolejnego polecenia (sygnał2) klienci mogą
ponownie wrócić do danej atrakcji.
Na polecenie Kierownika (sygnał3) wszyscy klienci i pracownicy natychmiast opuszczają park
rozrywki. Przy czym osoby będące w trakcie korzystania z atrakcji od A6 do A12 mogą opuścić park
z opóźnieniem 1min.(z uwagi na konieczność zatrzymania urządzenia). Park po wyjściu wszystkich
zostaje zamknięty.

Zasady, którymi kieruje się klient:
- Przestrzega regulaminu parku i danej atrakcji;
- Może w dowolnym momencie przerwać korzystanie z atrakcji A1-A5, A13-A17 (ok. 10%);
- Nie może zrezygnować w trakcie trwania atrakcji A6-A12;
- Klient może z danej atrakcji korzystać wielokrotnie (ok. 5%);
- Jeżeli klient (w tym VIP) korzysta z restauracji w czasie pobytu w parku, za konsumpcję płaci
w kasie przy wyjściu z parku (ok. 15%);
- Jeżeli korzysta z restauracji przed lub po wyjściu z parku rozrywki płaci za konsumpcję w
kasie restauracji (ok.10%);
- Jeżeli limit czasu (pobytu w parku) Ti zostanie przekroczony, klient przy wyjściu musi dopłacić
różnicę proporcjonalnie do pierwotnego limitu – nie dotyczy osób VIP (dopłata o 100%
wyższa niż podstawowa cena);
Zasady działania pracownika obsługi:
- uruchamia daną atrakcję; kontroluje czas i bezpieczeństwo klientów;
- pracownik wpuszcza w danej turze określoną liczbę osób do danej atrakcji – jeżeli liczba osób
jest mniejsza niż liczba miejsc (kolejka pusta) dana atrakcja jest uruchamiana z bieżącą
ilością klientów;
- każdy pracownik parku zna PID-y klientów, którzy w danym momencie korzystają z atrakcji,
którą on nadzoruje.
- Jeżeli klient zrezygnował z danej atrakcji (A1-A5, A13-A17) w trakcie jej trwania na jego
miejsce nie jest wpuszczana nowa osoba;

Napisz procedury Kierownik, Kasa, Kasa restauracji, Pracownik obsługi i Klient symulujące działanie
parku rozrywki. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.