#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

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

int main() {
    // Specify the path to the input .trec file
    const std::string trecFilename = "/Users/shreyasmac/Documents/VS Code/inverted-index/msmarco-docs.trec";
    const int documentsPerFile = 1500; // Number of documents to store in each output file
    std::vector<std::string> documents; // Temporary storage for documents
    int documentCount = 0; // Count of documents processed
    int fileIndex = 0; // Index for naming output files

    std::ifstream trecFile(trecFilename);

    if (!trecFile.is_open()) {
        std::cerr << "Failed to open the .trec file." << std::endl;
        return 1;
    }

    std::string line;
    bool isInsideDocument = false;

    while (std::getline(trecFile, line)) {
        if (line.find("<DOC>") != std::string::npos) {
            isInsideDocument = true;
            documents.push_back(line);
        } else if (isInsideDocument) {
            documents.push_back(line); // Add lines to the current document
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

    trecFile.close();

    return 0;
}
