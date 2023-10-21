#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <filesystem>

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
    string outputFolderPath = "sortedPostings/";

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
            case 'b':
            case 'c':
                outputFileIndex = 0;
                break;
            case 'd':
            case 'e':
            case 'f':
                outputFileIndex = 1;
                break;
            case 'g':
            case 'h':
            case 'i':
                outputFileIndex = 2;
                break;
            case 'j':
            case 'k':
            case 'l':
                outputFileIndex = 3;
                break;
            case 'm':
            case 'n':
            case 'o':
                outputFileIndex = 4;
                break;
            case 'p':
            case 'q':
            case 'r':
                outputFileIndex = 5;
                break;
            case 's':
            case 't':
            case 'u':
                outputFileIndex = 6;
                break;
            default:
                outputFileIndex = 7;
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

    // Define the output files based on starting letters
    vector<string> outputFiles = {
        "output_a_c.txt",  // Lines starting with a, b, c
        "output_d_f.txt",  // Lines starting with d, e, f
        "output_g_i.txt",  // Lines starting with g, h, i
        "output_j_l.txt",  // Lines starting with j, k, l
        "output_m_o.txt",  // Lines starting with m, n, o
        "output_p_r.txt",  // Lines starting with p, q, r
        "output_s_u.txt",  // Lines starting with s, t, u
        "output_v_z.txt"   // Lines starting with v, w, x, y, z
    };

    externalMergeSort(inputFileNames, outputFiles);

    return 0;
}
