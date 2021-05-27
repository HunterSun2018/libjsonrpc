#include <iostream>
#include "../include/co_helper.hpp"
#include "../include/http_client.hpp"
#include "../include/json_rpc.hpp"

using namespace std;

co_helper::Task<string>
test_http_client(http_client_ptr client)
{
    string ret = co_await client->co_get("https://cn.bing.com/?mkt=zh-CN"); // /?mkt=zh-CN

    cout << "The http Get request is done. " << endl;

    co_return ret;
}

co_helper::Task<void>
test_task_void(http_client_ptr client)
{
    string ret = co_await client->co_get("https://cn.bing.com/");

    cout << " http get response : " << ret << endl;

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
    string response = co_await client->co_post("https://client.testnet.diem.com/", "application/json", R"([{"jsonrpc":"2.0","method":"get_metadata","params":[1],"id":1},{"jsonrpc":"2.0","method":"get_metadata","params":[9],"id":2}])");
    
    cout << " http post response : " << response << endl;

    co_return response;
}

void test()
{
    auto client = HttpClient::create();

    // test_task_void(client);

    // auto ret = test_http_client(client);

    auto ret = test_http_post(client);

    // client->async_run();
    // getchar();

    client->sync_run();

    cout << "test finished." << endl;
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

        fail().get();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception : " << e.what() << '\n';
    }

    return 0;
}