#pragma once
#define PARK_QUEUE_ID "/park-"
/// in ubuntu-based systems default max queue size is 10
#define DEFAULT_MAX_MSQ_SIZE 10

#define SHARED_MEMORY_PATH "/tmp/park_rozrywki_shm_main"
#define SHARED_MEMORY_KEY 'A'

#define MAIN_SEMAPHORE_ARRAY_PATH "/tmp/park_rozrywki_sem_main"
#define MAIN_SEMAPHORE_ARRAY_KEY 'B'

#define ATTRACTION_COUNT 17
#define RESTAURANT_INDEX ATTRACTION_COUNT - 1
#define LOGGER_FILE_FLAGS O_CREAT | O_WRONLY | O_TRUNC

#define LOGGER_PATH "/tmp/park-logger"
#define LOGGER_MESSAGE_MAX_LENGTH 256

#define MAIN_LOG_PATH "main.log"
#define CASHIER_LOG_PATH "cashier.log"
#define ATTRACTION_LOG_PATH "attraction.log"
#define RESTAURANT_LOG_PATH "restaurant.log"
#define CLIENT_LOG_PATH "client.log"

constexpr int PARK_SIZE = 200;
constexpr float VIP_PROB = 0.01;
constexpr float CHILD_PROB = 0.6;
constexpr int CLIENT_SPAWN_TIME_MIN = 25000;
constexpr int CLIENT_SPAWN_TIME_MAX = 250000;
constexpr int CLIENT_MQ_TIMEOUT = 10;
constexpr int DEFAULT_MQ_TIMEOUT = 2;

constexpr const char* logFileNames[5] = {
    MAIN_LOG_PATH,
    CASHIER_LOG_PATH,
    ATTRACTION_LOG_PATH,
    RESTAURANT_LOG_PATH,
    CLIENT_LOG_PATH
};

/// CONFIG CHECK
static_assert(CLIENT_MQ_TIMEOUT >= 0, "czas timeoutu klienta w kolejce jest mniejszy od zera!");
static_assert(DEFAULT_MQ_TIMEOUT >= 0, "czas timeoutu kas/atrakcji jest mniejszy od zera!");

static_assert(CLIENT_SPAWN_TIME_MIN > 0, "minimalny czas tworzenia klientow musi byc wiekszy od 0");
static_assert(CLIENT_SPAWN_TIME_MIN <= CLIENT_SPAWN_TIME_MAX, "minimalny czas tworzenia klientow jest wiekszy od maksymalnego czasu");
static_assert(PARK_SIZE > 0, "rozmiar parku musi byc wiekszy od 0");

static_assert(VIP_PROB <= 1, "prawdopodobienstwo bycia klientem vip musi byc mniejsze lub rowne 1");
static_assert(CHILD_PROB <= 1, "prawdopodobienstwo bycia klientem vip musi byc mniejsze lub rowne 1");

static_assert(VIP_PROB >= 0, "prawdopodobienstwo bycia klientem vip musi byc wieksze lub rowne 0");
static_assert(CHILD_PROB >= 0, "prawdopodobienstwo bycia klientem vip musi byc wieksze lub rowne 0");

static_assert(DEFAULT_MAX_MSQ_SIZE > 0, "rozmiar kolejek komunikatow musi byc wiekszy od 0");
static_assert(LOGGER_MESSAGE_MAX_LENGTH > 0, "dlugosc wiadomosci loggera musi byc wieksza od 0");