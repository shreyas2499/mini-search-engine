#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <chrono>
#include <filesystem>

namespace fs = std::__fs::filesystem;

// Define constants for maximum term and URL lengths
const int MAX_TERM_LENGTH = 256;
const int MAX_URL_LENGTH = 1024;

// Define a data structure for a document entry
struct Document {
    std::string term;
    int docID;
};

// Define a data structure for the posting list
struct PostingList {
    std::map<int, int> documentFrequencies;
};

int documentID = -1;
int fileIndex = 0;

// Function to remove specified characters from the beginning and end of a string
std::string removeCharactersFromBothEnds(const std::string& input) {
    const char charactersToRemove[] = { '.', ',', '?', '!', ':', ';', '-', ' ', ')', '(', '"','\'', '[', ']', '/'};

    std::string result = input;

    // Remove characters from the beginning
    size_t firstValidChar = result.find_first_not_of(charactersToRemove);
    if (firstValidChar != std::string::npos) {
        result = result.substr(firstValidChar);
    } else {
        result.clear();
    }

    // Remove characters from the end
    size_t lastValidChar = result.find_last_not_of(charactersToRemove);
    if (lastValidChar != std::string::npos) {
        result.erase(lastValidChar + 1);
    } else {
        result.clear();
    }

    return result;
}

// Function to compress an integer using VarByte and append the result to a vector
void VarEncodeTermPosition(int termPosition, std::vector<uint8_t>& encodedPositions) {
    while (termPosition > 127) {
        encodedPositions.push_back(static_cast<uint8_t>((termPosition & 127) + 128));
        termPosition = termPosition >> 7;
    }
    encodedPositions.push_back(static_cast<uint8_t>(termPosition));
}

void VarEncode(int number, std::vector<uint8_t>& encoded) {
    while (number >= 128) {
        uint8_t encodedValue = static_cast<uint8_t>((number & 127) | 128);
        encoded.push_back(encodedValue);
        encoded.push_back(' ');
        number >>= 7;
    }
    uint8_t finalValue = static_cast<uint8_t>(number);
    encoded.push_back(finalValue);
}

// Function to add an entry to the posting list
void addToPostingList(std::map<std::string, PostingList>& postingLists, const std::string& term, int docID) {
    if (postingLists.find(term) == postingLists.end()) {
        PostingList newPostingList;
        newPostingList.documentFrequencies[docID] = 1;
        postingLists[term] = newPostingList;
    } else {
        postingLists[term].documentFrequencies[docID]++;
    }
}

// Function to print the posting lists
void writePostingListsToFile(std::ofstream& outputFile, const std::map<std::string, PostingList>& postingLists, std::string fileName) {
    for (const auto& posting : postingLists) {
        outputFile << posting.first << ": ";
        for (const auto& docInfo : posting.second.documentFrequencies) {
            outputFile << "(" << docInfo.first << ", " << docInfo.second << ") ";
        }
        outputFile << std::endl;
    }

    std::cout << "Postings have been written to " << fileName << std::endl;

    outputFile.close();
}

void outputDocIDToUrlMapToFile(const std::map<int, std::string>& docIDToUrlMap) {
    std::ofstream outputFile("docIDToUrlMapping.txt", std::ios::out | std::ios::trunc);
    if (outputFile.is_open()) {
        for (const auto& entry : docIDToUrlMap) {
            outputFile << entry.first << ": " << entry.second << std::endl;
        }

        std::cout << "DocID to URL mapping has been written to " << "docIDToUrlMapping.txt" << std::endl;
    } else {
        std::cerr << "Failed to open the output file: " << "docIDToUrlMapping.txt" << std::endl;
    }
    outputFile.close();
}

bool skipWord(std::string word){
    return word == std::string(".") || word == std::string(",") || word == std::string("?") || word == std::string("!") ||
    word == std::string(":") || word == std::string(";") || word == std::string("-") || word == std::string(" ") ||
    word == std::string(")") || word == std::string("(") || word == std::string("\"") || word == std::string("/");
}


// Function to parse a .txt file and create intermediate posting lists
void parseTxtFile(std::ifstream& txtFile, std::vector<std::string>& urls, std::vector<std::string>& documents, std::map<int, std::string>& docIDToUrlMap) {
    std::map<std::string, PostingList> postingLists;
    
    std::string line;
    std::string documentContent;
    std::string urlContent;
    int termPosition = 0;
    bool isInsideTextTag = false;
    int sizeCount = 0;

    while (std::getline(txtFile, line)) {
    
        if (line.find("<DOC>") != std::string::npos) {
            sizeCount++;
        }

        if (line.find("<TEXT>") != std::string::npos) {
            isInsideTextTag = true;
            urlContent.clear();
            documentContent.clear();
            documentID++;
            termPosition = 0;
            continue;
        }
        if (line.find("</TEXT>") != std::string::npos) {
            isInsideTextTag = false;
            continue;
        }

        if (isInsideTextTag) {
            if (urlContent.empty()) {
                urlContent += line;
                urls.push_back(line);
                docIDToUrlMap[documentID] = line;
            } else {
                std::istringstream iss(line);
                std::vector<std::string> words;
                std::string word;
                while (iss >> word) {
                    if(skipWord(word)){
                        continue;
                    }
                    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                    addToPostingList(postingLists, removeCharactersFromBothEnds(word), documentID);
                    termPosition++;
                }
            }
        }
    
    }


    const std::string folderName = "postings";
    if (!fs::is_directory(folderName)) {
        fs::create_directory(folderName); // Create the "data" folder if it doesn't exist
    }

    std::string file = "/postings_" + std::to_string(fileIndex) + ".txt";
    std::string filepath = folderName + file;
    std::string filename = "postings_" + std::to_string(fileIndex) + ".txt";

    std::ofstream outputFile(filepath, std::ios::out | std::ios::trunc);
    // Output the posting lists and docIDToUrlMap
    writePostingListsToFile(outputFile, postingLists, filename);
    outputFile.close();
    outputDocIDToUrlMapToFile(docIDToUrlMap);
}

int main() {
    const std::string dataDirectory = "data"; // Set the path to your data folder
    std::map<int, std::string> docIDToUrlMap;

    auto startTime = std::chrono::high_resolution_clock::now();

    
    // Use std::filesystem to recursively iterate over .txt files in the data folder
    for (const auto& entry : std::__fs::filesystem::recursive_directory_iterator(dataDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == std::string(".txt")) {
            // Open the .txt file for processing
            std::ifstream txtFile(entry.path().string());
            
            if (!txtFile.is_open()) {
                std::cerr << "Failed to open the .txt file: " << entry.path() << std::endl;
                continue;
            }

            std::vector<std::string> urls;
            std::vector<std::string> documents;

            while (!txtFile.eof()) {
                parseTxtFile(txtFile, urls, documents, docIDToUrlMap);
                
                urls.clear();
                documents.clear();
            }

            txtFile.close();
            fileIndex++;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    

    return 0;
}