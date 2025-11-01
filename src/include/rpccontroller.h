#ifndef _rpccontroller_H
#define _rpccontroller_H

#include<google/protobuf/service.h>
#include<string>

class rpccontroller:public google::protobuf::RpcController {
    public:
        rpccontroller();
        void Reset();
        bool Failed() const;
        std::string ErrorText() const;
        void SetFailed(const std::string &reason);
        void StartCancel();
        bool IsCanceled() const;
        void NotifyOnCancel(google::protobuf::Closure* callback);
    private:
        bool m_failed;
        std::string m_errText;
};
#endif