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
        encoded.push_back(static_cast<uint8_t>((number & 127) | 128));
        number >>= 7;
    }
    encoded.push_back(static_cast<uint8_t>(number));
}

// Function to add an entry to the posting list
void addToPostingList(std::map<std::string, PostingList>& postingLists, const std::string& term, int docID, int termPosition) {
    // Check if the term exists in the posting list

    std::vector<uint8_t> encodedTermPosition;


    if (postingLists.find(term) == postingLists.end()) {
        // Term not present, create a new entry
        PostingList newPostingList;
        newPostingList.termFrequency = 1;
        VarEncode(termPosition, encodedTermPosition);
        newPostingList.documentIDs.push_back(std::make_pair(docID, std::vector<uint8_t>{encodedTermPosition}));
        postingLists[term] = newPostingList;
    } else {
        // Term is present, update the posting list
        postingLists[term].termFrequency++;
        
         // Encode the termPosition using VarByte encoding
        std::vector<uint8_t> encodedTermPosition;
        VarEncode(termPosition, encodedTermPosition);

        postingLists[term].documentIDs.push_back(std::make_pair(docID, std::vector<uint8_t>{encodedTermPosition}));
    }
}

// Function to print the posting lists
void writePostingListsToFile(const std::map<std::string, PostingList>& postingLists, std::ofstream& outputFile) {
    for (const auto& posting : postingLists) {
        outputFile << posting.first << ", ";
        outputFile << posting.second.termFrequency << ": ";
        outputFile << "Doument IDs: ";
        for (const auto& docInfo : posting.second.documentIDs) {
            outputFile << "(" << docInfo.first << ", ";
            for (int position : docInfo.second) {
                outputFile << position;
            }
            outputFile << ") ";
        }
        outputFile << std::endl;
    }
}

// Function to parse the .trec file and create intermediate posting lists
void parseTrecFile(std::ifstream& trecFile, std::vector<std::string>& urls, std::vector<std::string>& documents, const int chunkSize, std::ofstream& outputFile) {
    // Define a map to store posting lists for terms
    std::map<std::string, PostingList> postingLists;
    std::map<int, std::string> docIDToUrlMap;

    std::string line;
    std::string documentContent;
    std::string urlContent;
    int documentID = -1;
    int termPosition = 0;
    bool isInsideTextTag = false;
    int sizeCount = 0;

    while (std::getline(trecFile, line)) {
        if (sizeCount > chunkSize) {
            break;
        }

        if (line.find("<TEXT>") != std::string::npos) {
            isInsideTextTag = true;
            urlContent.clear();
            documentContent.clear();
            documentID++;
            sizeCount++;
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

        writePostingListsToFile(postingLists, outputFile);
    }
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

    // Open the output file
    std::ofstream outputFile("output.txt");

    while (!trecFile.eof()) {
        parseTrecFile(trecFile, urls, documents, chunkSize, outputFile);
        urls.clear();
        documents.clear();
    }

    trecFile.close();
    outputFile.close();
    
    return 0;
}