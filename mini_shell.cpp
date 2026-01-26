#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

std::vector<char*> tokenize(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    std::vector<char*> args;

    while (ss >> token) {
        args.push_back(strdup(token.c_str()));
    }
    args.push_back(nullptr);
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

        // ---- Built-in: exit ----
        if (std::string(args[0]) == "exit") {
            break;
        }

        // ---- Built-in: cd ----
        if (std::string(args[0]) == "cd") {
            const char* path = args[1];

            if (!path) {
                path = getenv("HOME");
            }

            if (!path || chdir(path) != 0) {
                perror("cd failed");
            }

            for (char* arg : args) free(arg);
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args.data());
            perror("execvp failed");
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, nullptr, 0);
        } else {
            perror("fork failed");
        }

        for (char* arg : args) free(arg);
    }

    return 0;
}
