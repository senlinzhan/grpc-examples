#include "company.grpc.pb.h"

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

class CompanyClient
{
public:
    CompanyClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(Company::NewStub(channel))  { }

    void AddEmployee(const std::string &name, int32_t age)
    {
        Employee employee;
        employee.set_name(name);
        employee.set_age(age);
        EmployeeID id;
        grpc::ClientContext context;
        auto status = stub_->AddEmployee(&context, employee, &id);
        if (status.ok())
        {
            std::cout << "AddEmployee() successed, new id is " << id.id() << std::endl;                            
        }
        else
        {
            std::cout << "AddEmployee() failed" << std::endl;                            
        }
    }

    void ListEmployeesByAge(int32_t low, int32_t high)
    {
        AgeRange range;
        range.set_low(low);
        range.set_high(high);
        grpc::ClientContext context;

        Employee employee;        
        auto reader = stub_->ListEmployees(&context, range);
        std::cout << "call ListEmployees() ..." << std::endl;
        
        while (reader->Read(&employee))
        {
            std::cout << "Employee: name = " << employee.name() << ", age = " << employee.age() << std::endl;
        }

        auto status = reader->Finish();
        if (status.ok())
        {
            std::cout << "ListEmployees() successed" << std::endl;                                        
        }
        else
        {
            std::cout << "ListEmployees() failed" << std::endl;
        }
    }

private:
    std::unique_ptr<Company::Stub> stub_;
};

int main(int argc, char *argv[])
{
    auto channel = grpc::CreateChannel("localhost:5000", grpc::InsecureChannelCredentials());
    CompanyClient client(channel);
        
    for (int32_t i = 0; i < 100; ++i)
    {
        std::string name = "hello" + std::to_string(i);        
        client.AddEmployee(name, i);
    }

    client.ListEmployeesByAge(0, 100);
    
    return 0;
}
