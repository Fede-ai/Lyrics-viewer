#include "curlwrapper.h"

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

Response CurlWrapper::send(Request req)
{
    if (req.method == Request::Methods::NONE)
        return Response("No HTTP method was provided");
    if (!curl)
        return Response("Curl wasn't initialized");

    Response res;

    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
    if (req.method == Request::Methods::POST)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());

    if (req.method == Request::Methods::POST)
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    else if (req.method == Request::Methods::GET)
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    else 
        return Response("Invalid HTTP method provided");

    struct curl_slist* chunk = nullptr;
    for (const std::string& header : req.headers)
        chunk = curl_slist_append(chunk, header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    CURLcode curlRes = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.code);

    if (curlRes == CURLE_OK)
        res.success = true;
    else
        res.error = curl_easy_strerror(curlRes);

    curl_slist_free_all(chunk);

    return res;
}

size_t CurlWrapper::writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

json Response::toJson()
{
    return json::parse(body);
}
