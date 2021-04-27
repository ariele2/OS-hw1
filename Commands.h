#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <iterator>
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

class ExternalCommand : public Command {
 public:
  explicit ExternalCommand(const char* cmd_line) : Command(cmd_line) {}
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  bool is_append;
  explicit RedirectionCommand(const char* cmd_line, bool is_append) : Command(cmd_line), is_append(is_append) {}
  virtual ~RedirectionCommand() {}
  void execute() override;
  void prepare();
  void cleanup();
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

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
 public:
  class JobEntry {
    public:
    std::string cmd_name;
    int process_id;
    time_t time_created;
    bool stopped;
  };
 private:
  std::list<JobEntry*>* job_list;
 public:
  JobsList(std::list<JobEntry*>* job_list) : job_list(job_list) {}
  ~JobsList();
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry* getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  JobEntry* getJobByPos(int job_pos);
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
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CatCommand : public BuiltInCommand {
 public:
  CatCommand(const char* cmd_line);
  virtual ~CatCommand() {}
  void execute() override;
};


class SmallShell {
  private:
    SmallShell();
    std::string new_p;
  public:
    JobsList* jobs;
    char** plastPwd; 
    Command* CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
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
};

#endif //SMASH_COMMAND_H_