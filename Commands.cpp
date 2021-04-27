#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <ctime>
#include "Commands.h"

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
void JobsList::addJob(Command* cmd, bool isStopped) {
  JobsList::JobEntry* new_job = new JobEntry();
  time_t curr_time;
  time(&curr_time);
  new_job->cmd_name = string((cmd->args)[0]) + " " + string((cmd->args)[1]);
  new_job->process_id = getppid(); //the addjob process should be called from child process and wait
  new_job->time_created = curr_time;
  new_job->stopped = isStopped;
  (this->job_list)->push_back(new_job);
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

void JobsList::killAllJobs() {

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
    std::cout << "signal number "<< signal_num <<"was sent to pid " << job_to_kill->process_id <<std::endl;
  }
  else {
    perror("smash error: kill failed");
  }
}

//before the print, dont forget to check if there are processes to remove.
void JobsCommand::execute() {
  jobs->printJobsList();
}

void RedirectionCommand::prepare() {
  dup2(0, 4); //backing up stdin inside fd[4]
  dup2(1, 5); //backing up stdout inside fd[5]
  close(0);
  close(1);
  /*std::string w_spath, r_spath;
  if (!(this->args)[1]) {// the > or >> sign shows up withough backspaces
  std::string cmd_s = string((this->args)[0]);
  r_spath = cmd_s.substr(0, cmd_s.find_first_of(">")); //path for the new "stdin"
    if (cmd_s.find(">>") != string::npos) {
      w_spath = cmd_s.substr(cmd_s.find_first_of(">")+2);
    }
    else {
      w_spath = cmd_s.substr(cmd_s.find_first_of(">")+1);
    }
  }
  else if (!(this->args)[2]) { //the > or >> sign shows up without one backspace
    std::string cmd_a1 = string((this->args)[0]);
    std::string cmd_a2 = string((this->args)[1]);
    if (cmd_a1.find(">>") != string::npos || cmd_a1.find(">") != string::npos) { //backspace in first word
      w_spath = cmd_a2;
      r_spath = cmd_a1.substr(0, cmd_s.find_first_of(">"));
    }
    else {
      r_spath = cmd_a1;
      if (cmd_a1.find(">>") != string::npos) { //backspace in second word
        w_spath = cmd_a2.substr(cmd_a2.find_first_of(">")+2);
      }
      else {
        w_spath = cmd_a2.substr(cmd_a2.find_first_of(">")+1);
      }
    }
  }
  else { //3 arguments means the path is in the third one
    w_spath = string((this->args)[2]);
    r_spath = string((this->args)[0]);
  }
  const char* r_path = r_spath.c_str();
  const char* w_path = w_spath.c_str();
  open((r_path, O_RDONLY); //open the new stdin
  if (!is_append) {
    open((w_path, O_WRONLY|O_CREATE, S_IWUSR));
  }
  else {
    open((w_path, O_WRONLY|O_CREATE|O_APPEND, S_IWUSR);
  }*/
}

void RedirectionCommand::cleanup() {
  close(0);
  close(1);
  dup(4); // restores fd[4] which is the stdin back to fd[0]
  dup(5); // restores fd[5] which is the stdout back to fd[1]
  close(4);
  close(5);
}

void RedirectionCommand::execute() {
  this->prepare();
}

void ExternalCommand::execute(){
  int i=0;
  std::string cmd;
  while (this->args[i]) {
    _removeBackgroundSign(this->args[i]);
    cmd = cmd + string(this->args[i]) + " ";
    i++;
  }
  char cmd_c[COMMAND_ARGS_MAX_LENGTH];
  strcpy(cmd_c, cmd.c_str());
  char* argsv[2] = {cmd_c, NULL};
  std::cout << "argsv of external cmd: " << *argsv <<std::endl;
  if (!(execv("/bin/bash", argsv))) {
      perror("smash error: execv failed");
  }
  std::cout <<"son reached a place he is not supposed to be.." << std::endl;
}


SmallShell::SmallShell(): new_p("smash>"), plastPwd(nullptr) {
// TODO: add your implementation
  std::list<JobsList::JobEntry*>* job_list = new std::list<JobsList::JobEntry*>();
  this->jobs = new JobsList(job_list);
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (cmd_s.find(">") != std::string::npos || cmd_s.find(">>") != std::string::npos) {
      bool is_append = false;
      if (cmd_s.find(">>") != string::npos) {
        is_append = true;
      }
      return new RedirectionCommand(cmd_line, is_append);
    }
    else if (firstWord.compare("chprompt") == 0) {
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
    else {
      return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char* cmd_line) {
    // TODO: Add your implementation here
    // for example:
    bool bg_cmd = _isBackgroundComamnd(cmd_line);
    Command* cmd = CreateCommand(cmd_line);
    cmd->args = new char*[COMMAND_ARGS_MAX_LENGTH]();
    cmd->args_num = _parseCommandLine(cmd_line, cmd->args);
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
          wait(NULL);
        }
        else { //it's running in the bg so we need to add it to the job list
          jobs->addJob(cmd, false);
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
    // Please note that you must fork smash process for some commands (e.g., external commands....)
    delete[] cmd->args;
}