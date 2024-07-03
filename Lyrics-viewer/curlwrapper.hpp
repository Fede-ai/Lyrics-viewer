#pragma once
#include "json.hpp"
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

struct Response {	
	Response() = default;
	Response(std::string inError)
		:
		error(inError)
	{
	}

	json toJson();

	bool success = false;
	std::string error = "";
	long code = 0;
	std::string body;
};

struct Request {
	enum class Methods {
		NONE,
		GET,
		POST
	};
	
	Request(Methods inMethod)
		:
		method(inMethod)
	{
	}

	Methods method = Methods::NONE;
	std::string url = "";
	std::vector<std::string> headers = {};
	//will be ignored unless the HTTP method is set to POST
	std::string body = "";
};

class CurlWrapper {
public:
	CurlWrapper();
	~CurlWrapper();

	Response send(Request req);

private:
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

	CURL* curl = NULL;
};
