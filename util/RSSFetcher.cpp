
#include "RSSFetcher.h"

RSSFetcher::RSSFetcher()
{   
    std::cout << " inside RSSFetcher.cpp::" << __FUNCTION__ << std::endl;

    // Store -1 in an atomic index so each thread can increment it
    // and not worry about going out of bounds
    atomicIndex.store(-1);

    
    std::cout << " finished RSSFetcher.cpp::" << __FUNCTION__ << std::endl;
}

RSSFetcher::~RSSFetcher()
{

}

void RSSFetcher::addItem(RSSInputData* input_data)
{
    // Grab the mutex and add the input data to the queue
    std::lock_guard<std::mutex> lock(mutex_);  // locks the mutex
    queue_.push(input_data);                        // safe push
    // mutex is automatically released when lock goes out of scope
}



void RSSFetcher::taskManager() 
{
    // Initialize CURL setup inside each taskManager

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();


    
    while (true) {
        RSSInputData* task;
        // Scope for unique_lock
        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait until queue is not empty or stop signal
            // cond_.wait(lock, [this]() { return !queue_.empty(); });
            if (queue_.empty()) {
                break; // safe: still holding the lock
            }
  
            // If the task queue is not empty, grab the first task
            if (!queue_.empty()) {
                task = queue_.front();
                queue_.pop(); 
            }
        } // unlock happens here
        
        if (task != nullptr) {

            // Iterate through the URLs and gather headlines
            for (int i = 0; i < task->urls.size(); i++) {

                std::cout << "RSSFetcher::" << __FUNCTION__ << " processing: " << task->urls[i] << std::endl;
     
                processTask(task, curl, i);   // actually fetch RSS

                parseRSS(task); // parse the extracted rss

                // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            // After gathering all the headlines, call the LLM to summarize
            // callLLMAPI(task, curl);

            
        }

        
    } // end while


    // CURL cleanup
    curl_easy_cleanup(curl);

    curl_global_cleanup();
    
}


// Declare write_callback before use in RSSFetcher::processTask
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* str = static_cast<std::string*>(userp); // Cast the void* to the data into a string*
    str->append(static_cast<char*>(contents), size * nmemb); // Cast the contents into a char* pointer
    return size * nmemb;
}



void RSSFetcher::processTask(RSSInputData* task, CURL* curl, int i)
{
    
    if (curl)  {
        

        std::string buffer;
            
        curl_easy_setopt(curl, CURLOPT_URL, task->urls[i].c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);


        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;
            task->success = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
        else {
            // std::cout << buffer << std::endl;  // RSS XML
            // std::cout << "RSSFetcher::" << __FUNCTION__ << " processed " << task->url << " succesfully " << std::endl;
            // std::cout << "\n\n\n\n\n\n\n\n";
            // std::cout << buffer << std::endl;
            // std::cout << "\n\n\n\n\n\n\n\n";
            task->rssData = buffer;
        }

        

        
    }
}

