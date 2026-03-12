#pragma once
#include <string>
#include <map>
#include <stdexcept>

namespace osdui::http {

struct HttpResponse {
    int         status_code{0};
    std::string body;
    bool        ok() const { return status_code >= 200 && status_code < 300; }
};

struct HttpError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct IHttpClient {
    virtual ~IHttpClient() = default;
    virtual HttpResponse get (const std::string& url,
                              const std::map<std::string, std::string>& headers = {}) = 0;
    virtual HttpResponse post(const std::string& url,
                              const std::string& body,
                              const std::map<std::string, std::string>& headers = {}) = 0;
};

} // namespace osdui::http
