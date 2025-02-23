#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <queue>
#include <chrono>

using namespace std;

typedef struct
{
    pthread_mutex_t *mutex;
    pthread_barrier_t *barrier;
    queue<pair<string, int>> *files;
    vector<unordered_map<string, set<int>>> *files_words_count;
} mapper_input;

typedef struct
{
    pthread_mutex_t *mutex;
    pthread_barrier_t *barrier;
    vector<unordered_map<string, set<int>>> *files_words_count;
    queue<char> *letter_queue;
} reducer_input;

#endif