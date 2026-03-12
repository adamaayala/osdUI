#pragma once
#include <string>
#include <filesystem>

namespace osdui::logging {

enum class Severity { Info, Warning, Error };

// Writes CMTrace-compatible .log entries.
// Thread-safe: each call opens, writes, and closes the file.
class CmLog {
public:
    explicit CmLog(std::filesystem::path path);

    void info   (std::wstring_view component, std::wstring_view message);
    void warning(std::wstring_view component, std::wstring_view message);
    void error  (std::wstring_view component, std::wstring_view message);

private:
    void write(Severity sev, std::wstring_view component, std::wstring_view message);
    std::filesystem::path path_;
};

} // namespace osdui::logging
