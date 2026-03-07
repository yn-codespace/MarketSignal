
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>


#include "file_reader.h"
#include "structs.h"
#include "RSSFetcher.h"


std::vector<RssSource> urlSources = {
    {"Google News", "https://news.google.com/rss/search?q=", "+stock&hl=en-US&gl=US&ceid=US:en"}, // (good)
    // {"Yahoo Finance", "https://feeds.finance.yahoo.com/rss/2.0/headline?s=", "&region=US&lang=en-US"}, // (bad)
    // {"MSNBC Money", "https://www.msnbc.com/feeds/rss/money/search?q=", ""}, // (bad)
    {"Seeking Alpha", "https://seekingalpha.com/api/sa/combined/", ".xml"}, // (good)
    {"Bing", "https://www.bing.com/news/search?q=", "+stock&format=rss"}, // (good)
    // {"Duck Duck Go", "https://duckduckgo.com/rss?q=", "+stock"} // (bad)

    
    {"Google News (earnings)", "https://news.google.com/rss/search?q=", "+earnings&hl=en-US&gl=US&ceid=US:en"},

    {"Google News (analyst)", "https://news.google.com/rss/search?q=", "+analyst+rating&hl=en-US&gl=US&ceid=US:en"},

    {"Bing News (earnings)", "https://www.bing.com/news/search?q=", "+earnings&format=rss"},

    {"Bing News (upgrade)", "https://www.bing.com/news/search?q=", "+analyst+upgrade&format=rss"}
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

    // C-style arrays are contiguous
    RSSInputData** tickerDataPtrArr   = (RSSInputData**)calloc(len_tickers, sizeof(RSSInputData*));
    RSSInputData* tickerDataArr         = (RSSInputData*)calloc(len_tickers, sizeof(RSSInputData));


    // std::string str_prefix = "https://news.google.com/rss/search?q=";
    // std::string str_postix = "+stock&hl=en-US&gl=US&ceid=US:en";
    // std::string search_str;
    
    std::string search_str;

    int n_tickers = 3;
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
    
    for (int i = 0; i < n_tickers; i++) {
        
        std::vector<Headline> headlines = tickerDataPtrArr[i]->headlines;

        std::cout << "i = " << i << "\n\n";
        for (int j = 0; j < headlines.size(); j++) {
            std::cout << "iteration j = " << j << 
                            " title    = " << headlines[j].title << std::endl <<
                            " link     = "  << headlines[j].link << std::endl <<
                            " pubDate  = "  << headlines[j].pubDate << "\n" << std::endl;

        }
        
    }
        
    
    std::cout << " finished main.cpp " << std::endl;
}