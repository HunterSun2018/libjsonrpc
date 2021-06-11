#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <thread>
#include "root_certificates.hpp"
#include "../include/http_client.hpp"
#include "utils.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace asio = boost::asio;     // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
using asio::awaitable;
using asio::detached;
using asio::use_awaitable;

using namespace std;

auto parse_url(std::string_view uri_view)
{
    string uri(uri_view);

    //parse URI
    std::size_t start = uri.find("://", 0);
    if (start == std::string::npos)
        std::__throw_runtime_error("cannot parse url");

    start += 3; //"://"

    std::size_t end = uri.find("/", start + 1);
    if (end == std::string::npos)
        std::__throw_runtime_error(format("cannot find path character '/' in URL \"", uri_view, "\"").c_str());

    std::string protocol = uri.substr(0, start - 3);
    std::string host = uri.substr(start, end - start);
    std::string path = uri.substr(end);

    return make_tuple<>(protocol, host, path);
}

awaitable<string> send_http_request(
    string host,
    string port,
    http::request<http::string_body> request,
    asio::io_context &ioc,
    ssl::context &ctx)
{
    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
    beast::error_code ec;
    auto expired_time = std::chrono::seconds(15);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
    {
        __throw_runtime_error("failed in SSL_set_tlsext_host_name");
    }

    // Look up the domain name
    auto results = co_await resolver.async_resolve(host, port, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(expired_time);

    // Make the connection on the IP address we get from a lookup
    co_await get_lowest_layer(stream).async_connect(results, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(expired_time);

    // Perform the SSL handshake
    co_await stream.async_handshake(ssl::stream_base::client, use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(expired_time);

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, request, use_awaitable);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buf;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    co_await http::async_read(stream, buf, res, use_awaitable);

    std::ostringstream data;
    data << res.body();

    // // Set the timeout.
    // beast::get_lowest_layer(stream).expires_after(expired_time);

    // // // Gracefully close the stream
    // // beast::get_lowest_layer(stream).cancel();

    // co_await stream.async_shutdown(use_awaitable);

    // // //beast::get_lowest_layer(stream).close();

    // // // Rationale:
    // // // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
    // // std::cout << "end of sesseion" << std::endl;

    beast::get_lowest_layer(stream).socket().shutdown(asio::socket_base::shutdown_both);

    co_return data.str();
}

class HttpClientImp : public HttpClient
{
public:
    HttpClientImp() : _io_ctx(1), _ssl_ctx{ssl::context::tlsv12_client}
    {
        // This holds the root certificate used for verification
        load_root_certificates(_ssl_ctx);

        // Verify the remote server's certificate
        _ssl_ctx.set_verify_mode(ssl::verify_none);
    }

    virtual Response
    co_send_request(std::string_view url, RequestType type, std::string_view content_type, std::string_view content) override
    {
        return Response(*this, url, type, content_type, content);
    }

    void async_run()
    {
        // boost::asio::signal_set signals(_io_ctx, SIGINT, SIGTERM);
        // signals.async_wait([&](auto, auto) { _io_ctx.stop(); });

        thread([this]()
               { _io_ctx.run(); })
            .detach();
    }

    void sync_run()
    {
        _io_ctx.run();
    }

    boost::asio::io_context &io_ctx() { return _io_ctx; }

    ssl::context &ssl_ctx() { return _ssl_ctx; }

private:
    boost::asio::io_context _io_ctx;

    // The SSL context is required, and holds certificates
    ssl::context _ssl_ctx{ssl::context::tlsv12_client};
};

auto get_request_type(HttpClient::RequestType type)
{
    switch (type)
    {
    case HttpClient::RequestType::Get:
        return http::verb::get;
    case HttpClient::RequestType::Post:
        return http::verb::post;
    default:
        __throw_runtime_error("");
    }
};

void HttpClient::Response::operator()()
{
    auto [protocol, host, path] = parse_url(_url);

    http::request<http::string_body> req{get_request_type(_type), path, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    if (!_content_type.empty())
        req.set(http::field::content_type, string(_content_type));

    if (!_content.empty())
    {
        req.set(http::field::content_length, to_string(_content.length()));
        req.body() = _content;
    }
    req.prepare_payload();

    auto client = static_cast<HttpClientImp *>(&_client);
    auto port = protocol == "https" ? "443" : "80";

    // Launch the asynchronous operation
    co_spawn(client->io_ctx(),
             send_http_request(host, port, req, client->io_ctx(), client->ssl_ctx()),
             [this](std::exception_ptr eptr, std::string response)
             {
                 try
                 {
                     if (eptr)
                         std::rethrow_exception(eptr);

                     // save response
                     _value = response;
                 }
                 catch (const std::exception &e)
                 {
                     std::cerr << e.what() << '\n';
                 }

                 // resume promise type
                 _co_handle.resume();
             });
}

std::shared_ptr<HttpClient> HttpClient::create()
{
    return std::make_shared<HttpClientImp>();
}