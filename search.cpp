#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <chrono>
#include <algorithm>

struct WordInfo {
    std::string word;
    int docID;
    int freq;
    double bm25;

    bool operator<(const WordInfo& other) const {
        return bm25 < other.bm25;
    }
};

struct WordInfoMulti {
    std::vector<std::string> word;
    int docID;
    std::vector<int> freqArray;
    double bm25;
    
    bool operator<(const WordInfoMulti& other) const {
        return bm25 < other.bm25;
    }
};


// Function to split a string into words based on delimiters
std::vector<std::string> splitString(const std::string& input) {
    const std::string delimiters = ".,?!:;()\"'[]/{}() +#-><$%&*|~";
    std::vector<std::string> tokens;
    std::string word;
    std::istringstream iss(input);

    char c;
    while (iss.get(c)) {
        if (delimiters.find(c) == std::string::npos) {
            word += c;
        } else {
            if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
        }
    }

    if (!word.empty()) {
        tokens.push_back(word);
    }

    return tokens;
}


//Function to get the document based on the offset pointer
std::string getDocument(const std::streampos pointer){
    std::string binFileName = "/Users/shreyasmac/Documents/VS Code/inverted-index/msmarco-docs.trec";
    std::streampos targetLineNumber = pointer;
    std::ifstream binFile(binFileName, std::ios::in);
    
    std::string document = "";

    if (!binFile.is_open())
    {
        std::cerr << "Error: Could not open the .bin file." << std::endl;
        return document;
    }

    binFile.seekg(targetLineNumber);

    if (!binFile)
    {
        std::cerr << "Error: Failed to seek to the desired line." << std::endl;
        return document;
    }

    std::string line;
    while (std::getline(binFile, line))
    {
        if (line.find("</DOC>") != std::string::npos) {
            break;
        }
        document = document + line;
    }
        binFile.close();
        return document;
}


//Function to return the snippet for a conjunctive query
std::string conjunctiveSnippet(std::vector<std::string> searchWords, const std::streampos pointer){

    std::string paragraph = getDocument(pointer);
    std::string snippet = "";
    
    std::transform(paragraph.begin(), paragraph.end(), paragraph.begin(), ::tolower);

    std::vector<size_t> wordPositions;
    for(const std::string& word: searchWords){
        size_t position = paragraph.find(word);
            
        if (position != std::string::npos) {
            wordPositions.push_back(position);
        }
    }

     if (!wordPositions.empty()) {
        // Find the minimum and maximum positions
        size_t startPos = *std::min_element(wordPositions.begin(), wordPositions.end());
        size_t endPos = *std::max_element(wordPositions.begin(), wordPositions.end());
        size_t zero = 0;
        // Extract the snippet
        snippet = snippet + paragraph.substr(std::min(zero,startPos-10), 150);
        snippet = snippet + "....";
        snippet = snippet + paragraph.substr(std::min(zero,endPos-10), 150);
    } else {
        std::cout << "Search words not found in the paragraph." << std::endl;
    }
    return snippet;
}

//Function to return a snippet for a disjunctive query
std::string disjunctiveSnippet(const std::string& keyword, const std::streampos pointer) {
    std::string snippet = "";
    int snippetLength = 150;
    
    std::string document = getDocument(pointer);
    
    std::transform(document.begin(), document.end(), document.begin(), ::tolower);

    size_t keywordPosition = document.find(keyword);

    if (keywordPosition != std::string::npos) {
        size_t snippetStart;

        if (keywordPosition < static_cast<size_t>(snippetLength / 2)) {
            snippetStart = 0;
        } else {
            snippetStart = keywordPosition - static_cast<size_t>(snippetLength / 2);
        }

        size_t snippetEnd = std::min(keywordPosition + keyword.length() + snippetLength / 2, document.length());

        snippet = document.substr(snippetStart, snippetEnd - snippetStart);

        if (snippetStart > 0) {
            snippet = "..." + snippet;
        }

        if (snippetEnd < document.length()) {
            snippet = snippet + "...";
        }
    }
    return snippet;
}


