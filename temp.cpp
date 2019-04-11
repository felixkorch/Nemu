#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << (std::chrono::duration<double, std::ratio<1>>(1.0) / 60).count() << '\n';
    return 0; 
}