#include <iostream>
#include "../include/co_helper.hpp"
#include "../include/http_client.hpp"
#include "../include/json_rpc.hpp"

using namespace std;

co_helper::Task<void>
test_http_get(http_client_ptr client)
{
    string ret = co_await client->co_send_request("https://cn.bing.com/");  //https://cn.bing.com/?mkt=zh-CN

    cout << "Http get response : " <<  endl;

    co_return;
}
// POST /test HTTP/1.1
// Host: foo.example
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 27
// field1=value1&field2=value2

//curl -X POST -H "Content-Type: application/json" --data '[{"jsonrpc":"2.0","method":"get_metadata","params":[1],"id":1},{"jsonrpc":"2.0","method":"get_metadata","params":[9],"id":2}]' "https://client.testnet.diem.com/"

co_helper::Task<string>
test_http_post(http_client_ptr client)
{
    // string response = co_await client->co_post("https://client.testnet.diem.com/", "application/json", R"([{"jsonrpc":"2.0","method":"get_metadata","params":[1],"id":1},{"jsonrpc":"2.0","method":"get_metadata","params":[9],"id":2}])");

    string response = co_await client->co_send_request("https://client.testnet.diem.com/",
                                                       HttpClient::RequestType::Post,
                                                       "application/json",
                                                       R"([{"jsonrpc":"2.0","method":"get_metadata","params":[1],"id":1},{"jsonrpc":"2.0","method":"get_metadata","params":[9],"id":2}])");

    cout << "Http post response : " << response << endl;

    co_return response;
}

std::future<void> test()
{
    auto client = HttpClient::create();

    test_http_get(client);
    cout << "run http get" << endl;

    auto ret = test_http_post(client);
    cout << "run http post" << endl;
    
    // client->async_run();
    // getchar();

    client->sync_run();

    cout << "exit finished." << endl;

    co_return;
}

// Utilize the infrastructure we have established.
std::future<int> compute()
{
    int a = co_await std::async([]
                                { return 6; });
    int b = co_await std::async([]
                                { return 7; });
    co_return a *b;
}

std::future<void> fail()
{
    throw std::runtime_error("bleah");
    co_return;
}

int main(int argc, char *argv[])
{
    std::cout << compute().get() << '\n';

    try
    {
        jsonrpc::client jr_client;

        jr_client.call(1, "get_metadata", 1);

        test();
        cout << "finished test" << endl;

        //fail().get();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception : " << e.what() << '\n';
    }

    return 0;
}