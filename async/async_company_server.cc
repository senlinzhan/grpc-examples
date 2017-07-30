#include "company.grpc.pb.h"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <grpc/support/log.h>
#include <chrono>

using company::Company;
using company::AgeRange;
using company::Employee;
using company::EmployeeID;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

int32_t                               nextID_ = 0;
std::mutex                            mtx_;
std::unordered_map<int32_t, Employee> employees_;

class AddEmployeeCallData
{
public:
    AddEmployeeCallData(Company::AsyncService *service, ServerCompletionQueue *queue)
        : service_(service),
          queue_(queue),
          responder_(&ctx_),
          status_(CREATE)
    {
        Proceed();
    }

    void Proceed()
    {
        if (status_ == CREATE)
        {
            status_ = PROCESS;
            service_->RequestAddEmployee(&ctx_, &request_, &responder_, queue_, queue_, this);
        }
        else if (status_ == PROCESS)
        {
            new AddEmployeeCallData(service_, queue_);

            {
                std::lock_guard<std::mutex> guard(mtx_);
                employees_[nextID_] = request_;
                reply_.set_id(nextID_);
                ++nextID_;                
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));            
            status_ = FINISH;
            responder_.Finish(reply_, Status::OK, this);
        }
        else
        {
            GPR_ASSERT(status_ == FINISH);
            delete this;
        }
    }
    
private:
    Company::AsyncService* service_;
    ServerCompletionQueue* queue_;
    ServerContext ctx_;

    Employee request_;
    EmployeeID reply_;
    ServerAsyncResponseWriter<EmployeeID> responder_;

    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_; 
};


class ServerImpl final
{
public:
    ServerImpl(const std::string &addr)
    {
        ServerBuilder builder;        
        builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        queue_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << addr << std::endl;        
    }

    ~ServerImpl()
    {
        server_->Shutdown();

        // Always shutdown the completion queue after the server.
        queue_->Shutdown();        
    }

    void Run()
    {
        new AddEmployeeCallData(&service_, queue_.get());

        void *tag; 
        bool ok;

        while (true)
        {
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or queue_ is shutting down.
            GPR_ASSERT(queue_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<AddEmployeeCallData *>(tag)->Proceed();
        }
    }    
    
private:
    std::unique_ptr<ServerCompletionQueue> queue_;
    Company::AsyncService service_;
    std::unique_ptr<Server> server_;    
};

int main(int argc, char** argv)
{
    ServerImpl server("localhost:5000");
    server.Run();
    
    return 0;
}
