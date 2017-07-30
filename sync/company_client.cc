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
        stub_->AddEmployee(&context, employee, &id);
        std::cout << "AddEmployee() - new id: " << id.id() << std::endl;
    }

    void ListEmployeesByAge(int32_t low, int32_t high)
    {
        AgeRange range;
        range.set_low(low);
        range.set_high(high);
        grpc::ClientContext context;
        auto reader = stub_->ListEmployees(&context, range);
        Employee employee;
        while (reader->Read(&employee))
        {
            std::cout << "Employee: name = " << employee.name() << ", age = " << employee.age() << std::endl;
        }
    }

private:
    std::unique_ptr<Company::Stub> stub_;
};

int main(int argc, char *argv[])
{
    auto channel = grpc::CreateChannel("localhost:5000", grpc::InsecureChannelCredentials());

    CompanyClient client(channel);
    client.AddEmployee("hello", 10);
    client.AddEmployee("world", 20);
    client.ListEmployeesByAge(0, 100);

    return 0;
}
