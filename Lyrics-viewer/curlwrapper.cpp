#include "curlwrapper.hpp"

CurlWrapper::CurlWrapper()
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
        std::cout << "failed curl global initialization\n";

    curl = curl_easy_init();
    if (!curl)
        std::cout << "failed curl easy initialization\n";
}
CurlWrapper::~CurlWrapper()
{
    if (curl)
        curl_easy_cleanup(curl);

    curl_global_cleanup();
}

Response CurlWrapper::sendReq(Request req)
{
    if (!curl)
        return Response("curl wasn't initialized");

    //set the request method
    switch (req.method) {
    default:
        return Response("invalid HTTP method provided");
        break;
    case Request::Methods::POST:
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        break;
    case Request::Methods::GET:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        break;
    case Request::Methods::PUT:
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    }

    curl_easy_setopt(curl, CURLOPT_CAINFO, ".\\cacert.pem");
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

    Response res;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);

    //set request headers
    struct curl_slist* chunk = nullptr;
    for (const std::string& header : req.headers)
        chunk = curl_slist_append(chunk, header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    //perform request
    CURLcode curlRes = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.code);
    if (curlRes != CURLE_OK)
        res.error = "curl error: " + std::string(curl_easy_strerror(curlRes));

    //cleanup
    curl_slist_free_all(chunk);
    curl_easy_reset(curl);

    return res;
}

size_t CurlWrapper::writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json Response::toJson()
{
    return nlohmann::json::parse(body);
}
