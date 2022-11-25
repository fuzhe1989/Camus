#include <boost/lockfree/queue.hpp>
#include <iostream>

int main() {
    boost::lockfree::queue<int> q(1);
    q.push(1);
    int ret = 0;
    q.pop(ret);
    if (ret != 1) {
        std::cerr << "wrong " << ret << std::endl;
    }
}
