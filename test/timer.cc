#include <xchange/base/Timer.h>
#include <xchange/base/Timestamp.h>

#include <signal.h>
#include <unistd.h>

#include <iostream>

using std::cout;
using std::endl;

using xchange::base::Timer;
using xchange::base::Timestamp;
using xchange::base::TimerManager;
using xchange::base::TimerEvent;
using xchange::base::TIMER_TIMEOUT;

// second arg map to the second arg of constructor
void callback(TimerEvent, void *) {
    std::cout << "[Timer]" << Timestamp::now().toString() << std::endl;
}

int main(void) {
    for (int i = 0; i < 10000; i++) {
        Timer *t = new Timer(Timestamp::now()+2, NULL);
        t->on(TIMER_TIMEOUT, callback);

        TimerManager::registerTimer(t);
        TimerManager::collectOutdatedTimer();

        sleep(1);
    }

    return 0;
}
