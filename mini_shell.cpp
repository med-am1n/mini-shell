#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>     // fork, execvp
#include <sys/wait.h>   // waitpid

std::vector<char*> tokenize(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    std::vector<char*> args;

    while (ss >> token) {
        // strdup allocates C-style string
        args.push_back(strdup(token.c_str()));
    }

    args.push_back(nullptr); // execvp requires null-terminated argv
    return args;
}

int main() {
    while (true) {
        std::cout << "mini-shell> ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nExiting shell.\n";
            break;
        }

        if (line.empty()) continue;

        auto args = tokenize(line);

        pid_t pid = fork();

        if (pid == 0) {
            // Child
            execvp(args[0], args.data());
            perror("execvp failed");
            exit(1);
        } 
        else if (pid > 0) {
            // Parent
            waitpid(pid, nullptr, 0);
        } 
        else {
            perror("fork failed");
        }

        // Free allocated memory
        for (char* arg : args) {
            free(arg);
        }
    }

    return 0;
}
