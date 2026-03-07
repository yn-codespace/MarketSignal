
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>  // This is the JSON library to parse the response (make sure to install it)


// Callback function to handle the response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {

    std::cout << " inside main()! " << std::endl;
    
    
    // Initialize CURL
    CURL* curl;
    CURLcode res;

    // The URL of the Google Gemini API endpoint
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3-flash-preview:generateContent";

    // Your Google Gemini API Key
    std::string api_key = "AIzaSyCp37kIUL8qcWJpZ9Mu7kXUxcSS6R5vhwA";

    // Create json object
    nlohmann::json json;

    // The JSON body containing the request data
    std::string data = R"(
    {
        "contents": [
            {
                "parts": [
                    {
                        "text": "What is the future of AI?"
                    }
                ]
            }
        ]
    })";

    // Initialize CURL session
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string response_data;  // To store the response

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the HTTP headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("x-goog-api-key: " + api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the POST data (the JSON request body)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        // Set the callback function to write the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Print the response data
            std::cout << "Response from Gemini API:" << std::endl;
            std::cout << response_data << std::endl;


            // Parse the response 
            nlohmann::json parsed_data = nlohmann::json::parse(response_data);

            if (parsed_data["error"].is_null()) {
                std::cout << "No error found! Parsing output ... " << std::endl;
            } else {
                std::cout << "Error: " << parsed_data["error"] << std::endl;
            }
            
            
        }


        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    // Clean up global CURL environment
    curl_global_cleanup();


    std::cout << " finished main()! " << std::endl;

    
    return 0;
}