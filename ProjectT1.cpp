#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

// Function to normalize words by converting to uppercase and removing punctuation
string normalize(const string& word) {
    string normalized;
    for (char c : word) {
        if (isalnum(c)) { // Include only alphanumeric characters
            normalized += toupper(c); // Convert to uppercase
        }
    }
    return normalized;
}

// Function to read a file and get normalized word counts, and return top 100 frequent words
unordered_map<string, double> getTop100FrequentWords(const string& filename, int& totalWords) {
    unordered_map<string, int> wordCounts;
    ifstream file(filename);
    string word;
    totalWords = 0;

    // Common words to remove (in uppercase format)
    unordered_set<string> removeWords = {"A", "AND", "AN", "OF", "IN", "THE"};

    while (file >> word) {
        string normalizedWord = normalize(word); // Normalize the word
        if (removeWords.find(normalizedWord) == removeWords.end()) { // Only count words that are not in removeWords
            wordCounts[normalizedWord]++;
            totalWords++;
        }
    }

    // Normalize frequencies and keep track of the top 100 frequent words
    unordered_map<string, double> normalizedCounts;
    vector<pair<string, int>> wordCountVector(wordCounts.begin(), wordCounts.end());
    
    // Sort words by their frequency in descending order
    sort(wordCountVector.begin(), wordCountVector.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
    });

    // Keep top 100 words and normalize their frequencies
    int count = 0;
    for (const auto& pair : wordCountVector) {
        if (count >= 100) break;
        normalizedCounts[pair.first] = static_cast<double>(pair.second) / totalWords;
        count++;
    }

    return normalizedCounts;
}

// Function to calculate the similarity index between two normalized word count maps
double SimilarityIndex(const unordered_map<string, double>& count1, const unordered_map<string, double>& count2) {
    double similarityIndex = 0.0;

    for (const auto& pair : count1) {
        const string& word = pair.first;
        auto it = count2.find(word);
        if (it != count2.end()) {
            // Sum normalized values
            similarityIndex += (pair.second + it->second);
        }
    }

    return similarityIndex;
}

// Structure to hold pairs of similar textbooks and their similarity index
struct SimilarityPair {
    int index1;
    int index2;
    double similarity;
};

// Comparator function to sort similarity pairs
bool compareSimilarity(const SimilarityPair& a, const SimilarityPair& b) {
    return a.similarity > b.similarity;
}

int main() {
    const int SIZE = 64; // Number of text files
    unordered_map<string, double> wordCounts[SIZE];
    double similarityMatrix[SIZE][SIZE] = {0};

    // Load files and calculate normalized word counts for top 100 frequent words
    for (int i = 0; i < SIZE; ++i) {
        string filename = "file" + to_string(i + 1) + ".txt"; // e.g., file1.txt, file2.txt, ...
        int totalWords = 0;
        wordCounts[i] = getTop100FrequentWords(filename, totalWords);
    }

    // Calculate the similarity matrix
    for (int i = 0; i < SIZE; ++i) {
        for (int j = i + 1; j < SIZE; ++j) {
            double similarityIndex = SimilarityIndex(wordCounts[i], wordCounts[j]);
            similarityMatrix[i][j] = similarityIndex;
            similarityMatrix[j][i] = similarityIndex; // Symmetric matrix
        }
    }

    // Create a vector to hold similarity pairs
    vector<SimilarityPair> similarPairs;

    // Collect pairs with their similarity scores, excluding self-similarity
    for (int i = 0; i < SIZE; ++i) {
        for (int j = i + 1; j < SIZE; ++j) {
            if (similarityMatrix[i][j] > 0) { // Consider only non-zero similarities
                similarPairs.push_back({i, j, similarityMatrix[i][j]});
            }
        }
    }

    // Sort pairs by similarity
    sort(similarPairs.begin(), similarPairs.end(), compareSimilarity);



    // Output the similarity matrix to an output file
    ofstream matrixFile("similarity_matrix.txt");
    if (!matrixFile) {
        cerr << "Error in opening similarity matrix file for writing." << endl;
        return 1;
    }

    // Write the matrix to the file
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            matrixFile << fixed << setprecision(4) << similarityMatrix[i][j] << " ";
        }
        matrixFile << endl; // New line after each row
    }

        // Output the top ten similar pairs
    cout << "Top 10 Similar Pairs:\n";
    for (size_t i = 0; i < min(similarPairs.size(), size_t(10)); ++i) {
        cout << "File " << similarPairs[i].index1 + 1 << " and File " << similarPairs[i].index2 + 1 
             << ": Similarity = " << fixed << setprecision(4) << similarPairs[i].similarity << "\n";
    }

    matrixFile.close();

    return 0;
}
