#ifndef AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include "protocol/Parser.h"
#include <afina/Storage.h>
#include <afina/execute/Command.h>

namespace Afina {
namespace Network {
namespace MTnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> ps) : _socket(s), _ps(ps) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }
    ~Connection() {
        _command_to_execute.reset();
        _argument_for_command.resize(0);
        _parser.Reset();
        _answers.clear();
    }

    inline bool isAlive() const {
        std::unique_lock<std::mutex> locker(_mutex);
        return _alive;
    }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class Worker;
    friend class ServerImpl;

    static constexpr uint32_t ERR = EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLET;

    int _socket;
    struct epoll_event _event;
    bool _alive;
    bool _need_to_close;

    std::shared_ptr<Afina::Storage> _ps;
    std::size_t _arg_remains;
    Protocol::Parser _parser;
    std::string _argument_for_command;
    std::unique_ptr<Execute::Command> _command_to_execute;

    char client_buffer[4096];
    int _readed_bytes;

    std::deque<std::string> _answers;
    std::size_t _off_set;

    mutable std::mutex _mutex;

    // void CleanOnClose();
};

} // namespace MTnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
