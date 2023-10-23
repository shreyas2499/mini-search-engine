#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <regex>
#include <sstream>
#include <string>
#include <filesystem>

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

int main() {
    // List of input file names
    std::vector<std::string> inputFiles = {"postings_misc.txt","postings_a.txt", "postings_b.txt", "postings_c.txt", "postings_d.txt", 
    "postings_e.txt", "postings_f.txt", "postings_g.txt", "postings_h.txt", "postings_i.txt", "postings_j.txt", 
    "postings_k.txt", "postings_l.txt", "postings_m.txt", "postings_n.txt", "postings_o.txt", "postings_p.txt", 
    "postings_q.txt", "postings_r.txt", "postings_s.txt", "postings_t.txt", "postings_u.txt", "postings_v.txt", 
    "postings_w_z.txt"};
    
    for (const std::string& inputFileName : inputFiles) {
        std::ifstream inputFile("sortedPostingsAlphabetically/" + inputFileName);
        std::string outputFileName = inputFileName.substr(0, inputFileName.find_last_of('.')) + ".bin";

        std::string folderName = "compressed";
       
        if (!fs::is_directory(folderName)) {
            fs::create_directory(folderName); // Create the "data" folder if it doesn't exist
        }   

        std::ofstream outputFile("compressed/" + outputFileName, std::ios::out | std::ios::binary); // Open in binary mode

        if (!inputFile.is_open() || !outputFile.is_open()) {
            std::cerr << "Error: Could not open files." << std::endl;
            return 1;
        }

        std::string line;
        std::regex entryPattern("(#?\\w+):?\\s*\\((\\d+), (\\d+)\\)");

        while (std::getline(inputFile, line)) {
            size_t pos = line.find(":");

            if (pos != std::string::npos) {
                std::string word = line.substr(0, pos);

                if (!word.empty()) {
                    std::string rest = line.substr(pos + 1);
                    std::stringstream ss(rest);
                    char c;
                    int a, b;
                    outputFile.write(word.c_str(), word.size());
                    outputFile.write(" ", 1);
                    int count = 0;

                    while (ss >> c >> a >> c >> b >> c) {
                        int docID = a;
                        int frequency = b;

                        // Encode the docID and frequency using VarByte
                        std::string binaryDocID = varByteEncode(docID);
                        std::string binaryFrequency = varByteEncode(frequency);

                        // Write the encoded data to the binary file
                        if (count != 0) {
                            outputFile.write(",", 1);
                        }
                        outputFile.write(binaryDocID.c_str(), binaryDocID.size());
                        outputFile.write(" ", 1);
                        outputFile.write(binaryFrequency.c_str(), binaryFrequency.size());
                        count++;
                    }
                    outputFile.write("\n", 1);
                }
            }
        }

        inputFile.close();
        outputFile.close();     
        
        std::cout << "VarByte encoding completed. Results saved to '" << outputFileName << "'." << std::endl;
    }

    return 0;
}
