//------------------------------------------------------------------------------
//
// HTTP server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "api/api_handler.h"

#define SERVER_VERSION_STRING "blabber/1.0 (Linux)"

namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using request_body_t = boost::beast::http::string_body;

// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// Returns a bad request response
template<class Body, class Allocator>
auto bad_request(http::request<Body, http::basic_fields<Allocator>>&& req, beast::string_view why)
{
    http::response<http::string_body> res{ http::status::bad_request, req.version() };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();

    return res;
};

// Returns a not found response
template<class Body, class Allocator>
auto not_found(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    http::response<http::string_body> res{ http::status::not_found, req.version() };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(req.target().data(), req.target().length()) + "' was not found.";
    res.prepare_payload();

    return res;
};

// Returns a server error response
template<class Body, class Allocator>
auto server_error(http::request<Body, http::basic_fields<Allocator>>&& req, beast::string_view what)
{
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();

    return res;
};

// Returns a unauthorized response
template<class Body, class Allocator>
auto unauthorized(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    http::response<http::string_body> res{http::status::unauthorized, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "You are not logged in";
    res.prepare_payload();

    return res;
};

// Returns a forbidden response
template<class Body, class Allocator>
auto forbidden(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    http::response<http::string_body> res{http::status::forbidden, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "You are not allowed to access" + std::string(req.target().data(), req.target().length());
    res.prepare_payload();

    return res;
};

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
    class Body, class Allocator,
    class Send>
void
handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    Send&& send)
{
    std::cout << "--- Request headers" << std::endl << req.base() << std::endl;

    // Make sure we can handle the method
    if (req.method() != http::verb::get &&
        req.method() != http::verb::head &&
        req.method() != http::verb::post &&
        req.method() != http::verb::options) {
            std::cout << "BAD: Unknown HTTP-method" << std::endl;
            return send(bad_request(std::move(req), "Unknown HTTP-method"));
        }

    // Request path must be absolute and not contain "..".
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos) {
            std::cout << "BAD: Illegal request-target" << std::endl;
            return send(bad_request(std::move(req), "Illegal request-target"));
        }

    // Build the path to the requested file
    boost::string_view rel_path = req.target();
    std::string path = path_cat(doc_root, rel_path);

    if (req.target().back() == '/') {
        path.append("index.html");
    }

    beast::error_code ec;
    http::file_body::value_type body;

    // Respond to HEAD request
    if (req.method() == http::verb::head) {

        // Attempt to open the file
        body.open(path.c_str(), beast::file_mode::scan, ec);
        // Handle the case where the file doesn't exist
        if (ec == beast::errc::no_such_file_or_directory) return send(not_found(std::move(req)));
        // Handle an unknown error
        if (ec) return send(server_error(std::move(req), ec.message()));

        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(body.size());
        res.keep_alive(req.keep_alive());

        return send(std::move(res));

    } else if (req.method() == http::verb::options) {

        http::response<http::string_body> res {http::status::created, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_headers, req["Access-Control-Request-Headers"]);
        res.set(http::field::access_control_allow_methods, req["Access-Control-Request-Method"]);
        res.set(http::field::content_type, "text/plain");
        res.body() = "\r\n";
        res.content_length(0);
        res.keep_alive(req.keep_alive());

        return send(std::move(res));

    } else if (req.method() == http::verb::get &&   // Respond to GET file request
               rel_path.substr(0, 4) != "/api") {
        
        // Attempt to open the file
        body.open(path.c_str(), beast::file_mode::scan, ec);
        // Handle the case where the file doesn't exist
        if (ec == beast::errc::no_such_file_or_directory) return send(not_found(std::move(req)));
        // Handle an unknown error
        if (ec) return send(server_error(std::move(req), ec.message()));

        http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())};

        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(body.size());
        res.keep_alive(req.keep_alive());

        return send(std::move(res));

    } else { // POST and GET api requests

        // response body
        std::string resp_body;
        // http response status
        http::status resp_status = CAPIHandler::Instance().HandleApiRequest(std::move(req), resp_body);

        if (resp_status == http::status::ok) {
            // Respond to API request
            http::response<http::string_body> res{ resp_status, req.version() };
            res.set(http::field::server, SERVER_VERSION_STRING);
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::content_type, "application/json");
            res.body() = resp_body.c_str();
            res.content_length(resp_body.size());
            res.keep_alive(req.keep_alive());

            std::cout << ">>POST body:" << rel_path << std::endl << req.body() << std::endl;
            std::cout << "<<" << std::endl;

            return send(std::move(res));

        } else {
            switch (resp_status)
            {
            case http::status::unauthorized:
                return send(unauthorized(std::move(req)));
                break;
            case http::status::forbidden:
                return send(forbidden(std::move(req)));
                break;
            case http::status::not_found:
                return send(not_found(std::move(req)));
                break;
            case http::status::bad_request:
                return send(bad_request(std::move(req), resp_body));
                break;

            default:
                return send(server_error(std::move(req), resp_body));
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << "error: " << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda
    {
        session& self_;

        explicit
        send_lambda(session& self)
            : self_(self)
        {
        }

        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                self_.stream_,
                *sp,
                beast::bind_front_handler(
                    &session::on_write,
                    self_.shared_from_this(),
                    sp->need_eof()));
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;

public:
    // Take ownership of the stream
    session(
        tcp::socket&& socket,
        std::shared_ptr<std::string const> const& doc_root)
        : stream_(std::move(socket))
        , doc_root_(doc_root)
        , lambda_(*this)
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &session::do_read,
                          shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        // Send the response
        handle_request(*doc_root_, std::move(req_), lambda_);
    }

    void
    on_write(
        bool close,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root)
        : ioc_(ioc)
        , acceptor_(net::make_strand(ioc))
        , doc_root_(doc_root)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(
                std::move(socket),
                doc_root_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------
// TODO: add logger
int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
            "Usage: http_server <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    http_server 8080 . 1\n";
        return EXIT_FAILURE;
    }

    auto const port = static_cast<unsigned short>(std::atoi(argv[1]));
    auto const doc_root = std::make_shared<std::string>(argv[2]);
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{tcp::v6(), port},
        doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}
