#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <filesystem>
#include <unordered_map>  // Added for the hashtable

namespace fs = std::filesystem;

using namespace std;

// Structure to hold lines and file index
struct LineWithIndex {
    string line;
    int fileIndex;
};

// Custom comparison function for LineWithIndex
bool lineWithIndexCompare(const LineWithIndex& a, const LineWithIndex& b) {
    return a.line > b.line; // Change to '<' for min-heap, '>' for max-heap
}

// Function to perform external merge sort
void externalMergeSort(const vector<string>& inputFileNames, const vector<string>& outputFiles) {
    string outputFolderPath = "mergePostings/";

    // Create the output folder if it doesn't exist
    if (!fs::exists(outputFolderPath)) {
        fs::create_directory(outputFolderPath);
    }

    priority_queue<LineWithIndex, vector<LineWithIndex>, decltype(&lineWithIndexCompare)> minHeap(lineWithIndexCompare);

    // Open input files and initialize the min-heap
    vector<ifstream> inputFiles;
    for (int i = 0; i < inputFileNames.size(); ++i) {
        inputFiles.emplace_back(inputFileNames[i]);
        string line;
        if (getline(inputFiles[i], line)) {
            minHeap.push({line, i});
        }
    }

    // Merge the files
    while (!minHeap.empty()) {
        LineWithIndex top = minHeap.top();
        minHeap.pop();

        // Determine the output file index based on the starting letter of the line
        char startingLetter = tolower(top.line[0]);
        int outputFileIndex;

        string outputFilePath = outputFolderPath + outputFiles[0];

        ofstream outputFile(outputFilePath, ios_base::app); // Open in append mode
        // std::cout << "Read to file: " << top.line << std::endl;
        outputFile << top.line << endl;

        if (getline(inputFiles[top.fileIndex], top.line)) {
            minHeap.push(top);
        }

        outputFile.close();
    }

    // Close input files
    for (int i = 0; i < inputFileNames.size(); ++i) {
        inputFiles[i].close();
    }
}

// Function to create and save the hashtable to a file
void createAndSaveHashtable(const string& inputFilePath, const string& outputFilePath) {
    ifstream inputFile(inputFilePath);
    unordered_map<string, pair<int, int>> hashtable; // word -> {frequency, line number}

    // Read lines from the input file and process them
    int lineNumber = 1;
    string line;
    while (getline(inputFile, line)) {
        // Parse the line to extract word and data
            size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string word = line.substr(0, colonPos);
            string data = line.substr(colonPos); // Skip ': ' after colon

            // Count the number of tuples (frequency)
            int frequency = 0;
            for (char c : data) {
                if (c == '(') {
                    frequency++;
                }
            }

            // Store word, frequency, and line number in the hashtable
            hashtable[word] = {frequency, lineNumber};
        }
        lineNumber++;
    }

    inputFile.close();


    // Create a vector of pairs for sorting
    vector<pair<string, pair<int, int>> > sortedHashtable(hashtable.begin(), hashtable.end());

    // Sort the vector alphabetically by word
    sort(sortedHashtable.begin(), sortedHashtable.end(),
        [](const pair<string, pair<int, int>>& a, const pair<string, pair<int, int>>& b) {
            return a.first < b.first;
        }
    );

    // Save the sorted hashtable to the output file
    ofstream outputFile(outputFilePath);
    for (const auto& entry : sortedHashtable) {
        outputFile << entry.first << ", " << entry.second.first << " - " << entry.second.second << endl;
    }
    outputFile.close();
}

// Function to recursively find files in a directory
void findFilesInDirectory(const fs::path& dirPath, vector<string>& fileNames) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().filename().string().find("output_") != string::npos) {
            fileNames.push_back(entry.path().string());
        }
    }
}

int main() {
    string directoryPath = "sortedPostings/";
    vector<string> inputFileNames;
    vector<string> allFileNames;

    // Find all text files in the directory and its subdirectories
    findFilesInDirectory(directoryPath, allFileNames);

    for (const string& fileName : allFileNames) {
        if (fileName.find("output_") != string::npos) {
            inputFileNames.push_back(fileName);
        }
    }

    // Define the output files based on starting letters
    vector<string> outputFiles = {
        "postings_inverted.txt", // Lines starting with misc characters and numbers
    };

    externalMergeSort(inputFileNames, outputFiles);

    string outputFolderPath = "hashtable/";
    string fileName = "hashtable.txt";
    string filePath = outputFolderPath + fileName;

    // Create the output folder if it doesn't exist
    if (!fs::exists(outputFolderPath)) {
        fs::create_directory(outputFolderPath);
    }

    // Create and save the hashtable
    createAndSaveHashtable("mergePostings/"+ outputFiles[0], filePath);

    return 0;
}
