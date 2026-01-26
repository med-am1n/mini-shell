#include <iostream>
#include <string>

int main() {
    while (true) {
        std::cout << "mini-shell> ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nExiting shell.\n";
            break;
        }
        
        std::cout << "You typed: " << line << std::endl;
    }

    return 0;
}
