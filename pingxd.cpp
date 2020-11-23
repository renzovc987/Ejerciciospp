#include <iostream>            
#include <thread>              
#include <mutex>               
#include <chrono>              
#include <condition_variable>  


using namespace std::chrono_literals;

int main() {

    std::mutex mutex;
    std::condition_variable v_condi;
    bool state = true; 

    auto ping = [&] ( ) -> void {
        while (true) {

            std::unique_lock<std::mutex> unique_lock(mutex);
            v_condi.wait(unique_lock,[&](){return state;});

            std::this_thread::sleep_for(1s);
            std::cout << " PING " << std::endl;

            state = !state;
            v_condi.notify_one();
        }
    };

    auto pong = [&] ( ) -> void {
        while (true) {
            std::unique_lock<std::mutex> unique_lock(mutex);
            v_condi.wait(unique_lock,[&](){return !state;});

            std::this_thread::sleep_for(1s);
            std::cout << " PONG " << std::endl;

            state = !state;
            v_condi.notify_one();
        }
    };

    std::thread ping_thread(ping);
    std::thread pong_thread(pong);
    ping_thread.join();
    pong_thread.join();
}