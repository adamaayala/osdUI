#include "cm_log.hpp"
#include <windows.h>
#include <fstream>
#include <chrono>
#include <format>
#include <mutex>

namespace osdui::logging {

namespace {
std::mutex g_log_mutex;

std::string to_utf8(std::wstring_view ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(),
                                  nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(),
                        s.data(), len, nullptr, nullptr);
    return s;
}

int severity_to_int(Severity s) {
    switch (s) {
        case Severity::Info:    return 1;
        case Severity::Warning: return 2;
        case Severity::Error:   return 3;
    }
    return 1;
}
} // anon namespace

CmLog::CmLog(std::filesystem::path path) : path_{std::move(path)} {}

void CmLog::info   (std::wstring_view c, std::wstring_view m) { write(Severity::Info,    c, m); }
void CmLog::warning(std::wstring_view c, std::wstring_view m) { write(Severity::Warning, c, m); }
void CmLog::error  (std::wstring_view c, std::wstring_view m) { write(Severity::Error,   c, m); }

void CmLog::write(Severity sev, std::wstring_view component, std::wstring_view message) {
    using namespace std::chrono;
    auto now  = system_clock::now();
    auto tt   = system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_s(&tm, &tt);

    auto ms   = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // CMTrace log format:
    // <![LOG[message]LOG]!><time="HH:MM:SS.mmm+000" date="M-D-YYYY" component="comp" type=N thread="0" file="">
    std::string line = std::format(
        "<![LOG[{}]LOG]!><time=\"{:02d}:{:02d}:{:02d}.{:03d}+000\" "
        "date=\"{}-{}-{}\" component=\"{}\" type={} thread=\"0\" file=\"\">\n",
        to_utf8(message),
        tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms.count(),
        tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900,
        to_utf8(component),
        severity_to_int(sev)
    );

    std::lock_guard lock{g_log_mutex};
    std::ofstream f{path_, std::ios::app};
    f << line;
}

} // namespace osdui::logging
