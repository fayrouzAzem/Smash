#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
using std::string;
using std::vector;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
    const char* cmd_line;
public:
    Command(const char* cmd_line);
    virtual ~Command() {}
    virtual void execute() = 0;
    const char* getCmd() {
        return cmd_line;
    }
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};


class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    string cmd1;
    string cmd2;
    bool is_stderr;
    // char delimiter;
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    vector<string> args_of_cmd;
    vector<string> args_of_file_name;
    string cmd_line_string;
    bool to_overwrite = true;
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChpromptCommand : public BuiltInCommand { // new! added by fayrouz. command 1 in the "built-in commands" part in the file.
    string inSidepromptName; // new
public:
    ChpromptCommand(const char* cmd_line, vector<string>& args, int argsSize); // the argument args is extracted from the input command line string and stored in a vector of arguments.
    virtual ~ChpromptCommand() = default; // changed
    void execute() override;
    // consider adding nested classes for validation purposes or error checking (this way of coding is more modular and organized, note that you may as well leave this part to the execute function)
};

class ShowPidCommand : public BuiltInCommand { // command 2
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand { // command 3
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand { // command 4
// TODO: Add your data members public:
public:
    vector<string> args;
    int argsSize;
    ChangeDirCommand(const char* cmd_line, vector<string>& args, int argsSize );
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    vector <string> args;
    int argsSize;
    QuitCommand(const char* cmd_line,vector<string>& args, int argsSize );
    virtual ~QuitCommand() = default;
    void execute() override;
};


class JobsList {
public:
    class JobEntry {
    public:
        // TODO: Add your data
        int job_id;
        string cmd_line;
        pid_t jobPid;
        time_t insertion_time;
        bool is_stopped;

        JobEntry(int jobId, string cmd_line, pid_t pid, time_t insertionTime, bool isStopped); // implemented in .cpp

        int getElapsedTIme() const { // maybe this function is unnecessary
            time_t now;
            time(&now);
            return difftime(now, insertion_time);
        }
    };
    // TODO: Add your data members
    vector<JobEntry> jobs_entry_vec;
    int max_job_id;
    int num_of_stopped_jobs;

public:
    JobsList(); // implemented in .cpp
    ~JobsList() = default;
    void addJob(Command* cmd, pid_t pid, bool isStopped = false);
    void printJobsList(); // implemented in .cpp
    void killAllJobs();
    int getSize();
    void removeFinishedJobs(); // implemented in .cpp
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand { // command 5
    // TODO: Add your data members
public:
    JobsCommand(const char* cmd_line); //
    //JobsCommand(const char* cmd_line, JobsList* jobs); // that's the course's suggested definition
    virtual ~JobsCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {

    vector<string> args;
    int argsSize;
public:
    ForegroundCommand(const char* cmd_line, vector<string>& args, int argsSize);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    vector<string> args;
    int argsSize;
public:
    BackgroundCommand(const char* cmd_line,vector<string>& args, int argsSIze);
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char* cmd_line);
    virtual ~TimeoutCommand() {}
    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
    // TODO: Add your data members
    string new_mode_Str;
    string file_path;
public:
    ChmodCommand(const char* cmd_line);
    virtual ~ChmodCommand() {}
    void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
    // TODO: Add your data members
    vector<string> args;
    int argsSize;
public:
    GetFileTypeCommand(const char* cmd_line,vector<string>& args,int argsSize);
    virtual ~GetFileTypeCommand() {}
    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
    vector<string> args;
    int argsSize;
public:
    SetcoreCommand(const char* cmd_line,vector<string>& args, int argsSize);
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members

    vector<string> args;
    int argsSize;
public:
    KillCommand(const char* cmd_line, vector<string>& args, int argsSize);
    //KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() = default;
    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();
public:
    JobsList* jobs_list; // new! added to implement the "execute" function of JobsCommand

    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete;
    bool isSetGrp;
    bool quitSignal = false;
    string promptName;
    string previousDR;
    int currentJobPid;
    int currentJobId;
    string currentJobCommand;
    pid_t smash_pid;
    int getPidSmash(){
        return smash_pid;
    };
    string getPrompName(){
        return promptName;
    }
    string getPreviousDR(){
        return previousDR;
    }
    int getCurrentJobId(){
        return currentJobId;
    }
    int getCurrentJobPid(){
        return currentJobPid;
    }
    string getCurrentJobCommand(){
        return currentJobCommand;

    }
    void setCurrJobId(int jobId){
        currentJobId = jobId;
    }
    void setCurrJobPid(int jobPid){
        currentJobPid = jobPid;
    }
    void setCurrjobCommand(string jobCMD){
        currentJobCommand = jobCMD;
    }

    void setPreviousDR(string Previous){
        previousDR = Previous;
    }
    void setPromptName(string PR){
        promptName= PR;
    }
    bool getGrp(){
        return isSetGrp;
    }

    ;

    // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed

};


#endif //SMASH_COMMAND_H_