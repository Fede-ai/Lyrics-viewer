#pragma once
#include "json.hpp"
#include <curl/curl.h>
#include <iostream>

//if (error == "") than the message was sent successfully
struct Response {	
	Response() = default;
	Response(std::string inError)
		:
		error(inError)
	{}

	nlohmann::json toJson();

	//if (error == "") than the message was sent successfully
	std::string error = "";
	long code = 0;
	std::string body;
};

struct Request {
	enum class Methods {
		NONE,
		GET,
		POST,
		PUT
	};
	
	Request(Methods inMethod)
		:
		method(inMethod)
	{}

	Methods method = Methods::NONE;
	std::string url = "";
	std::vector<std::string> headers = {};
	//will be ignored unless the HTTP method is set to POST
	std::string body = "";
};

//singleton class
class CurlWrapper {
public:
	~CurlWrapper();

	CurlWrapper(const CurlWrapper&) = delete;
	CurlWrapper& operator=(const CurlWrapper&) = delete;

	static Response send(const Request& req) {
		static CurlWrapper instance;
		return instance.sendRequest(req);
	}

private:
	CurlWrapper();
	Response sendRequest(const Request& req);

	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
	std::string caInfo = "";
};
