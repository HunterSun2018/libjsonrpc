#ifndef CO_HTTP_CLIENT_HPP
#define CO_HTTP_CLIENT_HPP
#include <string>
#include <memory>
#include "../include/co_helper.hpp"

/**
 * @brief A http client class implemented by c++ 20 corountine
 * 
 */
class HttpClient
{
public:
    static std::shared_ptr<HttpClient> create();
    virtual ~HttpClient() {}

    virtual void async_run() = 0;

    virtual void sync_run() = 0;

    enum RequestType
    {
        Get,
        Post,
    };

    //template <typename T>
    struct Response : public co_helper::Awaitable<std::string, Response>
    {
        Response(HttpClient &client,
                 std::string_view url,
                 RequestType type,
                 std::string_view content_type,
                 std::string_view content)
            : _client(client),
              _url(url),
              _type(type),
              _content_type(content_type),
              _content(content)
        {
        }

        void operator()();

    private:
        HttpClient &_client;
        RequestType _type;
        std::string _url, _content_type, _content;
    };

    virtual Response co_send_request(std::string_view url,
                                     RequestType type = RequestType::Get,
                                     std::string_view content_type = "",
                                     std::string_view content = "") = 0;
};

using http_client_ptr = std::shared_ptr<HttpClient>;

#endif