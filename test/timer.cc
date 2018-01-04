#include "base/Timer.h"
#include "base/Timestamp.h"

#include <signal.h>
#include <unistd.h>

#include <iostream>

using std::cout;
using std::endl;

using xchange::base::Timer;
using xchange::base::Timestamp;
using xchange::base::TimerContext;
using xchange::base::TimerEvent;
using xchange::base::TIMER_TIMEOUT;

void callback(TimerEvent, void *arg) {
    std::cout << "Timer triggered at "<< Timestamp::now().toString() << std::endl;
    std::cout << "argument of timer is: " << static_cast<char *>(arg) << endl;
}

int main(void) {
    TimerContext timerContext;

    char timerarg[] = "i'm the argument of timer";

    // trigger after 3 seconds
    Timer timer(Timestamp::now() += 3, timerarg);
    timer.on(TIMER_TIMEOUT, callback);

    cout << "Timer Registered at " << Timestamp::now().toString() << endl;
    timerContext.registerTimer(&timer);

    sleep(5);

    return 0;
}
