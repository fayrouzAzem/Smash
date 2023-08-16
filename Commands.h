#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
using std::string;
using std::vector;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
	const char* cmd_line;

public:
	Command(const char* cmd_line);
	virtual ~Command() {}
	virtual void execute() = 0;
	const char* getCmd() { return cmd_line; }
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

public:
	PipeCommand(const char* cmd_line);
	virtual ~PipeCommand() {}
	void execute() override;
};

class RedirectionCommand : public Command {
	vector<string> args_of_cmd;
	vector<string> args_of_file_name;
	string cmd_line_string;
	bool to_overwrite = true;

public:
	explicit RedirectionCommand(const char* cmd_line);
	virtual ~RedirectionCommand() {}
	void execute() override;
};

class ChpromptCommand : public BuiltInCommand {  
	string inSidepromptName;  

public:
	ChpromptCommand(const char* cmd_line, vector<string>& args,
		int argsSize);  // the argument args is extracted from the input command line string and stored in a
						// vector of arguments.
	virtual ~ChpromptCommand() = default;  
	void execute() override;
};

class ShowPidCommand : public BuiltInCommand {  
public:
	ShowPidCommand(const char* cmd_line);
	virtual ~ShowPidCommand() {}
	void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {  
public:
	GetCurrDirCommand(const char* cmd_line);
	virtual ~GetCurrDirCommand() {}
	void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {  
public:
	vector<string> args;
	int argsSize;
	ChangeDirCommand(const char* cmd_line, vector<string>& args, int argsSize);
	virtual ~ChangeDirCommand() {}
	void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
public:
	vector<string> args;
	int argsSize;
	QuitCommand(const char* cmd_line, vector<string>& args, int argsSize);
	virtual ~QuitCommand() = default;
	void execute() override;
};

class JobsList {
public:
	class JobEntry {
	public:
		int job_id;
		string cmd_line;
		pid_t jobPid;
		time_t insertion_time;
		bool is_stopped;

		JobEntry(int jobId, string cmd_line, pid_t pid, time_t insertionTime, bool isStopped);  // implemented in .cpp

		int getElapsedTIme() const {  
			time_t now;
			time(&now);
			return difftime(now, insertion_time);
		}
	};

	vector<JobEntry> jobs_entry_vec;
	int max_job_id;
	int num_of_stopped_jobs;

public:
	JobsList();  // implemented in .cpp
	~JobsList() = default;
	void addJob(Command* cmd, pid_t pid, bool isStopped = false);
	void printJobsList();  // implemented in .cpp
	void killAllJobs();
	int getSize();
	void removeFinishedJobs();  // implemented in .cpp
	JobEntry* getJobById(int jobId);
	void removeJobById(int jobId);
	JobEntry* getLastJob(int* lastJobId);
	JobEntry* getLastStoppedJob(int* jobId);
};

class JobsCommand : public BuiltInCommand {  
											 
public:
	JobsCommand(const char* cmd_line);  
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
	vector<string> args;
	int argsSize;

public:
	BackgroundCommand(const char* cmd_line, vector<string>& args, int argsSIze);
	virtual ~BackgroundCommand() {}
	void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
public:
	explicit TimeoutCommand(const char* cmd_line);
	virtual ~TimeoutCommand() {}
	void execute() override;
};

class ChmodCommand : public BuiltInCommand {
	string new_mode_Str;
	string file_path;

public:
	ChmodCommand(const char* cmd_line);
	virtual ~ChmodCommand() {}
	void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
	vector<string> args;
	int argsSize;

public:
	GetFileTypeCommand(const char* cmd_line, vector<string>& args, int argsSize);
	virtual ~GetFileTypeCommand() {}
	void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
	vector<string> args;
	int argsSize;

public:
	SetcoreCommand(const char* cmd_line, vector<string>& args, int argsSize);
	virtual ~SetcoreCommand() {}
	void execute() override;
};

class KillCommand : public BuiltInCommand {
	vector<string> args;
	int argsSize;

public:
	KillCommand(const char* cmd_line, vector<string>& args, int argsSize);
	virtual ~KillCommand() = default;
	void execute() override;
};

class SmallShell {
private:
	SmallShell();

public:
	JobsList* jobs_list; 
	Command* CreateCommand(const char* cmd_line);
	SmallShell(SmallShell const&) = delete;  
	void operator=(SmallShell const&) = delete;
	bool isSetGrp;
	bool quitSignal = false;
	string promptName;
	string previousDR;
	int currentJobPid;
	int currentJobId;
	string currentJobCommand;
	pid_t smash_pid;
	int getPidSmash() { return smash_pid; };
	string getPrompName() { return promptName; }
	string getPreviousDR() { return previousDR; }
	int getCurrentJobId() { return currentJobId; }
	int getCurrentJobPid() { return currentJobPid; }
	string getCurrentJobCommand() { return currentJobCommand; }
	void setCurrJobId(int jobId) { currentJobId = jobId; }
	void setCurrJobPid(int jobPid) { currentJobPid = jobPid; }
	void setCurrjobCommand(string jobCMD) { currentJobCommand = jobCMD; }
	void setPreviousDR(string Previous) { previousDR = Previous; }
	void setPromptName(string PR) { promptName = PR; }
	bool getGrp() { return isSetGrp; }

	// disable = operator
	static SmallShell& getInstance()  // make SmallShell singleton
	{
		static SmallShell instance;  // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}

	~SmallShell();
	void executeCommand(const char* cmd_line);
};

#endif  // SMASH_COMMAND_H_
