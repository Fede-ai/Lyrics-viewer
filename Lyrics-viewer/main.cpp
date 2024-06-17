#include "overlay.h"
#include "curlwrapper.h"
#include <windows.h>
#include <random>
#include <fstream>
#include <set>
#include "boost/asio.hpp"

#define ROOT "C:/Users/feder/Desktop/Lyrics-viewer/Redirect-website"

void runHttpServer() 
{
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3000));

    std::set<std::string> allowedFiles = { "/index.html", "/favicon.ico" };

    std::string url = "http://localhost:3000";
    ShellExecuteA(0, 0, url.c_str(), 0, 0, SW_SHOW);

    while (true) {
        //accept client and receive the request
        boost::asio::ip::tcp::socket socket(ioService);
        acceptor.accept(socket);
        boost::system::error_code error;
        boost::asio::streambuf request;
        read_until(socket, request, "\r\n", error);

        auto clientIp = socket.remote_endpoint().address();
        auto serverIp = socket.local_endpoint().address();
        //responce = 403 Forbidden
        if (clientIp != serverIp) {
            std::cout << "request didn't come from an authorized endpoint\n";

            std::string response = "HTTP/1.1 403 Forbidden\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: 13\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += "403 Forbidden";

            boost::asio::write(socket, boost::asio::buffer(response), error);
            continue;
        }

        //response = 500 Internal Server Error
        if (error) {
            std::cout << error.message() << "\n";

            std::string response = "HTTP/1.1 500 Internal Server Error\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: 25\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += "500 Internal Server Error";

            boost::asio::write(socket, boost::asio::buffer(response), error);
            continue;
        }

        //divide the request into its components
        std::istream request_stream(&request);
        std::string method, uri, http_version;
        request_stream >> method >> uri >> http_version;

        //response = 405 Method Not Allowed
        if (method != "GET") {
            std::cout << "invalid method used\n";

            std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: 22\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += "405 Method Not Allowed";

            boost::asio::write(socket, boost::asio::buffer(response), error);
            continue;
        }

        //divide path from query parameters
        std::string query, rawPath = uri;
        size_t pos = uri.find('?');
        if (pos != std::string::npos)
            rawPath = uri.substr(0, pos), query = uri.substr(pos + 1);

        //set the base directory
        if (rawPath == "/")
            rawPath = "/index.html";

        //response = 404 Not Found
        if (!allowedFiles.contains(rawPath)) {
            std::cout << "resource asked isn't available\n";

            std::string response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: 13\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += "404 Not Found";

            boost::asio::write(socket, boost::asio::buffer(response), error);
            continue;
        }

        //determinate the mime type of the requested file
        std::string extension = rawPath.substr(rawPath.find_last_of('.'));
        std::string mimeType = "";
        if (extension == ".html")
            mimeType = "text/html";
        else if (extension == ".css")
            mimeType = "text/css";
        else if (extension == ".ico")
            mimeType = "image/x-icon";

        std::ifstream file(ROOT + rawPath, std::ios::binary);
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();
        file.close();

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + mimeType + "\r\n";
        response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += content;
        boost::asio::write(socket, boost::asio::buffer(response), error);

        //std::map<std::string, std::string> queryParams;
        //std::istringstream queryStream(query);
        //std::string key_value;
        //while (std::getline(queryStream, key_value, '&')) {
        //    pos = key_value.find('=');
        //    if (pos != std::string::npos) {
        //        std::string key = key_value.substr(0, pos);
        //        std::string value = key_value.substr(pos + 1);
        //        queryParams[key] = value;
        //    }
        //    else {
        //        queryParams[key_value] = "";
        //    }
        //}
    }
}

int main()
{
    std::string clientId = "244ba241897d4c969d1260ad0c844f91";
    std::string clientSecret = "a3a3471cde7d440cb3f0a6c6f75f6a2e";

    runHttpServer();
    
    //std::string url = "https://accounts.spotify.com/authorize?client_id=" + clientId + "&response_type=code&redirect_uri=http://localhost:3000";
    //ShellExecuteA(0, 0, url.c_str(), 0, 0, SW_SHOW);
    //
	//Overlay ol;
	//return ol.run();
}