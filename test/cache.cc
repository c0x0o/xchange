#include <xchange/io/Cache.h>

#include <iostream>

#include <xchange/io/Buffer.h>

using xchange::io::Buffer;
using xchange::io::Cache;
using std::cout;
using std::endl;

int main(void) {
    // create a 10 bytes long cache
    Cache cache(10);

    cache.write("1234567890");

    // write will failed, return non-zero
    if (cache.write("123") != 0) {
        cout << "write failed" << endl;
    };

    cout << cache.read(5) << endl;

    // 'buff' will be destroy atomatically after the context is end
    Buffer buff = cache.read(5);

    cout << buff << endl;

    cache.write("123");

    // will return a Buffer with all data remained in cache
    cout << cache.read(4) << endl;

    return 0;
}
