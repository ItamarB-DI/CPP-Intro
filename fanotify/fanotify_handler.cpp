#include "fanotify_handler.hpp"

#include <fcntl.h>
#include <unistd.h>

FaNotifyHandler::FaNotifyHandler(std::vector<std::filesystem::path>& files)
: m_files(files),
  m_lock_replies(),
  m_lock_events(),
  m_events_cv(),
  m_stop(false) {

    m_fanotify = fanotify_init(FAN_NONBLOCK | FAN_CLOEXEC | FAN_CLASS_CONTENT, O_RDONLY);
    if (m_fanotify == -1) {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), "fanotify_init failed");
    }

    for (auto file_path : files) {
        if (fanotify_mark(m_fanotify, FAN_MARK_ADD, FAN_OPEN_PERM, AT_FDCWD, file_path.c_str()) == -1) {
            throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), std::string("fanotify_mark failed to mark")  + file_path.c_str());
        }
    }
}

FaNotifyHandler::~FaNotifyHandler() {
    close(m_fanotify);
}

void FaNotifyHandler::listenForEvents() {

    const size_t BUFFER_SIZE = 4096;
    std::vector<char> buffer(BUFFER_SIZE);

    int intr_fails = 0; 
    while (!m_stop) {

        int bytes_read = read(m_fanotify, buffer.data(), BUFFER_SIZE);
        if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            
            replyToFa();
        } else if (bytes_read == -1) {

            if ((errno == EINTR) && intr_fails < 5) {
                ++intr_fails;
                continue;
            } else {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), std::string("read syscall failed"));
            }
        }

        struct fanotify_event_metadata* metadata = reinterpret_cast<struct fanotify_event_metadata*>(buffer.data());
        
        while (FAN_EVENT_OK(metadata, bytes_read)) {
            
            handleEvent(metadata);

            metadata = FAN_EVENT_NEXT(metadata, bytes_read);
        }
    }
}

void FaNotifyHandler::stopListening() {
    m_stop = true;
}

FaNotifyHandler::EventItem FaNotifyHandler::getTopEvent() {
    
    std::unique_lock<std::mutex> lock(m_lock_events);

    m_events_cv.wait(lock, [this](){ return !m_events.empty(); });

    auto curr = std::move(m_events.front());
    m_events.pop();

    return curr;
}

void FaNotifyHandler::addNewReply(struct fanotify_response new_response) {

    std::lock_guard<std::mutex> lock(m_lock_replies);
    m_replies.push(new_response);
}

void FaNotifyHandler::handleEvent(struct fanotify_event_metadata *event_meta_data) {

    if (event_meta_data->vers != FANOTIFY_METADATA_VERSION) {
        throw std::runtime_error("compile version of fanotify doesn't equal to run-time version");
    }

    if (event_meta_data->fd == FAN_NOFD) {
        throw std::runtime_error("queue overflow occurred");
    }

    size_t BUFFER_MAX_SIZE = 1024;
    std::vector<char> path(BUFFER_MAX_SIZE, 0);
    
    std::ostringstream fds_system_data;
    fds_system_data << "/proc/self/fd/" << event_meta_data->fd;

    ssize_t len = readlink(fds_system_data.str().c_str(), path.data(), path.size());
    if (len == -1) {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), "readlink syscall failed");
    }

    std::vector<char> process(BUFFER_MAX_SIZE, 0);
    std::ostringstream process_system_data;
  
    process_system_data << "/proc/" << event_meta_data->pid << "/exe";

    len = readlink(process_system_data.str().c_str(), process.data(), process.size());
    if (len == -1) {
        throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), "readlink syscall failed");
    }

    std::lock_guard<std::mutex> lock(m_lock_events);
    m_events.push({path, event_meta_data->fd, process, event_meta_data->pid});
    m_events_cv.notify_all();
}

void FaNotifyHandler::replyToFa() {

    std::lock_guard<std::mutex> lock(m_lock_replies);
    if (m_replies.empty()) { return; }

    auto reply = m_replies.front();
    m_replies.pop();

    lock.~lock_guard();

    int times_tried = 0;
    while (times_tried < 3) {

        ssize_t len = write(m_fanotify, &reply, sizeof(reply));
        if (len == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                ++times_tried;
                continue;
            } else {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)), "write syscall failed");
            }
        } else if (len != sizeof(reply)) {
            throw std::runtime_error("write written less/more bytes than what excpected");
        } else {
            break;
        }
    }
}

