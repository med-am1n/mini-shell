#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

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

void free_args(std::vector<char*>& args) {
    for (char* a : args) free(a);
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

        // ---- check for pipe ----
        size_t pipe_pos = line.find('|');

        if (pipe_pos != std::string::npos) {
            std::string left = line.substr(0, pipe_pos);
            std::string right = line.substr(pipe_pos + 1);

            auto left_args = tokenize(left);
            auto right_args = tokenize(right);

            int fd[2];
            if (pipe(fd) == -1) {
                perror("pipe");
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // Child 1: cmd1
                dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                close(fd[1]);

                execvp(left_args[0], left_args.data());
                perror("execvp left failed");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                // Child 2: cmd2
                dup2(fd[0], STDIN_FILENO);

                close(fd[0]);
                close(fd[1]);

                execvp(right_args[0], right_args.data());
                perror("execvp right failed");
                exit(1);
            }

            // Parent
            close(fd[0]);
            close(fd[1]);

            waitpid(pid1, nullptr, 0);
            waitpid(pid2, nullptr, 0);

            free_args(left_args);
            free_args(right_args);
            continue;
        }

        // ---- no pipe: normal execution ----
        auto args = tokenize(line);

        if (std::string(args[0]) == "exit") {
            free_args(args);
            break;
        }

        if (std::string(args[0]) == "cd") {
            const char* path = args[1] ? args[1] : getenv("HOME");
            if (!path || chdir(path) != 0) perror("cd");
            free_args(args);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args.data());
            perror("execvp");
            exit(1);
        } else {
            waitpid(pid, nullptr, 0);
        }

        free_args(args);
    }

    return 0;
}
