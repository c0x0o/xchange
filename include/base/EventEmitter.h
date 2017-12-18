#ifndef _BASE_EVENTEMITTER_H_
#define _BASE_EVENTEMITTER_H_

#include <map>
#include <iostream>

namespace xchange {

    template<typename T>
    class EventEmitter {
        public:
            typedef void (*EventHandler)(T, void *);

            void on(T e, EventHandler callback) {
                if (callback != NULL) {
                    events_.insert(typename std::map<T, EventHandler>::value_type(e, callback));
                }
            };

            void emit(T e, void * arg = NULL) {
                typename std::map<T, EventHandler>::iterator iter = events_.find(e);

                if (iter != events_.end()) {
                    iter->second(e, arg);
                }
            };
        protected:
            std::map<T, EventHandler> events_;
    };
}

#endif
