#include "include/zookeeperutil.h"
#include "include/rpcchannel.h"
#include "rpcheader.pb.h"
#include "include/rpccontroller.h"
#include "include/rpcapplication.h"
#include "include/rpcLogger.h"
#include "memory"
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

std::mutex g_data_mutx;

void rpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
    ::google::protobuf::RpcController *controller,
    const ::google::protobuf::Message *request,
    ::google::protobuf::Message *response,
    ::google::protobuf::Closure *done){
        if(-1 == m_clientfd) {
            const google::protobuf::ServiceDescriptor* sd  =method->service();
            service_name = sd->name();
            method_name = method->name();

            ZkClient zkCli;
            zkCli.Start();
            std::string host_data = QueryServiceHost(&zkCli, service_name, method_name, m_idx);
            m_ip = host_data.substr(0, m_idx);
            std::cout << "ip: " << m_ip << std::endl;
            m_port = atoi(host_data.substr(m_idx + 1, host_data.size() - m_idx).c_str());
            std::cout<< "port: " << m_port <<std::endl;

            auto rt = newConnect(m_ip.c_str(), m_port);
            if(!rt) {
                LOG(ERROR) << "connect server error";
                return;
            }
            else {
                LOG(INFO) << "connect server success";
            }
        }

        uint32_t args_size{};
        std::string args_str;
        if(request->SerializeToString(&args_str)) {
            args_size = args_str.size();
        }
        else {
            controller->SetFailed("serialize request fail");
            return;
        }

        rpc::RpcHeader rpcheader;
        rpcheader.set_service_name(service_name);
        rpcheader.set_method_name(method_name);
        rpcheader.set_args_size(args_size);

        uint32_t header_size = 0;
        std::string rpc_header_str;

        if(rpcheader.SerializeToString(&rpc_header_str)) {
            header_size = rpc_header_str.size();
        }
        else {
            controller->SetFailed("serialize rpc header error!");
            return;
        }

        std::string send_rpc_str;
        {
            google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
            google::protobuf::io::CodedOutputStream coded_ouput(&string_output);
            coded_ouput.WriteVarint32(static_cast<uint32_t>(header_size));
            coded_ouput.WriteString(rpc_header_str);
        }
        send_rpc_str += args_str;

        if(-1 == send(m_clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)){
            close(m_clientfd);
            char errtxt[512] = {};
            std::cout<< "send error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
            controller->SetFailed(errtxt);
            return;
        }

        char recv_buf[1024] = {0};
        int recv_size = 0;
        if(-1 == (recv_size = recv(m_clientfd, recv_buf, 1024, 0))) {
            char errtxt[512] = {};
            std::cout<< "send error: " << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;
            controller->SetFailed(errtxt);
            return;
        }


    }

bool rpcChannel::newConnect(const char *ip, uint16_t port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        char errtxt[512] = {0};
        std::cout << "socket error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;  
        LOG(ERROR) << "socket error:" << errtxt;  
        return false;
    }
    
       
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;  // IPv4
    server_addr.sin_port = htons(port);  // port
    server_addr.sin_addr.s_addr = inet_addr(ip);  // IP
        
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        close(clientfd);  
            char errtxt[512] = {0};
            std::cout << "connect error" << strerror_r(errno, errtxt, sizeof(errtxt)) << std::endl;  
            LOG(ERROR) << "connect server error" << errtxt;  
            return false;
    }
        m_clientfd = clientfd;  
        return true;
}
    
std::string rpcChannel::QueryServiceHost(ZkClient *zkclient, std::string service_name, std::string method_name, int &idx) {
std::string method_path = "/" + service_name + "/" + method_name;  
std::cout << "method_path: " << method_path << std::endl;
    
std::unique_lock<std::mutex> lock(g_data_mutx);  
std::string host_data_1 = zkclient->GetData(method_path.c_str()); 
lock.unlock();  
    if (host_data_1 == "") { 
        LOG(ERROR) << method_path + " is not exist!";  
        return " ";
    }
    
    idx = host_data_1.find(":"); 
    if (idx == -1) { 
        LOG(ERROR) << method_path + " address is invalid!"; 
        return " ";
    }
    
    return host_data_1; 
}
    

rpcChannel::rpcChannel(bool connectNow) : m_clientfd(-1), m_idx(0) {
    if (!connectNow) { 
        return;
    }
    
    auto rt = newConnect(m_ip.c_str(), m_port);
    int count = 3; 
    while (!rt && count--) {
        rt = newConnect(m_ip.c_str(), m_port);
    }
}

