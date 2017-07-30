#include "company.grpc.pb.h"

#include <thread>
#include <iostream>
#include <memory>
#include <string>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

using company::Company;
using company::Employee;
using company::EmployeeID;
using company::AgeRange;

using grpc::Status;
using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;
using grpc::ClientContext;

struct AsyncAddEmployeeCall
{
    EmployeeID result;
    ClientContext context;
    Status status;
    std::unique_ptr<ClientAsyncResponseReader<EmployeeID>> responseReader;
};

class AsyncCompanyClient
{
public:
    AsyncCompanyClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(Company::NewStub(channel))  { }

    void AsyncAddEmployee(const std::string &name, int32_t age)
    {
        Employee employee;
        employee.set_name(name);
        employee.set_age(age);
        
        auto call = new AsyncAddEmployeeCall;        
        call->responseReader = stub_->AsyncAddEmployee(&call->context, employee, &queue_);
        call->responseReader->Finish(&call->result, &call->status, (void*)call);
        
        std::cout << "make AsyncAddEmployee() call ..." << std::endl;
    }

    void Loop()
    {
        void *tag;
        bool ok = false;

        while (queue_.Next(&tag, &ok))
        {
            auto *call = static_cast<AsyncAddEmployeeCall *>(tag);

            if (call->status.ok())
            {
                std::cout << "AsyncAddEmployee() successed, new id is " << call->result.id() << std::flush << std::endl;                
            }
            else
            {
                std::cout << "AsyncAddEmployee() failed" << std::endl;                
            }

            delete call;            
        }
    }
    
private:
    std::unique_ptr<Company::Stub> stub_;
    CompletionQueue queue_;
};

int main(int argc, char *argv[])
{
    auto channel = grpc::CreateChannel("localhost:5000", grpc::InsecureChannelCredentials());
    AsyncCompanyClient client(channel);
    
    // Spawn reader thread that loops indefinitely
    std::thread thread_ = std::thread(&AsyncCompanyClient::Loop, &client);

    for (int32_t i = 0; i < 1000; i++) {
        std::string name("hello" + std::to_string(i));
        client.AsyncAddEmployee(name, i);
    }

    std::cout << "Press control-c to quit" << std::endl << std::endl;
    thread_.join();  //blocks forever

    return 0;
}