//Function to calculate the bm25 score for disjunctive queries
double bm25Evaluation(double ft, double fdt, double d) {
    int N = 3213836;
    double k = 1.2;
    double b = 0.75;
    double davg = 422;
    
    double firstPart = log((N - ft + 0.5) / (ft + 0.5));
    double secondPart = ((k + 1) * fdt) / (k * ((1 - b) + b * (d / davg)));
    
    return firstPart * secondPart;
}

//Function to calculate the bm25 score for conjunctive queries
double bm25EvaluationMulti(std::vector<int> ft, std::vector<int> fdt, double d) {
    int N = 3213836;
    double k = 1.2;
    double b = 0.75;
    double davg = 422;
    double sum = 0;

    for(int i=0; i<ft.size();i++){
        double firstPart = log((N - ft[i] + 0.5) / (ft[i] + 0.5));
        double secondPart = ((k + 1) * fdt[i]) / (k * ((1 - b) + b * (d / davg)));
        double result = firstPart * secondPart;
        sum = sum + result;
    }
    return sum;
}

// Function to read a file in the given format and store data in a hashtable
// has data which points to the line in which the inverted index for a specific word is present
std::unordered_map<std::string, std::pair<int, std::streampos>> readFileToHashtable(const std::string &filename)
{
    std::unordered_map<std::string, std::pair<int, std::streampos>> hashtable;

    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open the file." << std::endl;
        return hashtable;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        
        std::streampos pointer;

        size_t comma1_pos = line.find(",");
        size_t colon_pos = line.find(":");
        
        if (comma1_pos != std::string::npos && colon_pos != std::string::npos) {
        // Extract substrings
            std::string word_str = line.substr(0, comma1_pos);
            std::string freq_str = line.substr(comma1_pos + 1, colon_pos - comma1_pos - 1);
            std::string pointer_str = line.substr(colon_pos + 1);
            
            // Convert to integers
            std::string word = word_str;
            int freq = std::stoi(freq_str);
            pointer = std::stoll(pointer_str);
            
            // Store the data in the hashtable
            hashtable[word].first = freq;
            hashtable[word].second = pointer;
        } else {
            std::cerr << "Error: Invalid line format: " << line << std::endl;
        }
    }
    
    return hashtable;
}


// Function to read the urls of all the documents parsed
std::unordered_map<int, std::string> readDocumentUrlsFromFile(const std::string &filename)
{
    std::unordered_map<int, std::string> documentUrls;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return documentUrls;
    }
    std::string url;
    
    std::string line;
    while(std::getline(file, line)){
        std::istringstream iss(line);
        
        size_t colon_pos = line.find(":");
        
        if (colon_pos != std::string::npos) {
            std::string docID_str = line.substr(0, colon_pos);
            std::string url = line.substr(colon_pos + 1);
            
            int docID = std::stoi(docID_str);
          
            documentUrls[docID] = url;
        } else {
            std::cerr << "Error: Invalid line format: " << line << std::endl;
        }
    }

    file.close();
    return documentUrls;
}

// Function to read a file in the given format and store data in a hashtable
// has data which points to the line number for each document
std::unordered_map<int, std::pair<int, std::streampos>> readDocumentFileToHashtable(const std::string &filename)
{
    std::unordered_map<int, std::pair<int, std::streampos>> hashtable;

    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open the file." << std::endl;
        return hashtable;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        // std::cout << line;
        std::string word;
        
        std::streampos pointer;

        size_t comma1_pos = line.find(",");
        size_t colon_pos = line.find(":");
    
        if (comma1_pos != std::string::npos && colon_pos != std::string::npos) {
        // Extract substrings
            std::string docID_str = line.substr(0, comma1_pos);
            std::string word_count_str = line.substr(comma1_pos + 1, colon_pos - comma1_pos - 1);
            std::string pointer_str = line.substr(colon_pos + 1);
            
            // Convert to integers
            int docID = std::stoi(docID_str);
            int words = std::stoi(word_count_str);
            pointer = std::stoll(pointer_str);
            
            // Store the data in the hashtable
            hashtable[docID].first = words;
            hashtable[docID].second = pointer;
        } else {
            std::cerr << "Error: Invalid line format: " << line << std::endl;
        }
    }

    return hashtable;
}


