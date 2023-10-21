#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_map>

std::string cleanWord(const std::string& word) {
    std::string cleanedWord;
    for (char c : word) {
        if (isalnum(c)) {
            cleanedWord += c;
        }
    }
    return cleanedWord;
}

// Function to update the entries
void updateEntries(std::map<std::string, std::vector<std::pair<int, int>>>& data, const std::string& line) {
    size_t pos = line.find(":");
    if (pos != std::string::npos) {
        std::string word = line.substr(0, pos);
        word = cleanWord(word);
        std::cerr<<word<<std::endl;

        for (const auto& entry : data) {
            std::cout << entry.first << ": ";
            for (const auto& pair : entry.second) {
                std::cout << "(" << pair.first << ", " << pair.second << ") ";
            }
            std::cout << std::endl;
        }

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
        {'_', "output_misc.txt"},
        {'a', "output_a_c.txt"},
        {'d', "output_d_f.txt"},
        {'g', "output_g_i.txt"},
        {'j', "output_j_l.txt"},
        {'m', "output_m_o.txt"},
        {'p', "output_p_r.txt"},
        {'s', "output_s_u.txt"},
        {'v', "output_v_z.txt"}
    };

    for (const auto& categoryFileNamePair : categoryToFileName) {
        char category = categoryFileNamePair.first;
        std::string fileName = categoryFileNamePair.second;
        std::string filePath = "sortedPostings/" + fileName;

        std::ifstream file(filePath);

        if (!file) {
            std::cerr << "Error: Unable to open file " << filePath << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::cerr<<line<<std::endl;
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
        data.clear();  // Clear the data for the next file
    }

    return 0;
}
