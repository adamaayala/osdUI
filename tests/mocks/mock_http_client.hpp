#pragma once
#include <osdui/ihttp_client.hpp>
#include <queue>
#include <stdexcept>

namespace osdui::test {

class MockHttpClient : public http::IHttpClient {
public:
    void enqueue(http::HttpResponse resp) { responses_.push(std::move(resp)); }

    http::HttpResponse get(const std::string&,
                           const std::map<std::string,std::string>&) override {
        return dequeue();
    }
    http::HttpResponse post(const std::string&, const std::string&,
                            const std::map<std::string,std::string>&) override {
        return dequeue();
    }

private:
    std::queue<http::HttpResponse> responses_;
    http::HttpResponse dequeue() {
        if (responses_.empty())
            throw std::logic_error{"MockHttpClient: no response queued"};
        auto r = responses_.front(); responses_.pop(); return r;
    }
};

} // namespace osdui::test
