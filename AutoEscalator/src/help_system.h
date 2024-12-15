#pragma once
#include <string>
#include <vector>
#include <map>

class HelpSystem {
public:
    struct HelpTopic {
        std::string title;
        std::string content;
        std::vector<std::string> keywords;
    };
    
private:
    std::map<std::string, HelpTopic> topics;
    
public:
    HelpSystem();
    
    void showHelp(const std::string& topic = "") const;
    void showTopicList() const;
    std::vector<std::string> searchHelp(const std::string& keyword) const;
    
private:
    void initializeTopics();
}; 