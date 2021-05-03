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
    struct sigaction new_action;
    new_action.sa_handler = alarmHandler;
    new_action.sa_flags = SA_RESTART;
    if(sigaction(SIGALRM, &new_action, NULL)<0){
        perror("smash error: sigaction failed");
    }

    SmallShell& smash = SmallShell::getInstance();
    //to stop debugging, remove the i and do while(true)
    while(true) {
        std::cout << smash.retrivePrompt() << " ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        if (!std::cin) {
            return 0;
        }
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}