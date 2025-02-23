#include "MapReduce.h"

// Function to parse a word and return it in a normalized form
string parse_word(string word) {
    string parsed_word;
    for (char c : word) {
        // If the character is a letter, add it to the parsed word in lowercase
        if (isalpha(c)) {
            parsed_word += tolower(c);
        }
    }
    return parsed_word;
}

void *mapper(void *arg)
{
    mapper_input *input = (mapper_input *)arg;

    // Read from the files queue and parse the words
    while (true) {
        pthread_mutex_lock(input->mutex);
        // Check if the queue is empty and exit the thread if it is
        if (input->files->empty()) {
            pthread_mutex_unlock(input->mutex);
            break;
        }
        // Get the file details from the queue
        pair<string, int> file = input->files->front();
        input->files->pop();
        pthread_mutex_unlock(input->mutex);
        
        unordered_map<string, set<int>> file_words_count;

        // Read the file word by word, parse them and add them to the file_words_count map
        ifstream in_file(file.first);
        string word;
        while (in_file >> word)
        {
            file_words_count[parse_word(word)].insert(file.second);
        }
        in_file.close();

        pthread_mutex_lock(input->mutex);
        // Add the words to the shared vector
        input->files_words_count->push_back(file_words_count);
        pthread_mutex_unlock(input->mutex);
    }

    // Wait for the other mapper threads to finish so the reducers can start
    pthread_barrier_wait(input->barrier);
    return NULL;
}

void *reducer(void *arg)
{
    reducer_input *input = (reducer_input *)arg;

    // Wait for the mappers to finish
    pthread_barrier_wait(input->barrier);

    // Read the words from the shared vector, group them by letter,
    // sort them and write them to the output files
    while (true)
    {
        pthread_mutex_lock(input->mutex);
        // Check if their are any letters left to process and exit the thread if there are none
        if (input->letter_queue->empty())
        {
            pthread_mutex_unlock(input->mutex);
            break;
        }
        // Get the letter from the queue
        char letter = input->letter_queue->front();
        input->letter_queue->pop();
        pthread_mutex_unlock(input->mutex);

        // Group the words by letter
        unordered_map<string, set<int>> words_count_per_letter;
        for (auto &file_words_count : *input->files_words_count)
        {
            for (auto &word : file_words_count)
            {
                if (word.first[0] == letter)
                {
                    words_count_per_letter[word.first].insert(word.second.begin(), word.second.end());
                }
            }
        }

        // Put the words in a vector to be easier to sort
        vector<pair<string, set<int>>> sorted_words;
        for (auto &word : words_count_per_letter)
        {
            sorted_words.push_back({word.first, word.second});
        }

        // Sort the words by the number of files they appear in and by the word itself if equal
        sort(sorted_words.begin(), sorted_words.end(), [](auto &left, auto &right) {
            if (left.second.size() == right.second.size())
            {
                return left.first < right.first;
            }
            return left.second.size() > right.second.size();
        });

        // Write the words to the output file of the letter
        ofstream out_file(string(1, letter) + ".txt");
        for (auto word : sorted_words)
        {
            // Write the word and the files it appears in
            out_file << word.first << ":[";
            for (auto file : word.second)
            {
                out_file << file;
                // Add a space if the file is not the last one
                if (file != *word.second.rbegin())
                {
                    out_file << " ";
                }
            }
            out_file << "]" << endl;
            
        }
        out_file.close();

    }

    return NULL;
}

int main(int argc, char **argv)
{
    int nr_mappers = atoi(argv[1]);
    int nr_reducers = atoi(argv[2]);
    string in_file_name = argv[3];

    ifstream in_file(in_file_name);
    string line;
    // Read the number of files
    int nr_files;
    if (getline(in_file, line))
    {
        nr_files = atoi(line.c_str());
    }

    // Read the files names and put them in a queue for the mappers
    queue<pair<string, int>> files_queue;
    int id = 1;
    while (getline(in_file, line))
    {
        files_queue.push({line, id});
        id++;
    }

    // Initialize the threads, mutexes, barriers and the shared files_words_count vector
    pthread_t threads[nr_mappers + nr_reducers];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, nr_mappers + nr_reducers);
    vector<unordered_map<string, set<int>>> files_words_counts;

    // Initialize the letter queue for the reducers
    queue<char> letter_queue;
    for (char c = 'a'; c <= 'z'; c++)
    {
        letter_queue.push(c);
    }

    // Create the threads
    for (int i = 0; i < nr_mappers + nr_reducers; i++)
    {
        if (i < nr_mappers)
        {
            // Create a mapper input struct and pass it to the mapper thread
            mapper_input *input = new mapper_input;
            input->mutex = &mutex;
            input->barrier = &barrier;
            input->files = &files_queue;
            input->files_words_count = &files_words_counts;
            pthread_create(&threads[i], NULL, mapper, input);
        }
        else
        {
            // Create a reducer input struct and pass it to the reducer thread
            reducer_input *input = new reducer_input;
            input->mutex = &mutex;
            input->barrier = &barrier;
            input->files_words_count = &files_words_counts;
            input->letter_queue = &letter_queue;
            pthread_create(&threads[i], NULL, reducer, input);
        }
    }

    // Join the threads
    for (int i = 0; i < nr_mappers + nr_reducers; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destroy the mutex and the barrier
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);

    // Close the input file
    in_file.close();
    return 0;
}