void RSSFetcher::parseRSS(RSSInputData* task)
{
    
    if (!task) {
        std::cerr << "parseRSS: task is null\n";
        return;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError parseResult = doc.Parse(task->rssData.c_str());

    if (parseResult != tinyxml2::XML_SUCCESS) {
        std::cerr << "ticker " << task->ticker << " parseRSS: Failed to parse RSS XML, error code: " << parseResult << "\n";
        task->success = false;
        return;
    }

    // Safely get <rss> element
    tinyxml2::XMLElement* rss = doc.FirstChildElement("rss");
    if (!rss) {
        std::cerr << "ticker " << task->ticker << " parseRSS: No <rss> element found\n";
        task->success = false;
        return;
    }

    // Safely get <channel> element
    tinyxml2::XMLElement* channel = rss->FirstChildElement("channel");
    if (!channel) {
        std::cerr << "ticker " << task->ticker  << " parseRSS: No <channel> element found\n";
        task->success = false;
        return;
    }

    // Iterate all <item> elements
    for (tinyxml2::XMLElement* item = channel->FirstChildElement("item"); 
         item != nullptr; 
         item = item->NextSiblingElement("item")) 
    {
        const char* titleC = item->FirstChildElement("title") 
                             ? item->FirstChildElement("title")->GetText() 
                             : nullptr;
        const char* linkC  = item->FirstChildElement("link") 
                             ? item->FirstChildElement("link")->GetText() 
                             : nullptr;
        const char* pubDateC = item->FirstChildElement("pubDate") 
                                ? item->FirstChildElement("pubDate")->GetText() 
                                : nullptr;
    
        Headline headline;
        headline.title = titleC && *titleC ? titleC : " ";
        headline.link = linkC  && *linkC  ? linkC  : " ";
        headline.pubDate = pubDateC && *pubDateC ? pubDateC : " ";
        headline.description = "";
        headline.source = "";

        task->headlines.push_back(headline);
    

        // std::cout << "Title: " << title << "\n";
        // std::cout << "Link: " << link << "\n";
        // std::cout << "Date: " << pubDate << "\n\n";
    }
    
}

void RSSFetcher::callLLMAPI(RSSInputData* task, CURL* curl)
{

    CURLcode res;

    
    // The URL of the Google Gemini API endpoint
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3-flash-preview:generateContent";

    std::string api_key = "abc123";

    // Iterate through headlines and push them to the LLM
    nlohmann::json payload;

    // Create a temporary content JSON
    std::string prompt_definition = "I'm going to give you many headlines for the ticker symbol " + task->ticker + ". I want you to summarize the information, discarding irrelevant headlines that do not pertain to the company associated with the ticker. Also, provide me a score for buying and score for selling inside of a JSON with the follow example format: { \"ticker\": \"AAPL\", \"buy_score\": 85, \"sell_score\": 45, \"score_description\": { \"buy_score\": \"Based on the recent news, earnings report, and market sentiment, the stock shows strong potential for growth in the short term.\", \"sell_score\": \"Despite recent rallies, there are concerns around valuation and market volatility, suggesting a potential risk for further gains.\" }, \"analysis_data\": { \"price_trend\": \"Upward\", \"analyst_ratings\": \"Moderate Buy\", \"earnings_performance\": \"Beating estimates\", \"news_sentiment\": \"Positive\", \"macro_trends\": \"Strong demand in technology sector\" } }";
        
    nlohmann::json content = {
        {"parts", nlohmann::json::array({ {{"text",  prompt_definition}} })}
    };

    // Now, push it back onto the contents array
    payload["contents"].push_back(content); 

    
    for (const auto& headline : task->headlines) {

        // Create a temporary content json
        nlohmann::json content = {
            {"parts", nlohmann::json::array({ {{"text", headline.title}} })}
        };

        // Now, push it back onto the contents array
        payload["contents"].push_back(content); 
    }

    std::string payload_str = payload.dump(); // Convert JSON object to string


    if (curl) {

        bool LLM_SUCESS = false;
        nlohmann::json parsed_data;
        while(!LLM_SUCESS) {
            

            std::cout << " calling LLM API for ticker " << task->ticker << "!" << std::endl;
            
            std::string response_data;  // To store the response
    
            // Set the URL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
            // Set the HTTP headers
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("x-goog-api-key: " + api_key).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");
    
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
            // Set the POST data (the JSON request body)
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    
            // Set the callback function to write the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
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
                parsed_data = nlohmann::json::parse(response_data);
    
                if (parsed_data["error"].is_null()) {
                    std::cout << "No error found! Parsing output ... " << std::endl;
                    LLM_SUCESS = true;
                    
                } else {
                    std::cout << "ticker " << task->ticker << " Error: " << parsed_data["error"] << "\nRetrying..." << std::endl;
                    LLM_SUCESS = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(30000));
                }
    
            } // end while

        } // end if

        std::cout << " finished LLM API grab for ticker " << task->ticker << "!" << std::endl;

    }




    
}





    