#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <regex>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::__fs::filesystem;

// Function to encode an integer using VarByte and return binary representation
std::string varByteEncode(int number) {
    std::vector<unsigned char> encodedBytes;

    while (number > 0) {
        // Encode 7 bits at a time (bottom 7 bits of the integer)
        unsigned char byte = number & 0x7F;
        // Set the high bit to 1 to indicate more bytes or 0 to indicate the last byte
        if (number > 0x7F) {
            byte |= 0x80;
        }
        encodedBytes.push_back(byte);
        number >>= 7;
    }
    return std::string(encodedBytes.begin(), encodedBytes.end());
}

// Custom comparison function to sort pairs by key (word)
bool compareWordPairs(const std::pair<std::string, std::pair<int, std::streampos>>& a, const std::pair<std::string, std::pair<int, std::streampos>>& b) {
    return a.first < b.first;
}

int main() {
    // List of input file names
    std::vector<std::string> inputFiles = {"postings_inverted.txt"};

    for (const std::string& inputFileName : inputFiles) {
        std::ifstream inputFile("mergePostings/" + inputFileName);
        std::string outputFileName = inputFileName.substr(0, inputFileName.find_last_of('.')) + ".bin";
        std::string wordPositionsFolder = "wordPositions"; 
        std::string wordPositionsFile = wordPositionsFolder + "/" + inputFileName.substr(0, inputFileName.find_last_of('.')) + "_word_positions.txt"; // Text file for word positions

        std::string folderName = "compressed";

        if (!fs::is_directory(folderName)) {
            fs::create_directory(folderName); // Create the "compressed" folder if it doesn't exist
        }

        std::ofstream outputFile("compressed/" + outputFileName, std::ios::out | std::ios::binary);
        std::ofstream wordPositionsOutputFile(wordPositionsFile);

        if (!inputFile.is_open() || !outputFile.is_open()) {
            std::cerr << "Error: Could not open files." << std::endl;
            return 1;
        }

        std::string line;
        std::regex entryPattern("(#?\\w+):?\\s*\\((\\d+), (\\d+)\\)");

        int blockSize = 10;
        int count = 0;

        int number = 0;
        // Create a hashtable (unordered_map) to store word positions in the binary file
        std::unordered_map<std::string, std::pair<int, std::streampos>> wordPositions;
        

        while (std::getline(inputFile, line)) {
            size_t pos = line.find(":");
            int totalFrequency = 0;
            if (pos != std::string::npos) {
                std::string word = line.substr(0, pos);   
                number++;
          
                if (!word.empty()) {
                    // Record the current position in the binary file for this word
                    wordPositions[word].second = outputFile.tellp();

                    std::string rest = line.substr(pos + 1);
                    std::stringstream ss(rest);
                    char c;
                    int a, b;
                
                    while (ss >> c >> a >> c >> b >> c) {
                        int docID = a;
                        int frequency = b;
                        totalFrequency++;
                        // Encode the docID and frequency using VarByte
                        std::string binaryDocID = varByteEncode(docID);
                        std::string binaryFrequency = varByteEncode(frequency);

                        outputFile.write(binaryDocID.c_str(), binaryDocID.size());
                        outputFile.write(" ", 1);
                        outputFile.write(binaryFrequency.c_str(), binaryFrequency.size());
                        outputFile.write("|", 1);
                        count++;

                        if (count >= blockSize) {
                            count = 0;
                        }
                    }

                    // Store the frequency in the wordPositions
                    wordPositions[word].first = totalFrequency; // Frequency
                    outputFile.write("\n", 1);
                }
            }
        }

         // Convert wordPositions to a vector of pairs and sort it
        std::vector<std::pair<std::string, std::pair<int, std::streampos>>>
            sortedWordPositions(wordPositions.begin(), wordPositions.end());
        std::sort(sortedWordPositions.begin(), sortedWordPositions.end(), compareWordPairs);

        // Save sorted word positions to a text file
        for (const auto& pair : sortedWordPositions) {
            wordPositionsOutputFile << pair.first << "," << pair.second.first << ":" << pair.second.second << '\n';
        }
        wordPositionsOutputFile.close();

        inputFile.close();
        outputFile.close();     
        
        std::cout << "Number of words added" << wordPositions.size() << std::endl;
        std::cout << "Number of words traversed" << number << std::endl;

        std::cout << "VarByte encoding completed. Results saved to '" << outputFileName << "'." << std::endl;
        std::cout << "Word positions saved to '" << wordPositionsFile << "'." << std::endl;
    }

    return 0;
}