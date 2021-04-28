#include <iostream>
#include <boost/json/src.hpp>
#include "../include/json_rpc.hpp"

using namespace std;
using namespace boost::json;

void show_json()
{
    object obj;                                              // construct an empty object
    obj["pi"] = 3.141;                                       // insert a double
    obj["happy"] = true;                                     // insert a bool
    obj["name"] = "Boost";                                   // insert a string
    obj["nothing"] = nullptr;                                // insert a null
    obj["answer"].emplace_object()["everything"] = 42;       // insert an object with 1 element
    obj["list"] = {1, 0, 2};                                 // insert an array with 3 elements
    obj["object"] = {{"currency", "USD"}, {"value", 42.99}}; // insert an object with 2 elements

    cout << obj << endl;
}