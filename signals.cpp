#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  JobsList::JobEntry* job = SmallShell::getInstance().curr_fg_p;
  if (!job) {
    return;
  }
  int pid = job->process_id;
  if (kill(pid, SIGTSTP) < 0 ) {
    perror("smash error: kill failed");
  }
  JobsList::JobEntry* new_job = new JobsList::JobEntry(job->cmd_name, pid, job->time_created, true);
  ((SmallShell::getInstance().jobs)->job_list)->push_back(new_job);
  std::cout<< "smash: process " << pid << " was stopped" <<std::endl;
  std::cout<< "smash: got ctrl-Z" << std::endl;
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
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}
