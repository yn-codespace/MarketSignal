
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <fstream>

#include <nlohmann/json.hpp>

#include "file_reader.h"
#include "structs.h"
#include "RSSFetcher.h"


std::vector<RssSource> urlSources = {
    // {"Google News", "https://news.google.com/rss/search?q=", "+stock&hl=en-US&gl=US&ceid=US:en"}, // (good)
    // {"Yahoo Finance", "https://feeds.finance.yahoo.com/rss/2.0/headline?s=", "&region=US&lang=en-US"}, // (bad)
    // {"MSNBC Money", "https://www.msnbc.com/feeds/rss/money/search?q=", ""}, // (bad)
    {"Seeking Alpha", "https://seekingalpha.com/api/sa/combined/", ".xml"} // (good)
    // {"Bing", "https://www.bing.com/news/search?q=", "+stock&format=rss"}, // (good)
    // {"Duck Duck Go", "https://duckduckgo.com/rss?q=", "+stock"} // (bad)

    
    // {"Google News (earnings)", "https://news.google.com/rss/search?q=", "+earnings&hl=en-US&gl=US&ceid=US:en"}

    // {"Google News (analyst)", "https://news.google.com/rss/search?q=", "+analyst+rating&hl=en-US&gl=US&ceid=US:en"},

    // {"Bing News (earnings)", "https://www.bing.com/news/search?q=", "+earnings&format=rss"},

    // {"Bing News (upgrade)", "https://www.bing.com/news/search?q=", "+analyst+upgrade&format=rss"}
    // add more sources here...
};


int main()
{   
    std::cout << " inside main.cpp " << std::endl;
    
    // Grab the ticker symbols
    std::string file_path = "../symbols/all_tickers.txt";
    std::vector<std::string> tickers;
    tickers         = read_txt_file(file_path);
    int len_tickers = tickers.size(); 

    std::vector<RSSInputData*> tickerDataPtrArr(len_tickers);
    std::vector<RSSInputData> tickerDataArr(len_tickers);        

    // std::string str_prefix = "https://news.google.com/rss/search?q=";
    // std::string str_postix = "+stock&hl=en-US&gl=US&ceid=US:en";
    // std::string search_str;
    
    std::string search_str;

    std::cout << "urlSources.size() = " << urlSources.size() << std::endl;

    
    int n_tickers = len_tickers;
    std::string ticker_i;
    for (int i = 0; i < n_tickers; i++) {
        
        ticker_i                = tickers[i];
    
        tickerDataArr[i].ticker = ticker_i;
    
        for (int j = 0; j < urlSources.size(); j++) {
        
            search_str = urlSources[j].str_prefix + ticker_i + urlSources[j].str_postix;
    
            tickerDataArr[i].urls.push_back(search_str);
            tickerDataArr[i].success = true;

            std::cout << "URL = " << tickerDataArr[i].urls[j] << std::endl;
        }
 
    }
    
    // return 0; 

    // Populate the array with pointers to the objects
    for (int i = 0; i < n_tickers; ++i) {
        tickerDataPtrArr[i] = &tickerDataArr[i];  // Point to the original array element
        
    }

    // Create an RSSFecther class object
    RSSFetcher rssFetcher;
    
    // Populate the queue with tasks
    for (int i = 0; i < n_tickers; ++i) {
        rssFetcher.addItem(tickerDataPtrArr[i]);       
    }

    // Launch workers
    int n_threads = 8; // tune based on your CPU/network
    if (n_threads > n_tickers)
        n_threads = n_tickers; // override with min necessary threads

    std::vector<std::thread> workers;
    for (int i = 0; i < n_threads; ++i) {
        workers.emplace_back(&RSSFetcher::taskManager, &rssFetcher);
    }


    // Join threads to block main()
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();  // wait for thread to finish
        }
    }

    nlohmann::json all_data;  // JSON object to hold everything
    all_data["tickers"] = nlohmann::json::array();

    int total_headlines = 0;
    for (int i = 0; i < n_tickers; i++) {
        
        // Setup the json object for this ticker's headers
        nlohmann::json ticker_json;
        ticker_json["ticker"] = tickerDataPtrArr[i]->ticker;
        ticker_json["headlines"] = nlohmann::json::array();

        // Extract the headlines
        std::vector<Headline> headlines = tickerDataPtrArr[i]->headlines;

        // Iterate through the headlines and extract the data
        std::cout << "i = " << i << " ticker = " << tickerDataPtrArr[i]->ticker << "\n\n";
        for (const auto& h : headlines) {
            std::cout <<    " title    = "  << h.title << std::endl <<
                            " link     = "  << h.link << std::endl <<
                            " pubDate  = "  << h.pubDate << "\n" << std::endl;


            // Put the headline into a headline json
            nlohmann::json headline_json;
            headline_json["title"] = h.title;
            headline_json["link"] = h.link;
            headline_json["pubDate"] = h.pubDate;
            headline_json["description"] = h.description;
            headline_json["source"] = h.source;

            ticker_json["headlines"].push_back(headline_json);


        } // end headlines loop

            // Store this data in the ticker json
            total_headlines = total_headlines + 1;
            all_data["tickers"].push_back(ticker_json);

    } // end ticker loop


        

    
    // Optionally, add total headlines
    all_data["total_headlines"] = total_headlines;

    std::cout << " total headlines found = " << total_headlines << std::endl;

    // Save out the json
    std::cout << " Saving results to json... " << std::endl;

    std::string filename = "../headlines.json";
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << all_data.dump(4);  // <-- noncompact JSON, indentation
        outfile.close();
        std::cout << "Saved JSON to " << filename << std::endl;
    } else {
        std::cerr << "Failed to open file for writing" << std::endl;
    }



    
    std::cout << " finished main.cpp " << std::endl;
}