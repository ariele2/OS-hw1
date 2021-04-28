#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <ctime>
#include "Commands.h"
#include <algorithm>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
      args[i] = (char*)malloc(s.length()+1);
      memset(args[i], 0, s.length()+1);
      strcpy(args[i], s.c_str());
      args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
      return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
      return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

std::string _removeRedirectionSign(std::string cmd_s) {
    size_t pos = 0;
    while ((pos = cmd_s.find(">", pos)) != std::string::npos) {
         cmd_s.replace(pos, 1, " ");
         pos += 1;
    }
    return cmd_s;
}

bool _isOnlyNumbers(char* line) {
  for (unsigned int i=0; i<strlen(line); i++) {
    if (!isdigit(line[i])) {
      return false;
    }
  }
  return true;
}

bool _isRedirection(const char* cmd_line) {
  if (!cmd_line) {
    return false;
  }
  std::string cmd_s = string(cmd_line);
  if (cmd_s.find(">") != std::string::npos || cmd_s.find(">>") != std::string::npos) {
    return true;
  }
  return false;
}

void prepareRedirection(char** args, bool is_append, int args_num) {
  std::string w_spath;
  if (!args[1]) {// the > or >> sign shows up without backspaces
  std::string cmd_s = string(args[0]);
    if (cmd_s.find(">>") != string::npos) {
      w_spath = cmd_s.substr(cmd_s.find_first_of(">")+2);
    }
    else {
      w_spath = cmd_s.substr(cmd_s.find_first_of(">")+1);
    }
  }
  else if (!args[2]) { //the > or >> sign shows up without one backspace
    std::string cmd_a1 = string(args[0]);
    std::string cmd_a2 = string(args[1]);
    if (cmd_a1.find(">>") != string::npos || cmd_a1.find(">") != string::npos) { //backspace after >
      w_spath = cmd_a2;
    }
    else {
      if (cmd_a2.find(">>") != string::npos) { //backspace after first word
        w_spath = cmd_a2.substr(cmd_a2.find_first_of(">")+2);
      }
      else {
        w_spath = cmd_a2.substr(cmd_a2.find_first_of(">")+1);
      }
    }
  }
  else { //3 arguments 
    w_spath = string(args[args_num-1]);
    w_spath = _removeRedirectionSign(w_spath);
    w_spath = _trim(w_spath);
  }
  const char* w_path = w_spath.c_str();
  if (!is_append) {
    open(w_path, O_WRONLY|O_CREAT, S_IWUSR);
  }
  else {
    open(w_path, O_WRONLY|O_CREAT|O_APPEND, S_IWUSR);
  }
}

// TODO: Add your implementation for classes in Commands.h 

void ChPromptCommand::execute() { 
    if (args[1]) {
        string new_p = string(args[1]);
        new_p.push_back('>');
        changePromptMessage(new_p);
    }
    else {
        changePromptMessage("smash>");
    }
}

void ShowPidCommand::execute(){
    cout << "smash pid is "<< getpid() << std::endl;
}

void GetCurrDirCommand::execute() {
    char cwd[COMMAND_ARGS_MAX_LENGTH];
    getcwd(cwd, sizeof(cwd));
    std::cout << cwd << std::endl;
}

void ChangeDirCommand::execute() {
    int success;
    char cwd[COMMAND_ARGS_MAX_LENGTH];
    getcwd(cwd, sizeof(cwd));
    char* o_cwd = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(o_cwd, cwd);
    if (args[1]) { //if we got more then just cd
      if (args[2]) {
          std::cout << "smash error: cd: too many arguments\n";
      }
      else if (string(args[1]).compare(string("-")) == 0) { //if we want to go back to the prevoius path
          const char* lpwd = *(this->plastPwd);
          if (!lpwd) { //no last pwd
              std::cout << "smash error: cd: OLDPWD not set\n";
          }
          else {
              chdir(lpwd);
              delete *(this->plastPwd);
              *(this->plastPwd) = new char[COMMAND_ARGS_MAX_LENGTH];
              *(this->plastPwd) = o_cwd;
          }
      }
      else {
        success = chdir(args[1]);
        if (success == 0) {
          delete *(this->plastPwd);
          *(this->plastPwd) = new char[COMMAND_ARGS_MAX_LENGTH];
          *(this->plastPwd) = o_cwd;
        } 
        else {
          perror("smash error: cd failed");
        }
      }
    }
}

