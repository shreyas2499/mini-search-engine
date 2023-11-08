#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_map>

std::string cleanWord(const std::string& word) {
    const char charactersToRemove[] = { '.', ',', '?', '!', ':', ';', '-', ' ', ')', '(', '"', '\'', '[', ']', '/' };

    std::string result = word;

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

// Function to update the entries
void updateEntries(std::map<std::string, std::vector<std::pair<int, int>>>& data, const std::string& line) {
    size_t pos = line.find(":");
    if (pos != std::string::npos) {
        std::string word = line.substr(0, pos);
        word = cleanWord(word);

        if (!word.empty()) { // Check if the word is not empty after cleaning
            std::string rest = line.substr(pos + 1);

            if (data.find(word) != data.end()) {
                std::vector<std::pair<int, int>>& values = data[word];
                std::stringstream ss(rest);
                char c;
                int a, b;
                while (ss >> c >> a >> c >> b >> c) {
                    values.push_back(std::make_pair(a, b));
                }
            } else {
                // If the word is not found, add it to the map with the current values
                std::vector<std::pair<int, int>> values;
                std::stringstream ss(rest);
                char c;
                int a, b;
                while (ss >> c >> a >> c >> b >> c) {
                    values.push_back(std::make_pair(a, b));
                }
                data[word] = values;
            }
        }
    }
}

int main() {
    // Create a map to store the data
    std::map<std::string, std::vector<std::pair<int, int>> > data;

    // Create a map to store the file names for each category
    std::unordered_map<char, std::string> categoryToFileName = {
        {'_', "postings_misc.txt"},
        {'a', "postings_a.txt"},
        {'b', "postings_b.txt"},
        {'c', "postings_c.txt"},
        {'d', "postings_d.txt"},
        {'e', "postings_e.txt"},
        {'f', "postings_f.txt"},
        {'g', "postings_g.txt"},
        {'h', "postings_h.txt"},
        {'i', "postings_i.txt"},
        {'j', "postings_j.txt"},
        {'k', "postings_k.txt"},
        {'l', "postings_l.txt"},
        {'m', "postings_m.txt"},
        {'n', "postings_n.txt"},
        {'o', "postings_o.txt"},
        {'p', "postings_p.txt"},
        {'q', "postings_q.txt"},
        {'r', "postings_r.txt"},
        {'s', "postings_s.txt"},
        {'t', "postings_t.txt"},
        {'u', "postings_u.txt"},
        {'v', "postings_v.txt"},
        {'w', "postings_w_z.txt"},
    };

    for (const auto& categoryFileNamePair : categoryToFileName) {
        char category = categoryFileNamePair.first;
        std::string fileName = categoryFileNamePair.second;
        std::string filePath = "sortedPostingsAlphabetically/" + fileName;

        std::ifstream file(filePath);

        if (!file) {
            std::cerr << "Error: Unable to open file " << filePath << std::endl;
            continue;
        }

        std::string line;
        std::cerr << "Reading file: " << fileName << std::endl;
        while (std::getline(file, line)) {
            updateEntries(data, line);
        }

        file.close();

        // Write the updated data back to the file
        std::ofstream outFile(filePath);
        if (!outFile) {
            std::cerr << "Error: Unable to open file for writing: " << filePath << std::endl;
            continue;
        }

        for (const auto& entry : data) {
            outFile << entry.first << ": ";
            for (const auto& pair : entry.second) {
                outFile << "(" << pair.first << ", " << pair.second << ") ";
            }
            outFile << std::endl;
        }

        outFile.close();
        std::cerr << "Updated file: " << fileName << std::endl;
        data.clear();  // Clear the data for the next file
    }

    return 0;
}
