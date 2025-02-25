
# **README: MapReduce Word Count Program**

## **Overview**

This program is a multithreaded implementation of a simplified MapReduce framework in C++. It processes a set of text files to count the occurrences of words, grouped by the first letter of each word. Results are stored in separate output files, one per letter of the alphabet.

### **Features**

-   **Mapper Threads**: Parse words from files and normalize them (convert to lowercase, remove non-alphabetic characters).
-   **Reducer Threads**: Group and sort words by the first letter, count occurrences, and write results to files.
-   **Thread Synchronization**: Ensures thread-safe operations using `pthread` mutexes and barriers.

----------

## **File Structure**

-   **`MapReduce.h`**: Contains data structures and utility function prototypes.
-   **`MapReduce.cpp`**: The main implementation of the MapReduce program.

----------

## **How It Works**

1.  **Input Files**:
    
    -   The program reads a list of file paths from an input file.
    -   Each file is associated with a unique ID.
2.  **Mapper Phase**:
    
    -   Mapper threads process the input files:
        -   Normalize words.
        -   Map each word to the file ID where it appears.
    -   Results are stored in a shared data structure prepared for the reducers.
3.  **Reducer Phase**:
    
    -   Reducer threads:
        -   Take each letter of the alphabet from a shared queue.
        -   Aggregate words starting with the assigned letter from the mapper results.
        -   Sort words by their frequency (descending) and alphabetically (ascending) when frequencies are equal.
        -   Write the results to a text file named after the letter (`a.txt`, `b.txt`,...).

----------
