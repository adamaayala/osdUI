#pragma once
#include <osdui/ihttp_client.hpp>
#include <memory>

namespace osdui::http {

class HttpClient : public IHttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse get (const std::string& url,
                      const std::map<std::string, std::string>& headers = {}) override;
    HttpResponse post(const std::string& url,
                      const std::string& body,
                      const std::map<std::string, std::string>& headers = {}) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace osdui::http
