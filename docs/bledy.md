# Błędy
### Maksymalna dozwolona liczba kolejek komunikatów w systemie
- w dystrybucjach Ubuntu i pochodnych domyślna maksymalna liczba kolejek komunikatów POSIX (`mq_open`) to 256, co powoduje, że procesy klientów wielokrotnie nie są w stanie utworzyć osobnej kolejki na odpowiedź
- aktualny limit można sprawdzić przez `sysctl fs.mqueue.queues_max`
- limit można tymczasowo zmienić poleceniem `sysctl -w fs.mqueue.queues_max=<nowa wartosc>` (wymaga uprawnień roota)

Jednak dalej może wystąpić `EMFILE`, który nie pozwala klientowi stworzyć kolejki do odpowiedzi - przekroczony limit deskryptorów w procesie.

### Brak przestrzegania priorytetu VIP
- jeżeli kolejka komunikatów kasy jest zapełniona komunikatami zwykłych klientów, a do wysłania wiadomości będzie czekał klient VIP, to najpierw zostanie obsłużony klient bez statusu VIP
