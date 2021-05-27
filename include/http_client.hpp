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

    struct HttpGet : public co_helper::Awaitable<std::string, HttpGet>
    {
        HttpGet(HttpClient &client, std::string_view host, std::string_view port, std::string_view target, int version)
            : _client(client), _host(host), _port(port), _target(target), _version(version)
        {
        }

        void operator()();

    private:
        HttpClient &_client;
        std::string _host, _port, _target;
        int _version;
    };
    /**
     * @brief awaitable Http Get request
     * 
     * @param url 
     * @return HttpGet 
     */
    virtual HttpGet co_get(std::string_view url) = 0;

    struct HttpPost : public co_helper::Awaitable<std::string, HttpPost>
    {
        HttpPost(HttpClient &client,
                 std::string_view host,
                 std::string_view port,
                 std::string_view target,
                 int version,
                 std::string_view content_type,
                 std::string_view content)
            : _client(client),
              _host(host),
              _port(port),
              _target(target),
              _version(version),
              _content_type(content_type),
              _content(content)
        {
        }

        void operator()();

    private:
        HttpClient &_client;
        std::string _host, _port, _target;
        int _version;
        std::string _content_type;
        std::string _content;
    };

    /**
     * @brief Http Post request by co_await
     * 
     * @param url 
     * @param content 
     * @return HttpPost 
     */
    virtual HttpPost co_post(std::string_view url, std::string_view content_type, std::string_view content) = 0;
};

using http_client_ptr = std::shared_ptr<HttpClient>;

#endif