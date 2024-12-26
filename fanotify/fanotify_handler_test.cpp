#include "fanotify_handler.hpp"
#include "file_system_monitor.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



void handlerJob(FaNotifyHandler &fa, std::exception_ptr& ex);
void monitorJob(FileSystemMonitor &monitor, std::exception_ptr& ex);

void basicTest();
void Test1(std::exception_ptr& ex);

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
    std::filesystem::path tracked_file = pwd / "tracked.txt";
    std::filesystem::path monitored_file = pwd / "monitored.txt";

    std::filesystem::path code = "/snap/code/177/usr/share/code/code";
    std::filesystem::path gedit = "/usr/bin/gedit";
    
    std::vector<std::filesystem::path> paths_to_listen_to({tracked_file, monitored_file});
    std::vector<std::filesystem::path> tracked_paths({tracked_file});
    std::vector<std::filesystem::path> valid_processes({code, gedit});

    FaNotifyHandler fa_handler(paths_to_listen_to);
    FileSystemMonitor monitor(fa_handler, tracked_paths, valid_processes);

    std::exception_ptr handler_ex;
    std::exception_ptr monitor_ex;
    std::exception_ptr test_ex;

    try {
        std::thread handler_thread(handlerJob, std::ref(fa_handler), std::ref(handler_ex));
        std::thread monitor_thread(monitorJob, std::ref(monitor), std::ref(monitor_ex));
        std::thread test_thread(Test1, std::ref(test_ex));

        if (handler_thread.joinable()) {
            handler_thread.join();
        }

        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }

        if (test_thread.joinable()) {
            test_thread.join();
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

void Test1(std::exception_ptr& ex) {
    (void)ex;
    // std::this_thread::sleep_for(std::chrono::seconds(3));

    // try {
    //     system("nano /home/itamarbloch-lap/Desktop/CppIntro/fanotify/tracked.txt");
    //     //system("nano ~/Desktop/CppIntro/fanotify/tracked.txt");
    // } catch(std::system_error& e) {
    //     std::cerr << "failed opening errno: " << errno << "what: " << e.what() << std::endl;
    // } catch(...) {
    //     std::cerr << "failed defaulkt opening errno: " << errno << std::endl;
    // }
}

