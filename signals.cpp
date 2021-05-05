#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  std::cout<< "smash: got ctrl-Z" << std::endl;
  JobsList::JobEntry* job = SmallShell::getInstance().curr_fg_p;
  if (!job) {
    return;
  }
  int pid = job->process_id;
  if (kill(pid, SIGTSTP) < 0 ) {
    perror("smash error: kill failed");
  }
  JobsList::JobEntry* new_job = new JobsList::JobEntry(job->cmd_name, pid, job->time_created, true, job->pos);
  (SmallShell::getInstance().jobs)->addJobByPos(job->pos, new_job);
  std::cout<< "smash: process " << pid << " was stopped" <<std::endl;
  SmallShell::getInstance().curr_fg_p = nullptr;
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  std::cout<< "smash: got ctrl-C" << std::endl;
  JobsList::JobEntry* job = SmallShell::getInstance().curr_fg_p;
  if (!job) {
    return;
  }
  int pid = job->process_id;
  if (kill(pid, SIGINT) != 0) {
    perror("smash error: kill failed");
  }
  std::cout<< "smash: process " << pid << " was killed" <<std::endl;
  SmallShell::getInstance().curr_fg_p = nullptr;
}

void alarmHandler(int sig_num) {
  TimeoutsList::Timeout* c_timeout = ((SmallShell::getInstance()).getTimeoutsList())->findTimeoutByTime(time(nullptr));
  if(c_timeout == nullptr){
    return;
  }
  std::cout << "smash: got an alarm" << std::endl;
  std::cout << "smash: timeout " << c_timeout->duration << c_timeout->cmd_name << " timed out!" << std::endl;
  if(c_timeout->process_id != getpid()) {
    if(kill(c_timeout->process_id , SIGKILL) < 0){
      perror("smash error: kill failed");
    }
  }
  ((SmallShell::getInstance()).getTimeoutsList())->removeTimeoutByPid(c_timeout->process_id);
  if (((SmallShell::getInstance()).getTimeoutsList())->getSize() != 0) {
    unsigned int new_timeout = ((SmallShell::getInstance()).getTimeoutsList())->getClosestTimeout(nullptr);
    alarm(new_timeout);
  }
}