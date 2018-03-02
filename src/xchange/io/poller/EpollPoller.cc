#include <xchange/io/poller/EpollPoller.h>

using xchange::io::channel::Channel;
using xchange::io::channel::ChannelEvent;
using xchange::io::poller::EpollPoller;
using xchange::io::poller::Poller;

EpollPoller::EpollPoller() {
    epollfd_ = ::epoll_create1(0);
}
EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

int EpollPoller::bind(Channel *channel) {
    struct epoll_event ev;
    int ret;

    ev.events = EPOLLET | EPOLLPRI;

    if (channel->hasEvent(ChannelEvent::IN)) {
        ev.events |= EPOLLIN;
    }
    if (channel->hasEvent(ChannelEvent::OUT)) {
        ev.events |= EPOLLOUT;
    }
    if (channel->hasEvent(ChannelEvent::HUP)) {
        ev.events |= EPOLLHUP;
    }
    if (channel->hasEvent(ChannelEvent::RDHUP)) {
        ev.events |= EPOLLRDHUP;
    }
    if (channel->hasEvent(ChannelEvent::ERROR)) {
        ev.events |= (EPOLLPRI | EPOLLERR);
    }

    ev.data.ptr = channel;

    ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, channel->fd(), &ev);
    if (ret < 0) {
        return ret;
    }

    return ret;
}

int EpollPoller::unbind(Channel *channel) {
    struct epoll_event ev;
    int ret;

    ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->fd(), &ev);

    return ret;
}

int EpollPoller::poll(uint64_t timeoutMs) {
    const uint64_t eventSize = 2048;
    struct epoll_event events[eventSize];
    int evnum = 0;

    evnum = epoll_wait(epollfd_, events, eventSize, timeoutMs);
    if (evnum < 0) {
        return errno;
    }

    for (int i = 0; i < evnum; i++) {
        Channel *channel = static_cast<Channel *>(events[i].data.ptr);

        if (events[i].events & EPOLLERR || events[i].events & EPOLLPRI) {
            channel->setErrorStatus(true);
            channel->setEofStatus(true);

            if (channel->hasEvent(ChannelEvent::ERROR)) {
                channel->emit(ChannelEvent::ERROR, channel);
            }
        }
        if (events[i].events & EPOLLIN) {
            channel->setReadStatus(true);

            if (channel->hasEvent(ChannelEvent::IN)) {
                channel->emit(ChannelEvent::IN, channel);
            }
        }
        if (events[i].events & EPOLLOUT) {
            channel->setWriteStatus(true);

            if (channel->hasEvent(ChannelEvent::OUT)) {
                channel->emit(ChannelEvent::OUT, channel);
            }
        }
        if (events[i].events & EPOLLHUP && channel->hasEvent(ChannelEvent::HUP)) {
            channel->emit(ChannelEvent::HUP, channel);
        }
        if (events[i].events & EPOLLRDHUP && channel->hasEvent(ChannelEvent::RDHUP)) {
            channel->emit(ChannelEvent::RDHUP, channel);
        }
    }

    return 0;
}

int EpollPoller::updateChannelEvent(Channel &channel) {
    struct epoll_event ev;
    int ret = 0;

    ev.events = EPOLLET | EPOLLPRI;

    if (channel.hasEvent(ChannelEvent::IN)) {
        ev.events |= EPOLLIN;
    }
    if (channel.hasEvent(ChannelEvent::OUT)) {
        ev.events |= EPOLLOUT;
    }
    if (channel.hasEvent(ChannelEvent::HUP)) {
        ev.events |= EPOLLHUP;
    }
    if (channel.hasEvent(ChannelEvent::RDHUP)) {
        ev.events |= EPOLLRDHUP;
    }
    if (channel.hasEvent(ChannelEvent::ERROR)) {
        ev.events |= (EPOLLPRI | EPOLLERR);
    }

    ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, channel.fd(), &ev);

    return ret;
}