// Function to decode VarByte-encoded data
std::vector<int> decodeVarByte(const std::string &encodedData)
{
    std::vector<int> result;
    int value = 0;
    int shift = 0;

    for (char byte : encodedData){
        value |= (byte & 0x7F) << shift;
        shift += 7;

        if ((byte & 0x80) == 0)
        {
            result.push_back(value);
            value = 0;
            shift = 0;
        }
    }
    return result;
}


// Function to perform the conjunctive search operation
int conjunctiveSearch(std::unordered_map<std::string, std::pair<int, std::streampos>> pointerWord, std::unordered_map<int, std::string> documentUrls, std::vector<std::string> words, std::unordered_map<int, std::pair<int, std::streampos>> documentPointer){
    
    std::string binFileName = "/Users/shreyasmac/Documents/VS Code/inverted-index/compressed/postings_inverted.bin";
    std::ifstream binFile(binFileName, std::ios::in | std::ios::binary);
    if (!binFile.is_open())
    {
        std::cerr << "Error: Could not open the .bin file." << std::endl;
        return 1;
    }
    
    // Create a max heap using a priority_queue
    std::priority_queue<WordInfoMulti> maxHeapClubbed;
    std::priority_queue<WordInfo> maxHeapSingle;
    std::vector<WordInfo> wordInfoList;
    
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> invertedIndex;
    
    for (const std::string& word : words) {
        std::streampos targetLineNumber = pointerWord[word].second;
        
        binFile.seekg(targetLineNumber);
        
        if (!binFile)
        {
            std::cerr << "Error: Failed to seek to the desired line." << std::endl;
            return 1;
        }
        
        std::string line;
        
        while (std::getline(binFile, line))
        {
            std::vector<std::pair<int, int>> pairs;
            std::vector<int> decodedValues = decodeVarByte(line);
            
            for (size_t i = 0; i < decodedValues.size(); i += 4) {
                if (i + 2 < decodedValues.size()) {
                    pairs.push_back(std::make_pair(decodedValues[i], decodedValues[i + 2]));
                    
                }
            }
            
            invertedIndex[word] = pairs;
            //          break after 1 iteration
            break;
        }
    }
    for (const auto& entry : invertedIndex) {
        const std::string& word = entry.first;
        const std::vector<std::pair<int, int>>& pairs = entry.second;

        for (const std::pair<int, int>& pair : pairs) {
            int docID = pair.first;
            int freq = pair.second;

            WordInfo newWordInfo = {word, docID, freq, 0};

            wordInfoList.push_back(newWordInfo);
        }
    }
    
    std::unordered_map<int, std::pair<std::vector<std::string>, std::vector<int>>> clubbedWordInfo;

    for(const auto& entry: wordInfoList){
        if (clubbedWordInfo.find(entry.docID) == clubbedWordInfo.end()) {
            clubbedWordInfo[entry.docID] = {{}, {}};
        }
        clubbedWordInfo[entry.docID].first.push_back(entry.word);
        clubbedWordInfo[entry.docID].second.push_back(entry.freq);
    }

    for(const auto& entry: clubbedWordInfo){
        if(entry.second.second.size() == 1){
                                    //word                  docID           freq
            WordInfo newWordInfo = {entry.second.first[0], entry.first, entry.second.second[0],
                                    bm25Evaluation(pointerWord[entry.second.first[0]].first, entry.second.second[0],
                                    documentPointer[entry.first].first)};
            maxHeapSingle.push(newWordInfo);
        }
        else if(entry.second.second.size() > 1){
            std::vector<int> pointerWords;
            for(int i=0; i<entry.second.first.size(); i++){
                pointerWords.push_back(pointerWord[entry.second.first[i]].first);
            }
            WordInfoMulti newWordInfo = {entry.second.first, entry.first, entry.second.second,
                                                bm25EvaluationMulti(pointerWords, entry.second.second, documentPointer[entry.first].first)};
            maxHeapClubbed.push(newWordInfo);
        }
    }
    
    int i;
    size_t maxHeapClubbedSize = maxHeapClubbed.size();
    for(i=0; i<maxHeapClubbedSize; i++){
        if(i==10){
            break;
        }
        WordInfoMulti poppedWord = maxHeapClubbed.top();
        std::vector<std::string> wordList;

        std::cout << std::endl;
        std::cout << i+1 << ") Url: " << documentUrls[poppedWord.docID] << std::endl;
        std::cout<< "word (freq): ";
        for(int i=0; i<poppedWord.word.size(); i++){
            std::cout<< poppedWord.word[i] << " (" << poppedWord.freqArray[i] << "),";
            wordList.push_back(poppedWord.word[i]);
        }
        std::cout << " BM25 score: " << poppedWord.bm25 << std::endl;

        std::cout << "Snippet: " << conjunctiveSnippet(wordList, documentPointer[poppedWord.docID].second) << std::endl;
        maxHeapClubbed.pop();
    
        if(maxHeapClubbed.empty()){
            break;
        }
    }
    
    if(i<10){
        size_t maxHeapSingleSize = maxHeapSingle.size();
        int counter = i+1;
        for(i=0; i<maxHeapSingleSize; i++){
            counter++;
            if(counter==11){
                break;
            }
            
            WordInfo poppedWord = maxHeapSingle.top();
            
            std::cout << std::endl;
            std::cout << counter << ") Url: " << documentUrls[poppedWord.docID] << std::endl;
            std::cout<< "word: " << poppedWord.word << ", freq of word: " << poppedWord.freq << ", BM25 score: " << poppedWord.bm25 << std::endl;
            std::cout << "Snippet: " << disjunctiveSnippet(poppedWord.word, documentPointer[poppedWord.docID].second) << std::endl;
            maxHeapSingle.pop();
        
            if(maxHeapSingle.empty()){
                std::cout << "No more URLs present" << std::endl;
                return 1;
            }
        }
    }

    binFile.close();
    
    return 1;
}

