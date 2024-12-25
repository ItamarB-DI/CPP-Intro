#pragma once
#include "fanotify_handler.hpp"

#include <filesystem>
#include <vector>


class FileSystemMonitor
{
public:
    
    // important fa_handler is saved by reference.
    explicit FileSystemMonitor(FaNotifyHandler& fa_handler, std::vector<std::filesystem::path>& valid_files, std::vector<std::filesystem::path>& valid_exe);

    void run(); // blocking
    void stop();

private:
    FaNotifyHandler& m_fa_handler;
    std::vector<std::filesystem::path> m_valid_files;
    std::vector<std::filesystem::path> m_valid_exe;
    bool m_stop;
};
