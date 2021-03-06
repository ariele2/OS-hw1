#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <iterator>
#include <unistd.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

const std::string WHITESPACE = " \n\r\t\f\v";

class Command {
  // TODO: Add your data members
  std::string prompt_message;
  public:
    char** args;
    int args_num;
    const char* cmd_line;
    Command(const char* cmd_line) : cmd_line(cmd_line) {}
    virtual ~Command() {}
    virtual void execute() = 0;
    // TODO: Add your extra methods if needed
    void changePromptMessage(std::string m) {
        prompt_message = m;
    }
    std::string getPromptMessage() {
        return prompt_message;
    }
    const char* retriveCMD() {
        return cmd_line;
    }
};

class BuiltInCommand : public Command {
 public:
  explicit BuiltInCommand(const char* cmd_line) : Command(cmd_line) {}
  virtual ~BuiltInCommand() {}
};
class JobsList;
class TimeoutsList;
class ExternalCommand : public Command {
 public:
  TimeoutsList* timeouts;
  JobsList* jobs;
  unsigned int duration;
  pid_t process_id;
  ExternalCommand(const char* cmd_line, TimeoutsList* timeouts, JobsList* jobs) : 
                            Command(cmd_line), timeouts(timeouts), jobs(jobs), duration(0), process_id(-2) {}
  ExternalCommand(const char* cmd_line, TimeoutsList* timeouts, JobsList* jobs, unsigned int duration) : 
                            Command(cmd_line), timeouts(timeouts), jobs(jobs), duration(duration) {}
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  bool is_stderr;
  PipeCommand(const char* cmd_line, bool is_stderr) : Command(cmd_line), is_stderr(is_stderr) {}
  virtual ~PipeCommand() {}
  void execute() override;
};

class ChPromptCommand : public BuiltInCommand {
 public:
  ChPromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ChPromptCommand() {}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  public:
    char** plastPwd;
    ChangeDirCommand(const char* cmd_line, char** plastPwd) : BuiltInCommand(cmd_line), plastPwd((!plastPwd) ? new char*[COMMAND_ARGS_MAX_LENGTH]:plastPwd) {}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
 public:
  JobsList* jobs;
  QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
 public:
  class JobEntry {
    public:
    JobEntry() = default;
    JobEntry(std::string cmd_name, int process_id, time_t time_created, bool stopped, int pos) : 
                                    cmd_name(cmd_name), process_id(process_id), time_created(time_created), stopped(stopped), pos(pos) {}
    std::string cmd_name;
    int process_id;
    time_t time_created;
    bool stopped;
    int pos;
  };
 public:
  std::list<JobEntry*>* job_list;
  int biggest_job;
  JobsList(std::list<JobEntry*>* job_list, int biggest_job) : job_list(job_list), biggest_job(biggest_job) {}
  ~JobsList();
  void addJob(Command* cmd, int child_pid, const char* cmd_line, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry* getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry* getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  JobEntry* getJobByPos(int job_pos);
  int getJobsListSize();
  void addJobByPos(int pos, JobEntry* job);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
 JobsList* jobs;
  JobsCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs) {}
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsList* jobs;
  KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsList* jobs;
  ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsList* jobs;
  BackgroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs) {}
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CatCommand : public BuiltInCommand {
 public:
  CatCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~CatCommand() {}
  void execute() override;
};

class TimeoutsList{
  public:
    class Timeout {
    public:
    pid_t process_id;
    time_t time_created;
    unsigned int duration;
    std::string cmd_name;
    Timeout(std::string cmd_name, pid_t process_id, time_t time_created, unsigned int duration)
            :process_id(process_id), time_created(time_created), duration(duration), cmd_name(cmd_name) {}
    };
private:
    std::vector<Timeout> timeout_list;
public:
    TimeoutsList() : timeout_list(std::vector<Timeout>()) {}
    ~TimeoutsList() = default;
    void addTimeout(std::string cmd_name, pid_t process_id, time_t time_created, unsigned int duration);
    unsigned int getDurationByPid(pid_t pid);
    void killAllTimeouts();
    void removeFinishedTimeouts();
    Timeout* findTimeoutByTime(time_t curr_time);
    void removeTimeoutByPid(pid_t pid);
    unsigned int getSize();
    pid_t getPidByIndex(unsigned int index);
    unsigned int getClosestTimeout(pid_t* pid);
};

class SmallShell {
  private:
    SmallShell();
    std::string new_p;
  public:
    int smash_pid;
    char** plastPwd;
    TimeoutsList timeouts;
    JobsList* jobs; 
    JobsList::JobEntry* curr_fg_p;
    Command* CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    TimeoutsList* getTimeoutsList() { 
      return &(timeouts); 
    }
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
    void setNewPrompt(std::string p) {
        new_p = p;
    }
    void updateLastPWD(char** p) {
        plastPwd = p;
    }
    std::string retrivePrompt() {
        return new_p;
    }
    int getPid() {
      return smash_pid;
    }
};

#endif //SMASH_COMMAND_H_