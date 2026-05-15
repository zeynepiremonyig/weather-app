#include "WeatherClient.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

// Constructor simply stores API key for later requests.
WeatherClient::WeatherClient(std::string key) : apiKey(std::move(key)) {}

std::string WeatherClient::urlEncode(const std::string& value) {
    // URL encoding converts unsafe URL characters into %XX format.
    // Example: space becomes %20.
    // This matters for city names with spaces or special characters.
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex << std::uppercase;

    for (const unsigned char ch : value) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            encoded << ch;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(ch);
        }
    }

    return encoded.str();
}

std::string WeatherClient::fetchWeather(const std::string& city) {
    // This method does a full HTTPS GET request manually using Boost.Beast.
    // Steps:
    // 1) Build endpoint path
    // 2) Resolve DNS
    // 3) Open TCP connection
    // 4) Start TLS handshake
    // 5) Send HTTP request
    // 6) Read response
    // 7) Validate status code
    // 8) Return response body as plain string
    const std::string host = "api.weatherapi.com";
    const std::string port = "443";
    const std::string target = "/v1/current.json?key=" + apiKey + "&q=" + urlEncode(city);
    constexpr int httpVersion = 11;

    // io_context is the central engine that drives async/sync I/O operations.
    net::io_context ioc;

    // TLS context stores SSL/TLS settings.
    // verify_peer means we verify server certificate chain.
    ssl::context ctx(ssl::context::tls_client);
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    // SNI tells the server which host we are asking for.
    // Without SNI, some hosts return wrong certificate in shared hosting setups.
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        throw std::runtime_error("Failed to set TLS server name indication.");
    }

    // DNS lookup: convert host name to one or more IP addresses.
    const auto results = resolver.resolve(host, port);
    // TCP connect to one of the resolved endpoints.
    beast::get_lowest_layer(stream).connect(results);

    // TLS handshake upgrades plain TCP connection to secure encrypted channel.
    stream.handshake(ssl::stream_base::client);

    // Build HTTP GET request.
    http::request<http::string_body> req{http::verb::get, target, httpVersion};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send request to server.
    http::write(stream, req);

    beast::flat_buffer buffer;
    // Receive full HTTP response.
    http::response<http::dynamic_body> res;
    http::read(stream, buffer, res);

    // We only accept HTTP 200 OK as success.
    // Any other status returns detailed error text.
    if (res.result() != http::status::ok) {
        const std::string body = beast::buffers_to_string(res.body().data());
        throw std::runtime_error(
            "Weather API request failed with status " +
            std::to_string(static_cast<unsigned>(res.result_int())) + ". Response: " + body
        );
    }

    const std::string responseBody = beast::buffers_to_string(res.body().data());

    // Try graceful TLS shutdown.
    beast::error_code ec;
    stream.shutdown(ec);

    // Important:
    // Some real-world servers close the connection in a "good enough" way
    // and do not send strict TLS close frames.
    // In those cases Boost may report eof or stream_truncated.
    // For a simple one-request client, we treat these as non-fatal.
    if (ec == net::error::eof || ec == ssl::error::stream_truncated) {
        ec = {};
    }
    if (ec) {
        throw std::runtime_error("TLS shutdown failed: " + ec.message());
    }

    return responseBody;
}