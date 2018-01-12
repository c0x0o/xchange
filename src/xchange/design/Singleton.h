#ifndef XCHANGE_DESIGN_SINGLETON_H_
#define XCHANGE_DESIGN_SINGLETON_H_

#include <xchange/design/Noncopyable.h>

namespace xchange {

    template<typename T>
    struct Singleton : xchange::Noncopyable {
        public:
            static T &instance() {
                static T single;

                obj.doNothing();

                return single;
            }
        private:
            struct ObjGetter {
                ObjGetter() {
                    Singleton<T>::instance();
                }

                inline void doNothing() const {};
            };
            static ObjGetter obj;

            Singleton();
    };

}

template<typename T> typename xchange::Singleton<T>::ObjGetter xchange::Singleton<T>::obj;

#endif
