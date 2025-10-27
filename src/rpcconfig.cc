#include "rpcconfig.h"
#include "memory"

void rpcconfig::LoadConfigFile(const char* config_file) {
    std::unique_ptr<FILE, decltype(&fclose)> pf(fopen(config_file, "r"), &fclose);

    if(pf==nullptr) {
        exit(EXIT_FAILURE);
    }
    char buf[1024];
    while(fgets(buf,1024,pf.get())!=nullptr) {
        std::string read_buf(buf);
        Trim(read_buf);
        if(read_buf[0]=='#' || read_buf.empty()) continue;
        int index = read_buf.find('=');
        if(index == -1) continue;
        std::string key = read_buf.substr(0, index);
        Trim(key);
        int endindex = read_buf.find('\n', index);
        std::string value = read_buf.substr(index + 1, endindex - index - 1);
        Trim(value);
        config_map.insert({key, value});
    }
}

std::string rpcconfig::Load(const std::string& key) {
    auto it = config_map.find(key);
    if(it == config_map.end()) {
        return "";
    }
    return it->second;
}

void rpcconfig::Trim(std::string& read_buf) {
    int index = read_buf.find_first_not_of(' ');
    if(index != -1) {
        read_buf = read_buf.substr(index, read_buf.size() - index);
        index = read_buf.find_last_not_of(' ');
        if(index != -1) {
            read_buf = read_buf.substr(0, index + 1);
        }
    }
}