#ifndef RPC_LOG_H
#define RPC_LOG_H
#include<glog/logging.h>
#include<string>

class rpcLogger {
    public:
        explicit rpcLogger(const char* argv0) {
            google::InitGoogleLogging(argv0);
            FLAGS_colorlogtostderr = true;
            FLAGS_logtostderr = true;
        }
        ~rpcLogger() {
            google::ShutdownGoogleLogging();
        }
        static void Info(const std::string& message) {
            LOG(INFO) << message;
        }
        static void Warning(const std::string& message) {
            LOG(WARNING) << message;
        }
        static void ERROR(const std::string& message) {
            LOG(ERROR) << message;
        }
        static void Fatal(const std::string& message) {
            LOG(FATAL) <<message;
        }
        
    private:
        rpcLogger(const rpcLogger&) = delete;
        rpcLogger& operator=(const rpcLogger&) = delete;
};
#endif