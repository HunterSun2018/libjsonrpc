#include <iostream>
#include "../include/co_helper.hpp"
#include "../include/http_client.hpp"
#include "../include/json_rpc.hpp"

using namespace std;

co_helper::Task<string>
test_http_client(http_client_ptr client)
{
    string ret = co_await client->aw_get("https://cn.bing.com/?mkt=zh-CN"); // /?mkt=zh-CN

    cout << "The http Get request is done. " << endl;

    co_return ret;
}

co_helper::Task<void>
test_task_void(http_client_ptr client)
{
    string ret = co_await client->aw_get("https://cn.bing.com/");

    cout << " test_task_void ends. " << endl;

    co_return;
}

co_helper::Task<string>
test_http_post(http_client_ptr client)
{
    string ret = co_await client->aw_get("https://cn.bing.com/");

    cout << " test_task_void ends. " << endl;

    co_return ret;
}

void test()
{
    auto client = HttpClient::create();

    // test_task_void(client);

    auto ret = test_http_client(client);

    client->async_run();

    getchar();

    cout << "co_return result : " << ret.get() << endl;
}

//curl -X POST -H "Content-Type: application/json" --data '[{"jsonrpc":"2.0","method":"get_metadata","params":[1],"id":1},{"jsonrpc":"2.0","method":"get_metadata","params":[9],"id":2}]' "https://client.testnet.diem.com/"

int main(int argc, char *argv[])
{
    try
    {
        jsonrpc::client jr_client;

        jr_client.call(1, "get_metadata", 1);

        test();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception : " << e.what() << '\n';
    }

    return 0;
}