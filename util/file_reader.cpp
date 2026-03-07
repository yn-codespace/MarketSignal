
#include "file_reader.h"

std::vector<std::string> read_txt_file(std::string file_path)
{
    std::cout << " inside file_reader.cpp::" << __FUNCTION__ << std::endl;
    std::cout << " reading input from " << file_path << " ... " << std::endl;

    // Open the text file
    std::ifstream infile(file_path); // text mode by default
    bool grab_data = true;
    if (!infile) {
        std::cerr << "Failed to open file in " << __FUNCTION__ << "\n";
        grab_data = false;
    }

    std::vector<std::string> tickers;
    std::string line;

    if (grab_data) {
        while (std::getline(infile, line)) {
            // line contains everything up to '\n', newline is removed
            // std::cout << "Line: " << line << "\n";
            tickers.push_back(line);
        }
    }
    
    std::cout << " finished file_reader.cpp::" << __FUNCTION__ << std::endl;

    return tickers;
}
