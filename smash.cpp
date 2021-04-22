#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    //to stop debugging, remove the i and do while(true)
    int i = 0;
    while(i<200) {
        std::cout << smash.retrivePrompt() << " ";
        std::cout << "\nr - main"; //debugging purpose
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        if (cmd_line.empty()) {
            std::cout << "\ncmd_line is null..."; //debugging purpose
        }
        std::cout << "\nr - getline"; //debugging purpose
        smash.executeCommand(cmd_line.c_str());
        i++;
    }
    return 0;
}