// Function to perform the disjunctive search operation
int disjunctiveSearch(std::unordered_map<std::string, std::pair<int, std::streampos>> pointerWord, std::unordered_map<int, std::string> documentUrls, std::vector<std::string> words, std::unordered_map<int, std::pair<int, std::streampos>> documentPointer){
    
    std::string binFileName = "/Users/shreyasmac/Documents/VS Code/inverted-index/compressed/postings_inverted.bin";
    std::ifstream binFile(binFileName, std::ios::in | std::ios::binary);
    if (!binFile.is_open())
    {
        std::cerr << "Error: Could not open the .bin file." << std::endl;
        return 1;
    }
    
    std::priority_queue<WordInfo> maxHeap;
    std::vector<WordInfo> wordInfoList;
    
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> invertedIndex;
    
    for (const std::string& word : words) {
        std::streampos targetLineNumber = pointerWord[word].second;
        
        binFile.seekg(targetLineNumber);

        if (!binFile)
        {
            std::cerr << "Error: Failed to seek to the desired line." << std::endl;
            return 1;
        }

        std::string line;

        while (std::getline(binFile, line))
        {
            std::vector<std::pair<int, int>> pairs;
            std::vector<int> decodedValues = decodeVarByte(line);
            
            for (size_t i = 0; i < decodedValues.size(); i += 4) {
               if (i + 2 < decodedValues.size()) {
                   pairs.push_back(std::make_pair(decodedValues[i], decodedValues[i + 2]));
                   
               }
           }
            
            invertedIndex[word] = pairs;
            
//          break after 1 iteration
            break;
        }
    }
    
    for (const auto& entry : invertedIndex) {
        const std::string& word = entry.first;
        const std::vector<std::pair<int, int>>& pairs = entry.second;

        for (const std::pair<int, int>& pair : pairs) {
            int docID = pair.first;
            int freq = pair.second;

            
//                                                                        f          fdt                    documentSize
            WordInfo newWordInfo = {word, docID, freq, bm25Evaluation(pointerWord[word].first, freq, documentPointer[docID].first)};
            maxHeap.push(newWordInfo);
        }
    }
    
    for(int i=0; i<10; i++){
        WordInfo poppedWord = maxHeap.top();
        
        std::cout << std::endl;
        std::cout << i+1 << ") Url: " << documentUrls[poppedWord.docID] << std::endl;
        std::cout<< "word: " << poppedWord.word << ", freq of word: " << poppedWord.freq << ", BM25 score: " << poppedWord.bm25 << std::endl;
        std::cout << "Snippet: " << disjunctiveSnippet(poppedWord.word, documentPointer[poppedWord.docID].second) << std::endl;
        maxHeap.pop();
    
        if(maxHeap.empty()){
            std::cout << "No more URLs present" << std::endl;
            return 1;
        }
    }
    
    binFile.close();
    return 1;
}


