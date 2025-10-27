#include "./include/rpcapplication.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>

rpcconfig RpcApplication::m_config;
std::mutex RpcApplication::m_mutex;
RpcApplication* RpcApplication::m_application = nullptr;

void RpcApplication::Init(int argc, char** argv) {
    if(argc<2) {
        std::cout << "format: command -i <file path>" << std::endl;
        exit(EXIT_FAILURE);
    }

    int o;
    std::string config_file;
    while(-1 != (o = getopt(argc, argv, "i:"))) {
        switch (o)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << "Format: command -i <file path>" << std::endl;
            exit(EXIT_FAILURE);
            break;
        case ':':
            std::cout<< "Format: command -i <file path>" <<std::endl;
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }
    m_config.LoadConfigFile(config_file.c_str());
}

RpcApplication& RpcApplication::GetInstance() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_application ==nullptr) {
        m_application = new RpcApplication();
        atexit(deleteInstance);
    }
    return *m_application;
}

void RpcApplication::deleteInstance() {
    if(m_application) {
        delete m_application;
    }
}

rpcconfig& RpcApplication::GetConfig() {
    return m_config;
}