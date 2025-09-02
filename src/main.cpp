#include "Application.h"
#include <iostream>

int main() {
    Application app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application\n";
        return -1;
    }
        
    app.run();
    app.shutdown();
    std::cout << "User initiated shutdown - Exiting...\n";
    
    return 0;
}
