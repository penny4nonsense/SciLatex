#ifndef MATLAB_API_MODULE_H
#define MATLAB_API_MODULE_H

#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace matlabutils {
    // Callback for libcurl to store response data
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // Save MATLAB code to a temporary script
    std::string saveMatlabScript(const std::string& matlabCode);

    // Extract MATLAB block (if present) or return full content
    std::string extractMatlabCode(const std::string& responseContent);

    // Execute MATLAB script using -batch
    std::string runMatlabScript(const std::string& scriptFile, const std::string& matlabPath);

    // Generate the API prompt
    std::string generatePrompt(const std::string& operation, const std::string& latexCode);

    // Prepare API request payload
    nlohmann::json prepareApiRequest(const std::string& apiKey, const std::string& prompt);

    // Setup CURL for API request
    void setupCurl(CURL*& curl, const std::string& api_url, const std::string& api_key, const std::string& jsonPayload, struct curl_slist*& headers, std::string& readBuffer);

    // Handle API response
    std::string handleApiResponse(const std::string& readBuffer, const std::string& matlabPath);
}
#endif // MATLAB_API_MODULE_H
