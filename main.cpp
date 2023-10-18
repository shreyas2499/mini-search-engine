#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

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
    int termFrequency;
    std::vector<std::pair<int, std::vector<uint8_t>>> documentIDs;
};

int documentID = -1;

// Function to remove specified characters from the beginning and end of a string
std::string removeCharactersFromBothEnds(const std::string& input) {
    const char charactersToRemove[] = { '.', ',', '?', '!', ':', ';', '-', ' ', ')', '(', '"' };

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
        // std::cout << "Encoded: " << static_cast<int>(encodedValue) << std::endl;
        number >>= 7;
        // std::cout << "Number after shift: " << number << std::endl;
    }
    uint8_t finalValue = static_cast<uint8_t>(number);
    encoded.push_back(finalValue);
    // std::cout << "Final Value: " << static_cast<int>(finalValue) << std::endl;
}


// Function to add an entry to the posting list
void addToPostingList(std::map<std::string, PostingList>& postingLists, const std::string& term, int docID, int termPosition) {
    // Check if the term exists in the posting list

    std::vector<uint8_t> encodedTermPosition;


    if (postingLists.find(term) == postingLists.end()) {
        // Term not present, create a new entry
        PostingList newPostingList;
        newPostingList.termFrequency = 1;
        std::cout << term << "," << termPosition << std::endl;
        VarEncode(termPosition, encodedTermPosition);
        newPostingList.documentIDs.push_back(std::make_pair(docID, std::vector<uint8_t>{encodedTermPosition}));
        postingLists[term] = newPostingList;
    } else {
        // Term is present, update the posting list
        postingLists[term].termFrequency++;
        
        std::cout << term << "," << termPosition << std::endl;
         // Encode the termPosition using VarByte encoding
        std::vector<uint8_t> encodedTermPosition;
        VarEncode(termPosition, encodedTermPosition);

        postingLists[term].documentIDs.push_back(std::make_pair(docID, std::vector<uint8_t>{encodedTermPosition}));
    }
}

// Function to print the posting lists
void writePostingListsToFile(const std::map<std::string, PostingList>& postingLists) {
    std::ofstream outputFile("postingsList.txt", std::ios::out | std::ios::trunc);
    
    for (const auto& posting : postingLists) {
        outputFile << posting.first << ", ";
        outputFile << posting.second.termFrequency << ": ";
        outputFile << "Document IDs: ";
        for (const auto& docInfo : posting.second.documentIDs) {
            outputFile << "(" << docInfo.first << ", ";
            for (int position : docInfo.second) {
                outputFile << position;
            }
            outputFile << ") ";
        }
        outputFile << std::endl;
    }

    std::cout << "Postings have been written to " << "postingsList.txt" << std::endl;

    outputFile.close();

}

void outputDocIDToUrlMapToFile(const std::map<int, std::string>& docIDToUrlMap) {
    std::ofstream outputFile("docIDToUrlMapping.txt", std::ios::out | std::ios::trunc);
    if (outputFile.is_open()) {
        for (const auto& entry : docIDToUrlMap) {
            std::cout << entry.first << ": " << entry.second << std::endl;
            outputFile << entry.first << ": " << entry.second << std::endl;
        }

        std::cout << "DocID to URL mapping has been written to " << "docIDToUrlMapping.txt" << std::endl;
    } else {
        std::cerr << "Failed to open the output file: " << "docIDToUrlMapping.txt" << std::endl;
    }
    outputFile.close();
}

// Function to parse the .trec file and create intermediate posting lists
void parseTrecFile(std::ifstream& trecFile, std::vector<std::string>& urls, std::vector<std::string>& documents, const int chunkSize, std::ofstream& outputFile, std::ofstream& docIDToUrlMapping, std::map<int, std::string>& docIDToUrlMap, std::map<std::string, PostingList>& postingLists) {
    // Define a map to store posting lists for terms
    // std::map<std::string, PostingList> postingLists;
    // std::map<int, std::string> docIDToUrlMap;

    std::string line;
    std::string documentContent;
    std::string urlContent;
    int termPosition = 0;
    bool isInsideTextTag = false;
    int sizeCount = 0;

    std::cout << "documentID" << documentID << std::endl;

    while (std::getline(trecFile, line)) {
        std::cerr << line << std::endl;
        if (sizeCount > chunkSize) {
            break;
        }

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
                    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                    addToPostingList(postingLists, removeCharactersFromBothEnds(word), documentID, termPosition);
                    termPosition++;
                }
            }
        }
    }

    // if (outputFile.is_open()) {
    // Call writePostingListsToFile to write data to the file
    writePostingListsToFile(postingLists);
    // } else {/
        // std::cerr << "Failed to open the output file." << std::endl;
    // }
    
    outputDocIDToUrlMapToFile(docIDToUrlMap);
}

int main() {
    const std::string trecFilename = "D:\\Notes\\Web Search Engines\\Assignment 2\\fulldocs-new.trec";
    std::ifstream trecFile(trecFilename);

    if (!trecFile.is_open()) {
        std::cerr << "Failed to open the .trec file." << std::endl;
        return 1;
    }

    const int chunkSize = 10;
    std::vector<std::string> urls;
    std::vector<std::string> documents;
    std::map<std::string, PostingList> postingLists;
    std::map<int, std::string> docIDToUrlMap;

    // Open the output file
    std::ofstream outputFile2("test.txt", std::ios::trunc);
    std::ofstream docIDToUrlMapping("test2.txt", std::ios::trunc);
    int count = 0;
    while (!trecFile.eof()) {
        if(count == 2500){
            break;
        }
        count++;
        parseTrecFile(trecFile, urls, documents, chunkSize, outputFile2, docIDToUrlMapping, docIDToUrlMap, postingLists);
        urls.clear();
        documents.clear();
    }

    trecFile.close();
    outputFile2.close();
    docIDToUrlMapping.close();
    
    return 0;
}