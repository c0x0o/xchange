#include "algorithm/RedBlackTree.h"

#include <stdlib.h>

#include <iostream>
#include <vector>
#include <stdexcept>

using xchange::algorithm::RedBlackTree;
using std::cout;
using std::endl;

int main() {
    RedBlackTree<long int, int> *mapp = new RedBlackTree<long int, int>(), & map = *mapp;

    map.insert(9, 9);
    map.insert(5, 5);
    map.insert(6, 6);
    map.insert(1, 1);
    map.insert(3, 3);
    map.insert(4, 4);

    // the second argument is the key
    map.each([](int val, long int)->void{cout << val << " ";});

    cout << endl;

    cout << "find key 5:" << map.find(5) << endl;
    cout << "find key 9:" << map.find(9) << endl;
    try {
        cout << "find key 10:" << map.find(10) << endl;
    } catch (std::out_of_range e) {
        cout << "not found" << endl;
    }

    cout << "length: " << map.size() << endl;

    cout << "find key 1:" << map.find(1) << endl;
    map.remove(1);
    try {
        cout << "find key 1:" << map.find(1) << endl;
    } catch (std::out_of_range e) {
        cout << "not found" << endl;
    }

    map.remove(3);
    map.remove(4);
    map.remove(9);
    map.remove(5);
    map.remove(6);

    cout << "length: " << map.size() << endl;

    map.insert(9, 9);
    map.insert(5, 5);
    map.insert(6, 6);
    map.insert(1, 1);
    map.insert(3, 3);
    map.insert(4, 4);

    delete mapp;

    return 0;
}
