#include "company.grpc.pb.h"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

using company::Company;
using company::AgeRange;
using company::Employee;
using company::EmployeeID;

class CompanyImpl final : public Company::Service
{
public:
    CompanyImpl()
        : nextID_(0) { }
    grpc::Status AddEmployee(grpc::ServerContext *context, const Employee *request, 
                             EmployeeID *response) override
    {
        std::lock_guard<std::mutex> guard(mtx_);
        employees_[nextID_] = *request;
        response->set_id(nextID_);
        ++nextID_;
        return grpc::Status::OK;
    }
    grpc::Status ListEmployees(grpc::ServerContext *context, const AgeRange *request, 
                               grpc::ServerWriter<Employee> *writer) override
    {
        auto low = request->low();
        auto high = request->high();
        std::lock_guard<std::mutex> guard(mtx_);
        for (auto &entry : employees_)
        {
            auto employee = entry.second;
            if (employee.age() >= low && employee.age() <= high)
            {
                writer->Write(employee); // 调用 Write 写入一个 Employee 消息给client
            }
        }
        return grpc::Status::OK;
    }
private:
    int32_t                               nextID_;
    std::mutex                            mtx_;
    std::unordered_map<int32_t, Employee> employees_;
};

int main(int argc, char *argv[])
{
    std::string addr = "0.0.0.0:5000";
    CompanyImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    auto server = builder.BuildAndStart();
    std::cout << "Server listening on " << addr << std::endl;
    server->Wait();
    return 0;
}
