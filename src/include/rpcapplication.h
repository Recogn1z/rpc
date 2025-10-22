#ifndef _rpcapplication_H
#define _rpcapplication_H
#include "rpcconfig.h"
//#include "rpcchannel.h"
//#include "rpccontroller.h"
#include<mutex>

class RpcApplication {
    public:
        static void Init(int argc, char** argv);
        static RpcApplication& GetInstance();
        static void deleteInstance();
        static rpcconfig& GetConfig();
    private:
        static rpcconfig m_config;
        static RpcApplication* m_application;
        static std::mutex m_mutex;
        RpcApplication() {}
        ~RpcApplication() {}
        RpcApplication(const RpcApplication&) = delete;
        RpcApplication(RpcApplication&&)=delete;
};
#endif