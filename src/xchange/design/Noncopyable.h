#ifndef XCHANGE_DESIGN_NONCOPYABLE_H_
#define XCHANGE_DESIGN_NONCOPYABLE_H_

namespace xchange {
    class Noncopyable {
        protected:
            Noncopyable() {};
            ~Noncopyable() {};
        private:
            Noncopyable(const Noncopyable &);
            Noncopyable &operator=(const Noncopyable &);
    };
}

#endif
