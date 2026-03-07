
#pragma once

#include <iostream>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include "structs.h"

#include <curl/curl.h>
#include "tinyxml2.h"
#include <nlohmann/json.hpp>

class RSSFetcher
{

public:
    
    // Constructor
    RSSFetcher();

    // Destructor
    ~RSSFetcher();

    void addItem(RSSInputData* input_data);
    void taskManager();
    void processTask(RSSInputData* task, CURL* curl, int i);
    void parseRSS(RSSInputData* task);
    void callLLMAPI(RSSInputData* task, CURL* curl);

private:

    // Variables for a queue of RRSFetches and score assigns
    std::queue<RSSInputData*> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<int> atomicIndex;

};