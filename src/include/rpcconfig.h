#ifndef _rpcconfig_H
#define _rpcconfig_H
#include <unordered_map>
#include <string>

class rpcconfig{
    public:
        void LoadConfigFile(const char* config_file); // load config file
        std::string Load(const std::string& key);
    private:
        std::unordered_map<std::string, std::string>config_map;
        void Trim(std::string& read_buf); // move the space from string
};


#endif