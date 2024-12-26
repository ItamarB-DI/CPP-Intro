#include "fanotify_handler.hpp"
#include "file_system_monitor.hpp"

#include <iostream>
#include <thread>
#include <chrono>


void handlerJob(FaNotifyHandler &fa, std::exception_ptr& ex);
void monitorJob(FileSystemMonitor &monitor, std::exception_ptr& ex);

void basicTest();

int main() {

    try {
        basicTest();    
    
    } catch (std::runtime_error& e) {
        std::cerr << "Test Failed: " << e.what() << std::endl;
    }

    return 0;
}

void basicTest() {
    std::filesystem::path pwd = std::filesystem::current_path();
    std::filesystem::path allow_file = pwd / "allow.txt";
    std::filesystem::path deny_file = pwd / "deny.txt";
    
    std::vector<std::filesystem::path> paths_to_listen_to({allow_file, deny_file});
    std::vector<std::filesystem::path> valid_paths({allow_file});
    std::vector<std::filesystem::path> valid_processes;

    FaNotifyHandler fa_handler(paths_to_listen_to);
    FileSystemMonitor monitor(fa_handler, valid_paths, valid_processes);

    std::exception_ptr handler_ex;
    std::exception_ptr monitor_ex;

    try {
        std::thread handler_thread(handlerJob, std::ref(fa_handler), std::ref(handler_ex));
        std::thread monitor_thread(monitorJob, std::ref(monitor), std::ref(monitor_ex));

        if (handler_thread.joinable()) {
            handler_thread.join();
        }

        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }

    } catch (std::system_error& e) {
        throw std::runtime_error(std::string("failed to create thread") + e.what());
    }

    if (handler_ex != nullptr) {
        std::rethrow_exception(handler_ex);
    }

    if (monitor_ex != nullptr) {
        std::rethrow_exception(monitor_ex);
    }

    std::cout << "passed basic test" << std::endl;
}



void handlerJob(FaNotifyHandler &fa, std::exception_ptr& ex) {

    try {
        fa.listenForEvents();
    } catch (...) {
        auto eptr = std::current_exception();
        ex = eptr;
    }
}
void monitorJob(FileSystemMonitor &monitor, std::exception_ptr& ex) {

    try {
        monitor.run();
    } catch (...) {
        auto eptr = std::current_exception();
        ex = eptr;
    }
}