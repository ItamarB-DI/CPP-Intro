#pragma once
#include <filesystem>
#include <vector>
#include <queue>
#include <mutex>
#include <sys/fanotify.h>
#include <condition_variable>


class FaNotifyHandler {
public:
    struct EventItem
    {
        std::filesystem::path m_path; // the path that the event occured on.
        int m_fd; // the fd that the event occured on.
        std::filesystem::path m_process; // the process name that made that event 
        int m_pid; // the process that made that event

        inline bool operator==(const FaNotifyHandler::EventItem& other);

    };
    const static EventItem EMPTY_EVENT;
    
    explicit FaNotifyHandler(const std::vector<std::filesystem::path>& files);
    FaNotifyHandler(const FaNotifyHandler& other) = delete; 
    FaNotifyHandler &operator=(const FaNotifyHandler& other) = delete; 
    ~FaNotifyHandler();

    void listenForEvents(); // this class run function (blocking)
    void stopListening();
    EventItem getTopEvent(); // returns the first event (blocking) upon stop returns EMPTY_EVENT
    void addNewReply(struct fanotify_response new_response); // abling to append a repliy for the FA

private:
    void handleEvent(struct fanotify_event_metadata *event_meta_data);
    void replyToFa();

    std::vector<std::filesystem::path> m_files;
    int m_fanotify;
    std::queue<EventItem> m_events;
    std::queue<struct fanotify_response> m_replies;
    std::mutex m_lock_replies;
    std::mutex m_lock_events;
    std::condition_variable m_events_cv;
    bool m_stop;
};


inline bool FaNotifyHandler::EventItem::operator==(const FaNotifyHandler::EventItem& other) {
    return m_path == other.m_path &&
           m_fd == other.m_fd &&
           m_process == other.m_process &&
           m_pid == other.m_pid;
}


