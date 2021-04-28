#ifndef JSON_RPC_HPP
#define JSON_RPC_HPP
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <boost/json.hpp>

namespace jsonrpc
{
    using namespace std;
    namespace json = boost::json;
    //using namespace boost::json;

    class request
    {
        json::object _obj;

    public:
        request(uint32_t id, const std::string &method_name)
        {
            _obj["jsonrpc"] = "2.0";
            _obj["id"] = id;
            _obj["method"] = method_name;
        }

        void set_params(const json::array &arr)
        {
            _obj["params"] = arr;
        }

        void print()
        {
            cout << _obj << endl;
        }
    };

    class json_response
    {
        json_response(uint32_t id, std::string_view method_name)
        {
        }
    };

    class json_notify
    {
        json_notify(uint32_t id, std::string_view method_name)
        {
        }
    };

    template <typename>
    struct function_meta;

    template <typename R, typename... T>
    struct function_meta<std::function<R(T...)>>
    {
        using return_type = std::decay_t<R>;
        using arguments_tuple_type = std::tuple<std::decay_t<T>...>;
    };

    class server
    {
    public:
        template <typename TFunc>
        void register_handler(std::string name, TFunc func)
        {
            if (_handles.find(name) != std::end(_handles))
            {
                throw std::invalid_argument{"The function \"" +
                                            name +
                                            "\" already exists."};
            }

            auto wrapper = [f = std::move(func)](std::string request) -> string {
                std::function func{std::move(f)};

                using function_type = function_meta<decltype(func)>;
                using arguments_tuple_type = typename function_type::arguments_tuple_type;
                //using return_type = typename function_meta::return_type;

                arguments_tuple_type arguments;
                std::istringstream iss(request);

                std::apply([&](auto &...args) { ((iss >> args), ...); }, arguments);

                auto ret = std::apply(std::move(func), std::move(arguments));

                return std::to_string(ret);
            };

            _handles.emplace(std::move(name), std::move(wrapper));
        }

    private:
        using handle = std::function<std::string(std::string)>;
        std::map<std::string, handle> _handles;
    };

    class client
    {
    public:
        template <typename... T>
        auto call(uint32_t id, const std::string &method, T... args)
        {
            request req(id, method);

            {
                json::array params;

                // std::ostringstream oss;
                // ((oss << args), ...);

                ((params.push_back(args)), ...);

                req.set_params(params);

                req.print();
            }

            std::string response; // = run(js.to_string());

            std::istringstream iss(response);
            //arg2_type ret;
            int ret;

            iss >> ret;

            return "";
        }
    };

} // namespace detail

#endif