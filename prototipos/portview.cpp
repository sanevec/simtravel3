#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <fstream>
#include <sstream>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

std::string leer_html(const std::string& ruta) {
    std::ifstream archivo(ruta);
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    return buffer.str();
}

void handle_get(http_request message) {
    message.reply(status_codes::OK, leer_html("./portview.html"), "text/html");
}

void handle_get_coordinates(http_request message) {
    json::value obj;
    obj[U("x")] = json::value::number(50);
    obj[U("y")] = json::value::number(50);
    obj[U("width")] = json::value::number(100);
    obj[U("height")] = json::value::number(100);
    message.reply(status_codes::OK, obj);
}

int main() {
    http_listener listener(U("http://localhost:18080"));
    listener.support(methods::GET, handle_get);

    http_listener listener_coordinates(U("http://localhost:18080/coordinates"));
    listener_coordinates.support(methods::GET, handle_get_coordinates);

    try {
        listener
            .open()
            .then([&listener_coordinates]() { listener_coordinates.open(); })
            .wait();

        while (true);
    }
    catch (std::exception const & e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
