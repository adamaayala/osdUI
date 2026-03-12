#include "http_client.hpp"
#include <curl/curl.h>
#include <stdexcept>

namespace osdui::http {

namespace {
std::size_t write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}
} // anon

struct HttpClient::Impl {
    CURL* curl{nullptr};
    Impl()  { curl = curl_easy_init(); }
    ~Impl() { if (curl) curl_easy_cleanup(curl); }
};

HttpClient::HttpClient() : impl_{std::make_unique<Impl>()} {
    if (!impl_->curl) throw HttpError{"curl_easy_init failed"};
}
HttpClient::~HttpClient() = default;

HttpResponse HttpClient::get(const std::string& url,
                             const std::map<std::string, std::string>& headers)
{
    auto* curl = impl_->curl;
    curl_easy_reset(curl);
    HttpResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &resp.body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_slist* hdrs = nullptr;
    for (const auto& [k, v] : headers)
        hdrs = curl_slist_append(hdrs, (k + ": " + v).c_str());
    if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    CURLcode rc = curl_easy_perform(curl);
    if (hdrs) curl_slist_free_all(hdrs);
    if (rc != CURLE_OK) throw HttpError{curl_easy_strerror(rc)};

    long code{0};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    resp.status_code = static_cast<int>(code);
    return resp;
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                              const std::map<std::string, std::string>& headers)
{
    auto* curl = impl_->curl;
    curl_easy_reset(curl);
    HttpResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,  (long)body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &resp.body);

    curl_slist* hdrs = nullptr;
    for (const auto& [k, v] : headers)
        hdrs = curl_slist_append(hdrs, (k + ": " + v).c_str());
    if (hdrs) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    CURLcode rc = curl_easy_perform(curl);
    if (hdrs) curl_slist_free_all(hdrs);
    if (rc != CURLE_OK) throw HttpError{curl_easy_strerror(rc)};

    long code{0};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    resp.status_code = static_cast<int>(code);
    return resp;
}

} // namespace osdui::http
