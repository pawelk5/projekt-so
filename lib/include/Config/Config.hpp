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
#define PARK_SIZE 100

#define LOGGER_PATH "/tmp/park-logger"
#define OUTPUT_PATH "log.txt"
#define LOGGER_MESSAGE_MAX_LENGTH 256