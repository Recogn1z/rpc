#include "rpcprovider.h"
#include "rpcapplication.h"
//#include "rpcheader.pb.h"
//#include "rpcLogger.h"
#include <iostream>

void rpcProvider::NotifyService(google::protobuf::Service* service) {
    ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor* psd = service->GetDescriptor();

    std::string service_name = psd->name();
    int method_count = psd->method_count();
    std::cout<< "service_name=" <<service_name <<std::endl;

    for(int i = 0; i < method_count; i++) {
        const google::protobuf::MethodDescriptor* pmd = psd->method(i);
        std::string method_name = pmd->name();
        std::cout<< "method_name=" <<method_name <<std::endl;
        service_info.method_map.emplace(method_name, pmd);
    }
    service_info.service = service;
    service_map.emplace(service_name, service_info);
}

void rpcProvider::Run() {
    //read config file's rpc server ip and port
    std::string ip = RpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(RpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    
    //use muduo to create address obj
    muduo::net::InetAddress address(ip, port);
    
    //create Tcpserver obj
    std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "rpcPoriver");

    server->setConnectionCallback(std::bind(&rpcProvider::OnConnection, this, std::placeholders::_1));
    server->setMessageCallback(std::bind(&rpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //set muduo thread number
    server->setThreadNum(4);

    //register all rpc nodes' services to zookeeper, so that rcp client can find service in zookeeper
    ZkClient zkclient;
    zkclient.Start();

    for(auto& sp : service_map) {
        std::string service_path = "/" + sp.first;
        zkclient.Create(service_path.c_str(), nullptr, 0);
        for(auto& mp : sp.second.method_map) {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkclient.Create(method_path.c_str, method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    
    std::cout<< "RpcProvider start service at ip:" << ip << "port:" <<port << std::endl;
    server->start();
    event_loop.loop();

}   

void rpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn) {
    if(!conn->connected()) {
        conn->shutdown();
    }
}

void rpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp receive_time) {
    std::cout << "OnMessage" <<std::endl;
    std::string recv_buf = buffer->retrieveAllAsString();

    google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
    google::protobuf::io::CodedInputStream coded_input(&raw_input);
    
    uint32_t header_size{};
    coded_input.ReadVarint32(&header_size);

    std::string rpc_header_str;
    rpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size{};

    google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
    code_input.ReadString(&rpc_header_str, header_size);
    code_input.PopLimit(msg_limit);
    
    if(rpcHeader.ParseFromString(rpc_header_str)) {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else {
        rpcLogger::ERROR("rpcHeader parse error");
        return;
    }

    std::string args_str;
    bool read_args_success = coded_input.ReadString(&args_str, args_size);
    if(!read_args_success) {
        rpcLogger::ERROR("read args error");
        return;
    }

    auto it = service_map.find(service_map);
    if(it == service_map.end()) {
        std::cout<< service_name << "is not exist!" << std::endl;
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if(mit == it->second.methond_map.end()) {
        std::cout<< service_name << "." << method_name << "is not exist" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)) {
        std::cout << service_name << "." << method_name << " parse error!" << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    google::protobuf::Closure *done = google::protobuf::NewCallback<rpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(this, &rpcProvider::SendRpcResponse, conn, response);

    service->CallMethod(method, nullptr, request, response, done);
}

void rpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response) {
    std::string response_str;
    if(response->SerializeToString(&response_str)) {
        conn->send(response_str);
    }
    else {
        std::cout<< "serialize error!" <<std::endl;
    }
}

rpcProvider::~rpcProvider() {
    std::cout<< "~rpcProvider()" <<std::endl;
    event_loop.quit();
}