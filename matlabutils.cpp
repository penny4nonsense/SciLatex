#include "matlabutils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace matlabutils {
    // Callback for libcurl to store response data
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // Save MATLAB code to a temporary script
    std::string saveMatlabScript(const std::string& matlabCode) {
        std::string currentDir = std::filesystem::current_path().string();
        std::string filename = currentDir + "/temp_script.m";

        std::ofstream scriptFile(filename);
        if (!scriptFile) {
            throw std::runtime_error("Failed to create MATLAB script file.");
        }
        scriptFile << matlabCode << std::endl;
        scriptFile.close();

        return filename;
    }

    // Extract MATLAB block (if present) or return full content
    std::string extractMatlabCode(const std::string& responseContent) {
        std::istringstream stream(responseContent);
        std::string line;
        std::string matlabCode;
        bool inMatlabBlock = false;

        while (std::getline(stream, line)) {
            if (line == "```matlab") {
                inMatlabBlock = true;
                matlabCode.clear();
                continue;
            } else if (inMatlabBlock && line == "```") {
                break;
            }

            if (inMatlabBlock) {
                matlabCode += line + "\n";
            }
        }

        return inMatlabBlock ? matlabCode : responseContent;
    }

    // Execute MATLAB script using -batch

    std::string runMatlabScript(const std::string& scriptFile, const std::string& matlabPath) {
        std::string scriptDir = std::filesystem::path(scriptFile).parent_path().string();
        std::string scriptName = std::filesystem::path(scriptFile).filename().string();

        // Define a temporary output file for MATLAB to write its result
        std::string tempOutputFile = scriptDir + "/matlab_output.txt";
        std::string command = matlabPath + " -batch \"cd('" + scriptDir + "'); " + scriptName.substr(0, scriptName.size() - 2) + "\" > " + tempOutputFile;

        // Execute MATLAB script
        int retCode = std::system(command.c_str());
        if (retCode != 0) {
            throw std::runtime_error("Failed to execute MATLAB script. Return code: " + std::to_string(retCode));
        }

        // Read the result from the output file
        std::ifstream outputFile(tempOutputFile);
        if (!outputFile) {
            throw std::runtime_error("Failed to open MATLAB output file.");
        }

        std::string result;
        std::string line;
        int lineCount = 0;
        while (std::getline(outputFile, line)) {
            if (lineCount >= 4) { // Skip the first 4 lines of the output
                result += line + "\n";
            }
            lineCount++;
        }

        outputFile.close();
        std::filesystem::remove(tempOutputFile);

        result.erase(result.find_last_not_of(" \t\n\r\f\v") + 1);
        result.erase(0, result.find_first_not_of(" \t\n\r\f\v"));

        return result;
    }

    // Generate the API prompt
    std::string generatePrompt(const std::string& operation, const std::string& latexCode) {
        const std::string basePrompt =
            "You are a component of a computer algebra system specializing in LaTeX-to-Matlab symbolic math translation. Your task is as follows:\n\n"
            "1. Take the provided LaTeX code and interpret it as symbolic math.\n"
            "2. Translate this symbolic math into Matlab's symbolic math toolkit, focusing on the specified operation: %PLACEHOLDER_OPERATION%.\n"
            "3. Generate Matlab code that performs the operation).\n"
            "4. Append a line of Matlab code that converts the result back into LaTeX format.\n"
            "5. In the end of the code, be sure to display the resulting LaTeX.\n\n"
            "Here is the LaTeX code for translation:\n\n"
            "%PLACEHOLDER_LATEX%\n\n"
            "Please ensure the translation preserves the mathematical meaning of the LaTeX code and uses Matlab's symbolic math capabilities effectively.\n"
            "Please also be sure to declare variables used in sums like i and j (if used in the code)";

        std::string prompt = basePrompt;
        size_t operationPos = prompt.find("%PLACEHOLDER_OPERATION%");
        if (operationPos != std::string::npos) {
            prompt.replace(operationPos, std::string("%PLACEHOLDER_OPERATION%").length(), operation);
        }

        size_t latexPos = prompt.find("%PLACEHOLDER_LATEX%");
        if (latexPos != std::string::npos) {
            prompt.replace(latexPos, std::string("%PLACEHOLDER_LATEX%").length(), latexCode);
        }

        return prompt;
    }

    // Prepare API request payload
    nlohmann::json prepareApiRequest(const std::string& apiKey, const std::string& prompt) {
        return {
            {"model", "gpt-4o"},
            {"messages", {{{"role", "user"}, {"content", prompt}}}},
            {"api_key", apiKey}
        };
    }

    // Setup CURL for API request
    void setupCurl(CURL*& curl, const std::string& api_url, const std::string& api_key, const std::string& jsonPayload, struct curl_slist*& headers, std::string& readBuffer) {
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    }

    // Handle API response
    std::string handleApiResponse(const std::string& readBuffer, const std::string& matlabPath) {
        nlohmann::json responseJson = nlohmann::json::parse(readBuffer);

        if (responseJson.contains("choices") && !responseJson["choices"].empty()) {
            std::string rawContent = responseJson["choices"][0]["message"]["content"];
            std::string matlabCode = extractMatlabCode(rawContent);

            std::cout << "Extracted MATLAB Code:\n" << matlabCode << std::endl;

            std::string scriptFile = saveMatlabScript(matlabCode);

            try {
                // Pass the MATLAB path to the runMatlabScript function
                std::string result = runMatlabScript(scriptFile, matlabPath);
                return result; // Return the result of the MATLAB script execution
            } catch (const std::exception &e) {
                // Handle errors during script execution
                std::cerr << "Error executing MATLAB script: " << e.what() << std::endl;
                throw; // Re-throw to allow the caller to handle this error
            }
        } else {
            throw std::runtime_error("No valid response content found.");
        }
    }

    /*
    // Main function
    int main() {
        const std::string api_url = "https://api.openai.com/v1/chat/completions";
        const std::string api_key = "YOUR_API_KEY_HERE";

        const std::string operation = "evaluate";
        const std::string latexCode = "\\sum_{i=1}^{n}i";

        try {
            std::string prompt = generatePrompt(operation, latexCode);
            nlohmann::json requestData = prepareApiRequest(api_key, prompt);
            std::string jsonPayload = requestData.dump();

            CURL* curl = nullptr;
            struct curl_slist* headers = nullptr;
            std::string readBuffer;

            setupCurl(curl, api_url, api_key, jsonPayload, headers, readBuffer);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
            }

            handleApiResponse(readBuffer);

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }

        return 0;
    }
    */
}
