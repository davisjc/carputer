
#include "logger.hpp"

#include <iostream>
#include <pthread.h>

static pthread_mutex_t mutex_output;

void
logger::log(std::string msg) {
    pthread_mutex_lock(&mutex_output);
    std::cout << msg << std::endl;
    pthread_mutex_unlock(&mutex_output);
}

void
logger::error(std::string msg) {
    pthread_mutex_lock(&mutex_output);
    std::cerr << msg << std::endl;
    pthread_mutex_unlock(&mutex_output);
}

