#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <filesystem>

namespace fs = std::__fs::filesystem;
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
    string outputFolderPath = "sortedPostingsAlphabetically/";

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

        switch (startingLetter) {
            case 'a': 
                outputFileIndex = 1;
                break;
            case 'b':
                outputFileIndex = 2;
                break;  
            case 'c':
                outputFileIndex = 3;
                break;
            case 'd':
                outputFileIndex = 4;
                break;
            case 'e':
                outputFileIndex = 5;
                break;
            case 'f':
                outputFileIndex = 6;
                break;
            case 'g':
                outputFileIndex = 7;
                break;
            case 'h':
                outputFileIndex = 8;
                break;
            case 'i':
                outputFileIndex = 9;
                break;
            case 'j':
                outputFileIndex = 10;
                break;
            case 'k':
                outputFileIndex = 11;
                break;
            case 'l':
                outputFileIndex = 12;
                break;
            case 'm':
                outputFileIndex = 13;
                break;
            case 'n':
                outputFileIndex = 14;
                break;
            case 'o':
                outputFileIndex = 15;
                break;
            case 'p':
                outputFileIndex = 16;
                break;
            case 'q':
                outputFileIndex = 17;
                break;
            case 'r':
                outputFileIndex = 18;
                break;
            case 's':
                outputFileIndex = 19;
                break;
            case 't':
                outputFileIndex = 20;
                break;
            case 'u':
                outputFileIndex = 21;
                break;
            case 'v':
                outputFileIndex = 22;
                break;
            case 'w':
            case 'x':
            case 'y':
            case 'z':
                outputFileIndex = 23;
                break;
            default:
                outputFileIndex = 0;
                break;
        }

        string outputFilePath = outputFolderPath + outputFiles[outputFileIndex];
        ofstream outputFile(outputFilePath, ios_base::app); // Open in append mode
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

// Function to recursively find files in a directory
void findFilesInDirectory(const fs::path& dirPath, vector<string>& fileNames) {
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().filename().string().find("postings_") != string::npos) {
            fileNames.push_back(entry.path().string());
        }
    }
}

int main() {
    string directoryPath = "unfilteredPostings/";
    vector<string> inputFileNames;
    vector<string> allFileNames;

    // Find all text files in the directory and its subdirectories
    findFilesInDirectory(directoryPath, allFileNames);

    // Define the output files based on starting letters
    vector<string> outputFiles = {
        "postings_misc.txt", // Lines starting with misc characters and numbers
        "postings_a.txt",  // Lines starting with a
        "postings_b.txt",  // Lines starting with b
        "postings_c.txt",  // Lines starting with c
        "postings_d.txt",  // Lines starting with d
        "postings_e.txt",  // Lines starting with e
        "postings_f.txt",  // Lines starting with f
        "postings_g.txt",  // Lines starting with g
        "postings_h.txt",  // Lines starting with h
        "postings_i.txt",  // Lines starting with i
        "postings_j.txt",  // Lines starting with j
        "postings_k.txt",  // Lines starting with k
        "postings_l.txt",  // Lines starting with l
        "postings_m.txt",  // Lines starting with m
        "postings_n.txt",  // Lines starting with n
        "postings_o.txt",  // Lines starting with o
        "postings_p.txt",  // Lines starting with p
        "postings_q.txt",  // Lines starting with q    
        "postings_r.txt",  // Lines starting with r
        "postings_s.txt",  // Lines starting with s
        "postings_t.txt",  // Lines starting with t
        "postings_u.txt",  // Lines starting with u
        "postings_v.txt",  // Lines starting with v
        "postings_w_z.txt",  // Lines starting with w, x, y, z
        
    };

    externalMergeSort(inputFileNames, outputFiles);

    return 0;
}
