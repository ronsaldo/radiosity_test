#ifndef RADIOSITY_TEST_FILE_HPP
#define RADIOSITY_TEST_FILE_HPP

#include <stdio.h>
#include <vector>
#include <string>

namespace RadiosityTest
{

inline std::vector<uint8_t> readWholeFile(const std::string &filename)
{
    std::vector<uint8_t> result;

    // Open the file.
    auto f = fopen(filename.c_str(), "rb");
    if(!f)
    {
        fprintf(stderr, "Failed to open file '%s'\n", filename.c_str());
        return result;
    }

    // Get the file size.
    fseek(f, 0, SEEK_END);
    result.resize(ftell(f));
    fseek(f, 0, SEEK_SET);

    // Read the file content
    auto readCount = fread(&result[0], result.size(), 1, f);
    if(readCount != 1)
    {
        fprintf(stderr, "Failed to read content from file '%s'\n", filename.c_str());
        result.clear();
    }

    // Close the file.
    fclose(f);

    return result;
}

inline std::string readWholeTextFile(const std::string &filename)
{
    auto content = readWholeFile(filename);
    return std::string(content.begin(), content.end());
}

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_FILE_HPP
