#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>

namespace fs = std::__fs::filesystem;

// Function to write documents to a new text file in the "data" folder
void writeDocumentsToFile(const std::vector<std::string>& documents, int fileIndex) {
    const std::string folderName = "data";
    if (!fs::is_directory(folderName)) {
        fs::create_directory(folderName); // Create the "data" folder if it doesn't exist
    }

    std::string file = "/output_" + std::to_string(fileIndex) + ".txt";
    std::string filename = folderName + file;
    std::ofstream outputFile(filename);

    if (outputFile.is_open()) {
        // Write each document to the file
        for (const std::string& doc : documents) {
            outputFile << doc << std::endl;
        }
        outputFile.close();
        std::cerr << "Wrote to file: " << file << std::endl;
    } else {
        std::cerr << "Failed to open the output file: " << filename << std::endl;
    }
}

// Function to split a string into words based on delimiters
int splitString(const std::string& input, const std::string& delimiters) {
    std::vector<std::string> tokens;
    std::string word;
    std::istringstream iss(input);
    int count = 0;

    char c;
    while (iss.get(c)) {
        if (delimiters.find(c) == std::string::npos) {
            word += c;
        } else {
            if (!word.empty()) {
                count++;
                tokens.push_back(word);
                word.clear();
            }
        }
    }

    if (!word.empty()) {
        count++;
        tokens.push_back(word);
    }

    return count;
}

int main() {
    // Specify the path to the input .trec file
    const std::string trecFilename = "/Users/shreyasmac/Documents/VS Code/inverted-index/msmarco-docs.trec";
    const int documentsPerFile = 1500; // Number of documents to store in each output file
    std::vector<std::string> documents; // Temporary storage for documents
    int documentCount = 0; // Count of documents processed
    int documentID = 0;
    int fileIndex = 0; // Index for naming output files
    const std::string delimiters = ".,?!:;()\"'[]/{}() +#-><$%&*|~";

    std::string documentPositions = "documentPositions"; 
    std::string documentPositionsFile = documentPositions + "/" + "documentPositions.txt"; // Text file for word positions
    std::ofstream documentPositionsOutputFile(documentPositionsFile);

    std::ifstream trecFile(trecFilename);
    std::unordered_map<int, std::pair<int, std::streampos>> documentPointer;

    if (!trecFile.is_open()) {
        std::cerr << "Failed to open the .trec file." << std::endl;
        return 1;
    }

    std::string line;
    bool isInsideDocument = false;

    while (std::getline(trecFile, line)) {
        if (line.find("<DOC>") != std::string::npos) {
            documentID++;
            documentPointer[documentID].second = trecFile.tellg();
            isInsideDocument = true;
            documents.push_back(line);
            documentPointer[documentID].first = documentPointer[documentID].first + splitString(line, delimiters);
        } else if (isInsideDocument) {
            documents.push_back(line); // Add lines to the current document
            documentPointer[documentID].first = documentPointer[documentID].first + splitString(line, delimiters);
        }

        if (line.find("</DOC>") != std::string::npos) {
            documentCount++;

            if (documentCount >= documentsPerFile) {
                // Write the documents to a new file and reset the temporary storage
                writeDocumentsToFile(documents, fileIndex);
                documents.clear();
                documentCount = 0;
                fileIndex++;
                isInsideDocument = false;
            }
        }
    }

    // Check if there are any remaining documents and write them to a file
    if (!documents.empty()) {
        writeDocumentsToFile(documents, fileIndex);
    }

    // Save sorted word positions to a text file
        for (const auto& pair : documentPointer) {
            documentPositionsOutputFile << pair.first << "," << pair.second.first << ":" << pair.second.second << '\n';
        }
        documentPositionsOutputFile.close();

    trecFile.close();

    return 0;
}