//JobsList class methods implementation
void JobsList::addJob(Command* cmd, int child_pid, bool isStopped) {
  JobsList::JobEntry* new_job = new JobEntry();
  time_t curr_time;
  time(&curr_time);
  int i = 0;
  std::string job_s;
  while (i < cmd->args_num-1) {
    job_s = job_s + string((cmd->args)[i]) + " ";
    i++;
  }
  job_s = job_s + string((cmd->args)[i]);
  new_job->cmd_name = job_s;
  new_job->process_id = child_pid; //the addjob process should be called from child process and wait
  new_job->time_created = curr_time;
  new_job->stopped = isStopped;
  (this->job_list)->push_back(new_job);
}

void JobsList::addJob(JobEntry* job) {
  (this->job_list)->push_back(job);
}

void JobsList::printJobsList() {
  int i = 1;
  for (auto it = job_list->begin(); it != job_list->end(); it++) {
    time_t curr_time;
    time(&curr_time);
    double time_to_print = difftime(curr_time,(*it)->time_created);
    std::cout<<"["<<i<<"] "<<(*it)->cmd_name<<" : "<<(*it)->process_id<<" "<<time_to_print <<" secs\n";
    if ((*it)->stopped) {
      std::cout<<" "<<"(stopped)\n";
    }
    i++;
  }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
  for (auto it = job_list->begin(); it != job_list->end(); it++) {
    if ((*it)->process_id == jobId) {
      return *it;
    } 
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId) {
  JobEntry* job_to_remove = getJobById(jobId);
  if (!job_to_remove) {
    return;
  }
  job_list->remove(job_to_remove);
  delete job_to_remove;
}

JobsList::JobEntry* JobsList::getJobByPos(int job_pos) {
  int i = 1;
  for (auto it = job_list->begin(); it != job_list->end(); it++) {
    if (i == job_pos) { 
      return (*it);
    }
    i++;
  }
  return nullptr;
}

JobsList::JobEntry* JobsList::getLastJob(int* lastJobId) {
  if (job_list->empty()) {
    return nullptr;
  }
  JobsList::JobEntry* last_job = job_list->back();
  *lastJobId = last_job->process_id;
  return last_job;
}

int JobsList::getJobsListSize() {
  return job_list->size();
}

void ForegroundCommand::execute() {
  int* job_id = new int();
  int status;
  JobsList::JobEntry* fg_job;
  bool only_digits = true;
  if ((this->args)[1]) {
    only_digits = _isOnlyNumbers((this->args)[1]);
  }
  if ((this->args)[2] || !only_digits) {
    std::cout << "smash error: fg: invalid arguments" << std::endl;
  }
  else if (jobs->getJobsListSize() == 0) {
    std::cout << "smash error: fg: jobs list is empty" << std::endl;
  }
  else if (jobs->getJobsListSize() < stoi(string((this->args)[1]))) {
    std::cout << "smash error: fg: job-id " << stoi(string((this->args)[1])) << " does not exist" << std::endl;
  }
  else {
    if ((this->args)[1]) {
      fg_job = jobs->getJobByPos(stoi(string((this->args)[1])));
      *job_id = fg_job->process_id;
    }
    else {
      fg_job = jobs->getLastJob(job_id);
    }
    std::cout << fg_job->cmd_name << " : " << *job_id << std::endl;
    if (kill(*job_id, SIGCONT) != 0) {
      perror("smash error: kill failed");
    }
    waitpid(*job_id, &status, 0);
    jobs->removeJobById(*job_id);
  }
  delete job_id;
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
  if (job_list->empty()) {
    return nullptr;
  }
  int i = getJobsListSize();
  for (auto it = job_list->rbegin(); it != job_list->rend(); it++) {
    if ((*it)->stopped) {
      *jobId = (*it)->process_id; 
      return (*it);
    }
    i--;
  }
  return nullptr;
}

void BackgroundCommand::execute() {
  int* job_id = new int();
  JobsList::JobEntry* bg_job;
  bool only_digits = true;
  if ((this->args)[1]) {
    only_digits = _isOnlyNumbers((this->args)[1]);
  }
  if ((this->args)[2] || !only_digits) {
    std::cout << "smash error: bg: invalid arguments" << std::endl;
  }
  else if (jobs->getJobsListSize() == 0 || !(jobs->getLastStoppedJob(job_id))) {
    std::cout << "smash error: bg: there is no stopped jobs to resume" << std::endl;
  }
  else if (jobs->getJobsListSize() < stoi(string((this->args)[1]))) {
    std::cout << "smash error: bg: job-id " << stoi(string((this->args)[1])) << " does not exist" << std::endl;
  }
  else {
    if ((this->args)[1]) {
      bg_job = jobs->getJobByPos(stoi(string((this->args)[1])));
      *job_id = bg_job->process_id;
      if (bg_job->stopped == false) {
        std::cout << "smash error: bg: job-id "<< *job_id << " is already running in the background" << std::endl;
        return;
      }
    }
    else {
      bg_job = jobs->getLastStoppedJob(job_id);
    }
    if (kill(*job_id, SIGCONT) != 0) {
      perror("smash error: kill failed");
    }
    else {
      bg_job->stopped = false;
    }
  }
}

void JobsList::killAllJobs() {
  std::cout << "sending SIGKILL signal to " << getJobsListSize() << " jobs:" <<std::endl;
  for (auto it = job_list->begin(); it != job_list->end(); it++) {
    int p_id = (*it)->process_id;
    std::cout << p_id << ": " << (*it)->cmd_name <<std::endl;
    if (kill(p_id, SIGKILL) != 0) {
      perror("smash error: kill failed");
    }
  }
}

void QuitCommand::execute() {
  if (!(this->args)[1]) {
    exit(1);
  }
  else if (string((this->args)[1]).compare(string("kill")) == 0) {
    jobs->killAllJobs();
  }
  exit(1);
}

void KillCommand::execute() {
  std::string s_arg1 = string((this->args)[1]);
  std::string s_arg2 = string((this->args)[2]);
  if (s_arg1.find_first_of("-") != 0) {
    std::cerr << "smash error: kill: invalid arguments" << std::endl;
  }
  int signal_num = stoi(s_arg1.substr(1));
  int job_pos = stoi(s_arg2);
  JobsList::JobEntry* job_to_kill = jobs->getJobByPos(job_pos);
  if (!job_to_kill) {
    std::cerr << "smash error: kill: job-id "<<job_pos<<" does not exist\n";
  }
  else if (kill(job_to_kill->process_id, signal_num) == 0) {
    std::cout << "signal number "<< signal_num <<" was sent to pid " << job_to_kill->process_id <<std::endl;
    if (signal_num == SIGTSTP || signal_num == SIGSTOP) {
      job_to_kill->stopped = true;
    }
    else if (signal_num == SIGCONT) {
      job_to_kill->stopped = false;
    }
  }
  else {
    perror("smash error: kill failed");
  }
  jobs->removeFinishedJobs();
}

void JobsList::removeFinishedJobs() {
  int status;
  for (auto it = job_list->begin(); it != job_list->end(); it++) {
    int p_id = (*it)->process_id;
    int res = waitpid(p_id, &status, WNOHANG);
    if (res == -1) {
      perror("smash error: waitpid failed");
    }
    else if (res > 0) {
      auto temp_it = std::next(it,1);
      this->removeJobById(p_id);
      it = temp_it;
    }
  }
}

//before the print, dont forget to check if there are processes to remove.
void JobsCommand::execute() {
  jobs->removeFinishedJobs();
  jobs->printJobsList();
}

void ExternalCommand::execute(){
  int i = 0;
  std::string cmd_s;
  char cmd_c[COMMAND_ARGS_MAX_LENGTH];
  while (i < this->args_num && string((this->args)[i]).compare(string(">")) != 0 && string((this->args)[i]).compare(string(">>")) != 0) {
    cmd_s = cmd_s + string((this->args)[i]) + " ";
    i++;
  }
  const char* tmp_cmd = cmd_s.c_str();
  strcpy(cmd_c, tmp_cmd);
  _removeBackgroundSign(cmd_c);
  char* argsv[] = {"/bin/bash", "-c", cmd_c, NULL};
  if ((execv(argsv[0], argsv)) == -1) {
      perror("smash error: execv failed");
  }
}

SmallShell::SmallShell(): new_p("smash>"), plastPwd(nullptr) {
// TODO: add your implementation
  std::list<JobsList::JobEntry*>* job_list = new std::list<JobsList::JobEntry*>();
  this->jobs = new JobsList(job_list);
}

SmallShell::~SmallShell() {
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (firstWord.compare("chprompt") == 0) {
      return new ChPromptCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0) {
      return new ChangeDirCommand(cmd_line, this->plastPwd);
    }
    else if (firstWord.compare("kill") == 0) {
      return new KillCommand(cmd_line, this->jobs);
    }
    else if (firstWord.compare("jobs") == 0) {
      return new JobsCommand(cmd_line, this->jobs);
    }
    else if (firstWord.compare("fg") == 0) {
      return new ForegroundCommand(cmd_line, this->jobs);
    }
    else if (firstWord.compare("bg") == 0) {
      return new BackgroundCommand(cmd_line, this->jobs);
    }
    else if (firstWord.compare("quit") == 0) {
      return new QuitCommand(cmd_line, this->jobs);
    }
    else {
      return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char* cmd_line) {
    // TODO: Add your implementation here
    // for example:
    int status;
    string cmd_s = _trim(string(cmd_line));
    cmd_s = _removeRedirectionSign(cmd_s); //a procedure needed if its a redirection
    const char* prepared_cmd = cmd_s.c_str();
    cmd_s = _trim(string(cmd_line));
    bool bg_cmd = _isBackgroundComamnd(cmd_line);
    Command* cmd = CreateCommand(prepared_cmd);
    cmd->args = new char*[COMMAND_ARGS_MAX_LENGTH]();
    if (_isRedirection(cmd_line)) {
      bool is_append = false;
      char rdir_cmd[COMMAND_ARGS_MAX_LENGTH];
      strcpy(rdir_cmd, cmd_line);
      _removeBackgroundSign(rdir_cmd);
      cmd->args_num = _parseCommandLine(rdir_cmd, cmd->args);
      if (cmd_s.find(">>") != string::npos) { 
        is_append = true;
      }
      dup2(1, 5); //backing up stdout inside fd[5]
      close(1);
      prepareRedirection(cmd->args,is_append,cmd->args_num);
    }
    else {
      cmd->args_num = _parseCommandLine(cmd_line, cmd->args);
    }
    if (typeid(*cmd) == typeid(ExternalCommand)) { //external command
      pid_t child_pid = fork();
      if (child_pid == -1) {
        perror("smash error: fork failed");
      }
      else if (child_pid == 0) {//child
        setpgrp();
        cmd->execute();
      }
      else { //parent
        if (!bg_cmd) {
          if (!(cmd->args)[0]) {
            return;
          }
          int i = 0;
          std::string job_s;
          time_t curr_time;
          time(&curr_time);
          while (i < cmd->args_num-1) {
            job_s = job_s + string((cmd->args)[i]) + " ";
            i++;
          }
          job_s = job_s + string((cmd->args)[i]);
          this->curr_fg_p = new JobsList::JobEntry(job_s, child_pid, curr_time, false);
          if (waitpid(child_pid, &status, 0) == -1) {
            perror("smash error: waitpid failed");
          }
          delete this->curr_fg_p;
        }
        else { //it's running in the bg so we need to add it to the job list
          jobs->addJob(cmd, child_pid);
        }
      }
    }
    else {
      cmd->execute();
      if (string((cmd->args)[0]).compare("chprompt") == 0) {
          this->setNewPrompt(cmd->getPromptMessage());
      }
      if (string((cmd->args)[0]).compare("cd") == 0) {
          ChangeDirCommand* cmd2 = dynamic_cast<ChangeDirCommand*>(cmd);
          this->updateLastPWD(cmd2->plastPwd);
      }
    }
    if (_isRedirection(cmd_line)) { //if it was redirection, needs to restore stdout
      close(1);
      dup(5); // restores fd[5] which is the stdout back to fd[1]
      close(5);
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
    delete[] cmd->args;
}