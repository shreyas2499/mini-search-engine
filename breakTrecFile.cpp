#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // For creating directories

namespace fs = std::filesystem;

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
    const std::string trecFilename = "D:\\Notes\\Web Search Engines\\inverted-index\\fulldocs-new.trec";  // Replace with your .trec file
    const int documentsPerFile = 1500;
    std::vector<std::string> documents;
    int documentCount = 0;
    int fileIndex = 0;

    std::ifstream trecFile(trecFilename);

    if (!trecFile.is_open()) {
        std::cerr << "Failed to open the .trec file." << std::endl;
        return 1;
    }

    std::string line;
    bool isInsideDocument = false;

    while (std::getline(trecFile, line)) {
        // std::cerr << line << std::endl;
        if (line.find("<DOC>") != std::string::npos) {
            isInsideDocument = true;
            documents.push_back(line);
            // documents.clear(); // Start a new set of documents
        } else if (isInsideDocument) {
            documents.push_back(line); // Add the line to the current document
        }

        if (line.find("</DOC>") != std::string::npos) {
            documentCount++;

            if (documentCount >= documentsPerFile) {
                writeDocumentsToFile(documents, fileIndex);
                documents.clear();
                documentCount = 0;
                fileIndex++;
                isInsideDocument = false;
            }
        }
    }

    // Check if there are any remaining documents
    if (!documents.empty()) {
        writeDocumentsToFile(documents, fileIndex);
    }

    trecFile.close();

    return 0;
}
