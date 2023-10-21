#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

using namespace std;

// Structure to hold lines and file index
struct LineWithIndex {
    string line;
    int fileIndex;
};

// Function to perform external merge sort
void externalMergeSort(const vector<string>& inputFileNames, const string& outputFileName) {
    priority_queue<LineWithIndex, vector<LineWithIndex>, greater<LineWithIndex>> minHeap;

    // Open input files and initialize the min-heap
    vector<ifstream> inputFiles;
    for (int i = 0; i < inputFileNames.size(); ++i) {
        inputFiles.emplace_back(inputFileNames[i]);
        string line;
        if (getline(inputFiles[i], line)) {
            minHeap.push({line, i});
        }
    }

    // Open the output file
    ofstream outputFile(outputFileName);

    // Merge the files
    while (!minHeap.empty()) {
        LineWithIndex top = minHeap.top();
        minHeap.pop();
        outputFile << top.line << endl;

        if (getline(inputFiles[top.fileIndex], top.line)) {
            minHeap.push(top);
        }
    }

    // Close input and output files
    for (int i = 0; i < inputFileNames.size(); ++i) {
        inputFiles[i].close();
    }
    outputFile.close();
}

// Function to recursively find files in a directory
void findFilesInDirectory(const fs::path& dirPath, vector<string>& fileNames) {
    fs::directory_iterator end_iter;

    for (fs::directory_iterator dir_itr(dirPath); dir_itr != end_iter; ++dir_itr) {
        if (fs::is_regular_file(dir_itr->status())) {
            fileNames.push_back(dir_itr->path().string());
        } else if (fs::is_directory(dir_itr->status())) {
            findFilesInDirectory(dir_itr->path(), fileNames);
        }
    }
}

int main() {
    string directoryPath = "postings";
    vector<string> inputFileNames;
    vector<string> allFileNames;

    // Find all text files in the directory and its subdirectories
    findFilesInDirectory(directoryPath, allFileNames);

    for (const string& fileName : allFileNames) {
        if (fileName.find("postings_") != string::npos) {
            inputFileNames.push_back(fileName);
        }
    }

    string outputFileName = "mergedPostings.txt";

    externalMergeSort(inputFileNames, outputFileName);

    return 0;
}
