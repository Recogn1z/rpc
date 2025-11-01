#include "../src/include/rpcapplication.h"
#include "../user.pb.h"
#include "../src/include/rpccontroller.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include "../src/include/rpcLogger.h"
#include "../src/include/rpcchannel.h"

void send_request(int thread_id, std::atomic<int>& success_count, std::atomic<int>& fail_count, int requests_per_thread) {
    user::UserServiceRpc_Stub stub(new rpcChannel(false));

    user::LoginRequest request;
    request.set_name("hi");
    request.set_pwd("123");

    user::LoginResponse response;
    rpccontroller controller;

    for(int i =0; i < requests_per_thread; i++) {
        stub.Login(&controller, &request, &response, nullptr);

        if(controller.Failed()) {
            std::cout<< controller.ErrorText() << std::endl;
            fail_count++;
        }
        else {
            if(int{} == response.result().errcode()) {
                std::cout<< "rpc login response success: " << response.success() <<std::endl;
                success_count++;
            }
            else {
                std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
                fail_count++;
            }
        }
    }
}

int main(int argc, char** argv) {
    RpcApplication::Init(argc,argv);
    rpcLogger logger("MyRpc");
    const int thread_count =500;      
    const int requests_per_thread = 100; 

    std::vector<std::thread> threads; 
    std::atomic<int> success_count(0); 
    std::atomic<int> fail_count(0);    

    auto start_time = std::chrono::high_resolution_clock::now();  

    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back([argc, argv, i, &success_count, &fail_count, requests_per_thread]() {  
                send_request(i, success_count, fail_count,requests_per_thread);  
        });
    }


    for (auto &t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double> elapsed = end_time - start_time;  


    LOG(INFO) << "Total requests: " << thread_count * requests_per_thread;  
    LOG(INFO) << "Success count: " << success_count;  
    LOG(INFO) << "Fail count: " << fail_count; 
    LOG(INFO) << "Elapsed time: " << elapsed.count() << " seconds"; 
    LOG(INFO) << "QPS: " << (thread_count * requests_per_thread) / elapsed.count();  

    return 0;
}