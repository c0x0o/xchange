#ifndef XCHANGE_IO_CHANNEL_H_
#define XCHANGE_IO_CHANNEL_H_

#include <xchange/design/Noncopyable.h>
#include <xchange/base/EventEmitter.h>

namespace xchange {
namespace io {
namespace channel {
    enum ChannelEvent {IN = 1, OUT = 2, ERROR = 4, HUP = 8, RDHUP = 16};
    enum ChannelType {UDP, TCP, FS, PTCP/*passive tcp*/, TTY};

    // 3 method must be implemented
    class Channel: public xchange::Noncopyable, public xchange::EventEmitter<ChannelEvent> {
        public:
            Channel(int ev, bool rs = false, bool ws = true, bool es = false, bool eof = false)
                : events_(ev),
                readable_(rs),
                writeable_(ws),
                error_(es),
                eof_(eof)
            {}
            virtual ~Channel() {};

            // need implemented
            virtual int fd() const = 0;
            virtual ChannelType type() const = 0;
            virtual void close() = 0;

            virtual bool readable() const {return readable_;}
            virtual bool writeable() const {return writeable_;}
            virtual bool error() const {return error_;}
            virtual bool eof() const {return eof_;}

            virtual int getEvents() const {return events_;}
            virtual bool hasEvent(int event) {return events_ & event;}
            virtual void setEvents(int events) {events_ = events;}
            virtual void addEvents(int events) {events_ |= events;}
            virtual void removeEvents(int events) {events_ &= (~events);}

            virtual void setReadStatus(bool status) {readable_ = status;}
            virtual void setWriteStatus(bool status) {writeable_ = status;}
            virtual void setErrorStatus(bool status) {error_ = status;}
        protected:
            int events_;
            bool readable_;
            bool writeable_;
            bool error_;
            bool eof_;
    };
}
}
}

#endif