int main(){
    std::string filename = "/Users/shreyasmac/Documents/VS Code/inverted-index/wordPositions/postings_inverted_word_positions.txt";
    std::unordered_map<std::string, std::pair<int, std::streampos>> pointerToWord = readFileToHashtable(filename);
    std::cout << "Reading word hashtable done" << std::endl;

    std::string urlsFileName = "/Users/shreyasmac/Documents/VS Code/inverted-index/docIDToUrlMapping.txt";
    std::unordered_map<int, std::string> documentUrls = readDocumentUrlsFromFile(urlsFileName);
    std::cout << "Reading urls done" << std::endl;
    
    std::string documentPointerFile = "/Users/shreyasmac/Documents/VS Code/inverted-index/documentPositions/documentPositions.txt";
    std::unordered_map<int, std::pair<int, std::streampos>> documentPointer = readDocumentFileToHashtable(documentPointerFile);
    std::cout << "Reading document hashtable done" << std::endl;
    
    std::vector<std::string> words;
    
    while(true){
        int choice;
        std::cout << "1. Add a query" << std::endl;
        std::cout << "2. Perform a disjunctive query" << std::endl;
        std::cout << "3. Perform a conjunctive query" << std::endl;
        std::cout << "4. Exit" << std::endl;
        std::cin >> choice;
        
        std::cin.ignore();

//        The hot glowing surfaces of stars emit energy in the form of electromagnetic radiation.? It is a good approximation to assume that the emissivity e is equal to 1 for these surfaces.Find the radius of the star Rigel, the bright blue star in the constellation Orion Follow numbers together stores inverted frequency
        
        switch (choice) {
            case 1:{
                std::cout << "Enter the query." << std::endl;
                std::string sentence;
                std::vector<std::string> splitSentence;
                std::getline(std::cin, sentence);
                if (sentence.empty()) {
                    std::cout << "Empty line, please enter a word." << std::endl;
                } else {
                    std::vector<std::string> splitSentence = splitString(sentence);
                    for (std::string& word : splitSentence) {
                        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                        words.push_back(word);
                    }
                    
                }
                break;
            }
            case 2:{
                auto startTime = std::chrono::high_resolution_clock::now();
                
                if(words.size()==0){
                    std::cout << "Please enter word(s) to search" << std::endl;
                    break;
                }
                std::cout << "Disjunctive Query." << std::endl;
                std::unordered_map<std::string, std::pair<int, std::streampos>> pointerMap;
                std::cout << "Searched words (" << words.size() << ") : ";
                for (const std::string& word : words) {
                    pointerMap[word].first = pointerToWord[word].first;
                    pointerMap[word].second = pointerToWord[word].second;
                    std::cout << word << " ";
                }
                disjunctiveSearch(pointerMap, documentUrls, words, documentPointer);
                auto endTime = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;
                
                words.clear();
                break;
            }
            case 3:{
                auto startTime = std::chrono::high_resolution_clock::now();
                if(words.size()==0){
                    std::cout << "Please enter word(s) to search" << std::endl;
                    break;
                }
                std::cout << "Conjunctive Query." << std::endl;
                std::unordered_map<std::string, std::pair<int, std::streampos>> pointerMap;
                std::cout << "Searched words (" << words.size() << ") : ";
                for (const std::string& word : words) {
                    pointerMap[word].first = pointerToWord[word].first;
                    pointerMap[word].second = pointerToWord[word].second;
                    std::cout << word << " ";
                }
                conjunctiveSearch(pointerMap, documentUrls, words, documentPointer);
                auto endTime = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;
                
                words.clear();
                break;
            }
            case 4:
                std::cout << "Exit." << std::endl;
                return 0;
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 4." << std::endl;
                break;
        }
    }
    
    return 0;
}
