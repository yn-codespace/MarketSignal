
#ifndef STRUCTS_H

#define STRUCTS_H

#include <string>
#include <vector>

struct Headline {
    std::string title;
    std::string link;
    std::string pubDate;
    std::string description;
    std::string source;
};

struct RSSInputData {
    std::vector<std::string> urls; // urls to search through
    std::string rssData; // storage for passing data around the queue

    std::string ticker;

    std::vector<Headline> headlines;

    bool success;
};


// --- Define your RSS sources ---
struct RssSource {
    std::string name;
    std::string str_prefix;  // URL before the search term
    std::string str_postix; // URL after the search term
};



#endif