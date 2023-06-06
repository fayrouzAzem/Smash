#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iomanip>
#include "Commands.h"
#include <algorithm>
#include <sched.h>
#include <fstream>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/types.h>


using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s) //_Itrim returns a new string that is a substring of the original string s, with any leading whitespace characters removed.
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) //_rtrim returns a new string that is a substring of the original string s, with any trailing whitespace characters removed.
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) //_trim returns a new string that is a substring of the original string s, with any leading or trailing whitespace characters removed.
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) { //_parseCommandLine parses the command line string cmd_line into individual arguments and stores them in the array of strings args. The function returns the number of arguments parsed.
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

bool _isBackgroundComamnd(const char* cmd_line) { //isBackgroundComamnd checks whether the input command line string ends with the & symbol to determine whether the command should be run in the background or not.
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) { //_removeBackgroundSign removes the background sign & from the end of the command line if it is present, and removes any trailing whitespace characters after the background sign.
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

// definitions



// auxiliary methods.
char** vecToCharArr(vector<string> args)
{
    char** args_c = new char*[args.size() + 1];
    for (size_t i = 0; i < args.size(); i++)
    {
        args_c[i] = new char[args[i].size() + 1];
        strcpy(args_c[i], args[i].c_str());
    }
    args_c[args.size()] = nullptr;

    return args_c;
}

int newParseCommand(const std::string& cmd_line,std::vector<string>& args){
    std::istringstream iss(cmd_line);
    std::string word;
    while( iss >> word){
        args.push_back(word);
    }
    return args.size();

}
// Command Class.
Command::Command(const char *cmd_line) : cmd_line(cmd_line){}
// TODO: Add your implementation for classes in Commands.h
BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line){}


// implementations of ChpromptCommand methods
ChpromptCommand::ChpromptCommand(const char *cmd_line, vector<string> &args, int argsSize) : BuiltInCommand(cmd_line)
{
    //if no parameters were given, set the promptName to smash
    if (argsSize == 1) {
        inSidepromptName = "smash>";
    }
        //if one parameter or more were given, set the promptName to the first parameter
    else if(argsSize>1){
        inSidepromptName = args[1];
        //add >
        inSidepromptName += ">";
    }

}

void ChpromptCommand::execute()
{
    //change the prompt name

    SmallShell::getInstance().setPromptName(inSidepromptName);
    //going through getInstance() to call setPromptName is important since we're working with a singleton design pattern.
    //When working with a singleton design pattern, we create one instance of the class and then use that same instance
    //throughout the program. In the case of the SmallShell class, calling SmallShell::getInstance() returns the same instance
    //every time it is called, so all code that interacts with the SmallShell class is working with the same instance.
    //This ensures that the state of the SmallShell instance is consistent throughout the program and avoids problems that
    //might arise from multiple instances of the class being created and potentially conflicting with each other.
}

// implementations of ShowPidCommand methods
// showpidCommand

SmallShell::SmallShell():promptName("smash>"),previousDR("empty"),currentJobPid(-1),currentJobId(-1),currentJobCommand("")
{
    smash_pid= getpid();
    if(smash_pid==-1)
    {
        perror("smash error: getpid failed");
        return;
    }
    isSetGrp = true;
    jobs_list=new JobsList();

}
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void ShowPidCommand::execute() {
    std::cout << "smash pid is " << SmallShell::getInstance().getPidSmash() << std::endl;
}


// implementations of GetCurrDirCommand methods
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void GetCurrDirCommand::execute()
{
    char cwd[COMMAND_ARGS_MAX_LENGTH+1];
    getcwd(cwd, sizeof(cwd));
    cout << cwd << endl;
}

// implementations of ChangeDirCommand methods
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, vector<std::string>& args, int argSize) : BuiltInCommand(cmd_line),args(args),argsSize(argSize){}

// I HAVE TO ADD PERROR ERRORS (SYSCALLS ERRORS TOO)
void ChangeDirCommand::execute() {
    if(argsSize != 2) {
        std::cerr<<"smash error: cd: too many arguments"<<std::endl;
    }
    else if(argsSize == 2 && (args[1] == "-") && SmallShell::getInstance().previousDR == "empty"){
        std::cerr<<"smash error: cd: OLDPWD not set"<<std::endl;
    }
    else if(args[1] == "-" && argsSize ==2 && SmallShell::getInstance().previousDR !="empty" ){

        char cwd[COMMAND_ARGS_MAX_LENGTH+1];
        getcwd(cwd, sizeof(cwd));
        auto currDirectory = cwd;
        int result = chdir(SmallShell::getInstance().previousDR .c_str());
        if(result<0){
            std::perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().setPreviousDR(currDirectory) ;
    }
    else if(argsSize ==2 && args[1] != "-"){
        char cwd[COMMAND_ARGS_MAX_LENGTH+1];
        getcwd(cwd, sizeof(cwd));
        auto CurrDirectory = cwd;

        int result = chdir(args[1].c_str());
        if (result < 0){
            std::perror("smash error: chdir failed");
            return;
        }
        SmallShell::getInstance().setPreviousDR(CurrDirectory) ;
    }
}

// implementations of JobsCommand methods
JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void JobsCommand::execute()
{

    SmallShell::getInstance().jobs_list->printJobsList();
}



SmallShell::~SmallShell() {
// TODO: add your implementation
}

// implementations of KillCommand methods
KillCommand::KillCommand(const char* cmd_line, vector<string>& args,int argsSize) : BuiltInCommand(cmd_line), args(args),argsSize(argsSize) {}

void KillCommand::execute()
{
    // get the signal number

    if(argsSize !=3 ){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    else if(args[1].find("-") != 0 ){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    for( unsigned int i =1 ; i<args[1].length()-1 ; i++) {
        if (!std::isdigit(args[1][i])) {
            cerr << "smash error: kill: invalid arguments" << endl;
            return;
        }
    }

    if(args[2][0] == '-') {

        int job_id = stoi(args[2]);
        for (unsigned int i = 1; i < args[2].length(); i++) {
            if (!std::isdigit(args[2][i])) {
                cerr << "smash error: kill: invalid arguments" << endl;
                return;
            }
            cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
            return;
        }
    }
    else {
        for (unsigned int i = 0; i < args[2].length(); i++) {
            if (!std::isdigit(args[2][i])) {
                cerr << "smash error: kill: invalid arguments" << endl;
                return;
            }
        }
    }

    int job_id = stoi(args[2]);
    int signum = stoi(args[1].substr(1));
    if(signum>31 || signum <1){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    JobsList::JobEntry* job = SmallShell::getInstance().jobs_list->getJobById(job_id);
    if (!job){
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }


    if(kill(job->jobPid, signum) < 0){
        perror("smash error: kill failed ");
    };
    cout << "signal number " << signum << " was sent to pid " << job->jobPid << endl;

}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
ForegroundCommand::ForegroundCommand(const char *cmd_line, vector<std::string> &args, int argsSize) : BuiltInCommand(cmd_line), args(args),argsSize(argsSize){}

void ForegroundCommand::execute() {
    SmallShell::getInstance().jobs_list->removeFinishedJobs();

    if( argsSize > 2 ){
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    JobsList::JobEntry* Job;
    string myCMD;
    pid_t myPid;
    bool proccesCheck;

    if(argsSize == 2) {


        if(args[1][0]== '-') {
            for (unsigned int i = 1; i < args[1].size(); i++) {
                if (!isdigit(args[1][i])) {
                    cerr << "smash error: fg: invalid arguments" << endl;
                    return;
                }
            }
            int job_id = stoi(args[1]);
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }else{
            for (unsigned int  i = 0; i < args[1].size(); i++) {
                if (!isdigit(args[1][i])) {
                    cerr << "smash error: fg: invalid arguments" << endl;
                    return;
                }
            }
        }
        int job_id = stoi(args[1]);
        Job =  SmallShell::getInstance().jobs_list->getJobById(job_id);
        if( !Job){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }
        myCMD = Job->cmd_line;
        myPid = Job->jobPid;
        proccesCheck = Job->is_stopped;

    }


    if (SmallShell::getInstance().jobs_list->jobs_entry_vec.size() == 0 && argsSize == 1){
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }

    if(argsSize == 1  ){
        Job = SmallShell::getInstance().jobs_list->getJobById(SmallShell::getInstance().jobs_list->max_job_id);
        myCMD = Job->cmd_line;
        myPid = Job->jobPid;
        proccesCheck = Job->is_stopped;
    }
    std::cout << myCMD << " : " << myPid << std::endl;
    if(proccesCheck){
        if(killpg(myPid,SIGCONT)<0){
            perror("smash error: kill failed");
            return;
        }
    }
    SmallShell::getInstance().setCurrJobPid(myPid);
    int STATUS =0;
    pid_t checkPid = waitpid(myPid,&STATUS,WUNTRACED);
    if(checkPid == -1){
        perror("smash error: waitpid failed");
        return;
    }
    if(WIFSTOPPED(STATUS) || WIFSIGNALED(STATUS)){
        return;
    }
    if(checkPid == myPid){
        for(auto i = SmallShell::getInstance().jobs_list->jobs_entry_vec.begin();i<SmallShell::getInstance().jobs_list->jobs_entry_vec.end();i++ )
            if(i->jobPid == myPid){
                SmallShell::getInstance().jobs_list->jobs_entry_vec.erase(i);
                return;
            }
        SmallShell::getInstance().setCurrJobPid(-1);
    }




}
QuitCommand::QuitCommand(const char* cmd_line, vector<string>& args,int argsSize) : BuiltInCommand(cmd_line),
                                                                                    args(args),argsSize(argsSize) {}

void QuitCommand::execute() {
    SmallShell::getInstance().jobs_list->removeFinishedJobs();
    auto num = SmallShell::getInstance().jobs_list->jobs_entry_vec.size();
    auto start = SmallShell::getInstance().jobs_list->jobs_entry_vec.begin();
    auto end = SmallShell::getInstance().jobs_list->jobs_entry_vec.end();

    if((argsSize) && args[1] == "kill") {

        std::cout << "smash: sending SIGKILL signal to " << num << " jobs:" << std::endl;
        for (vector<JobsList::JobEntry>::iterator p = start; p < end; ++p) {
            std::cout << p->jobPid << ": " << p->cmd_line << std::endl;
        }
    }
    SmallShell::getInstance().jobs_list->killAllJobs();
    SmallShell::getInstance().quitSignal=true;
}



BackgroundCommand::BackgroundCommand(const char *cmd_line, vector<std::string> &args, int argsSize) : BuiltInCommand(cmd_line), args(args), argsSize(argsSize){}

void BackgroundCommand::execute() {
    if( argsSize > 2 ){
        cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }


    if(argsSize == 2) {


        if(args[1][0]== '-') {
            for (unsigned int i = 1; i < args[1].size(); i++) {
                if (!isdigit(args[1][i])) {
                    cerr << "smash error: bg: invalid arguments" << endl;
                    return;
                }
            }

            int job_id = stoi(args[1]);

            cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
            return;
        }else{
            for (unsigned int i = 0; i < args[1].size(); i++) {
                if (!isdigit(args[1][i])) {
                    cerr << "smash error: bg: invalid arguments" << endl;
                    return;
                }
            }
        }

        int job_id = stoi(args[1]);
        auto curr_job =SmallShell::getInstance().jobs_list->getJobById(job_id);
        if(!curr_job){
            cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
            return;
        }

        if(!curr_job->is_stopped){
            cerr << "smash error: bg: job-id " << job_id << " is already running in the background" <<  endl;
            return;
        }
        cout << curr_job->cmd_line << " : " << curr_job->jobPid << endl;
        curr_job->is_stopped = false;
        if(killpg(curr_job->jobPid, SIGCONT) < 0) {

            perror("smash error: kill failed");
            return;
        }
    }
    int job_id1;
    if(argsSize == 1){
        if(!SmallShell::getInstance().jobs_list->getLastStoppedJob(&job_id1)){
            cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        }
        else{


            auto curr_job =SmallShell::getInstance().jobs_list->getLastStoppedJob(&job_id1);
            curr_job->is_stopped = false;
            cout << curr_job->cmd_line << " : " << curr_job->jobPid << endl;
            if(kill(curr_job->jobPid, SIGCONT) < 0) {

                perror("smash error: kill failed");
                return;
            }
        }
    }
// i have to check when to call the killpg syscall
}
ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {}

void ExternalCommand::execute() {
    bool checkCommand = _isBackgroundComamnd(getCmd());
    char* myCMD=new char[strlen(getCmd())+1];
    strcpy(myCMD,getCmd());

    if(checkCommand) {
        _removeBackgroundSign((char *) myCMD);
    }
    char *  argv[4];
    argv[0]=new char[10];
    argv[1]=new char[3];
    strcpy(argv[0],"/bin/bash");
    strcpy( argv[1],"-c");
    argv[2]=myCMD;
    argv[3]=NULL;

    pid_t pid = fork();
    if(pid == -1) {
        perror("smash error: fork failed");
        return;
    }
    if(pid == 0) {
        if(setpgrp() < 0){
            perror("smash error: setgrp failed");
            delete argv[0];
            delete argv[1];
        }
        if(execv(argv[0], argv) == -1) {
            delete argv[0];
            delete argv[1];
        }
    }
    if(pid > 0) {
        if(checkCommand) {

            SmallShell::getInstance().jobs_list->addJob(this, pid);
        }

        else {
            SmallShell::getInstance().setCurrJobPid(pid);
            SmallShell::getInstance().setCurrjobCommand(myCMD);
            if(SmallShell::getInstance().isSetGrp) {
                if (waitpid(pid, NULL, WUNTRACED) == -1) {
                    perror("smash error: waitpid failed");
                    return;
                }
            }
            else{
                wait(NULL);
            }
            SmallShell::getInstance().setCurrJobPid(-1);
            SmallShell::getInstance().setCurrjobCommand("");

        }
        delete argv[0];
        delete argv[1];

    }
    return;
}
PipeCommand::PipeCommand(const char* cmd_line) : Command(cmd_line) {
    string cmd_str;
    cmd_str.assign(cmd_line); // initialize 'cmd_str' with a copy of 'cmd_line'

    // Find the position of the pipe symbol and determine whether it's a normal or a stderr pipe
    size_t pipe_pos = cmd_str.find("|&");
    if (pipe_pos == string::npos) {
        is_stderr = false;
        pipe_pos = cmd_str.find("|");
        //delimiter = '|';
    }
    else {
        is_stderr = true;
        //delimiter = '|&';
    }

    cmd1 = cmd_str.substr(0, pipe_pos);
    cmd2 = cmd_str.substr(pipe_pos + (is_stderr ? 2 : 1));

    _removeBackgroundSign((char*)cmd1.c_str());
    _removeBackgroundSign((char*)cmd2.c_str());
}

void PipeCommand::execute() {
    // create pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("smash error: pipe failed"); // I'M NOT SURE OF THIS MESSAGE!!!
        return;
    }
    // Fork first child to execute command1
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("smash error: fork failed");
        return;
    }
    else if (pid1 == 0) {
        if(!is_stderr){
            if(dup2(fd[1],1) == -1){
                perror("smash error: dup2 failed");
            };

        }
        else{
            if(dup2(fd[1],2) == -1){
                perror("smash error: dup2 failed");
            }
        }// Child process
        // Redirect stdout to pipe's write channel

        if(close(fd[0]) == -1){
            perror("smash error: close failed");
        }// LOOK FURTHER INTO THIS
        if(close(fd[1]) == -1){
            perror("smash error: close failed");
        }

        // Execute command1

        Command* command1 = SmallShell::getInstance().CreateCommand(cmd1.c_str());
        command1->execute();
        delete command1; // prevents memory leaks
        exit(0); // terminate the child process (it finished executing command1)
    }

    // Create child process for cmd2
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid2 == 0) { // child process
        // Redirect stdin to read end of pipe
        if(dup2(fd[0],0)==-1){
            perror("smash error: dup2 failed");
        }
        if(close(fd[0])== -1){
            perror("smash error: close failed");
        }
        if(close(fd[1]) == -1){
            perror("smash error: close failed");
        }

        // Execute cmd2

        Command* command2 = SmallShell::getInstance().CreateCommand(cmd2.c_str());
        command2->execute();
        delete command2; // prevents memory leaks
        exit(0); // terminate the child process (it finished executing command1)
    }

    // Parent process
    if(close(fd[0]) == -1){
        perror("smash error: close failed");
    }
    if(close(fd[1]) == -1){
        perror("smash error: close failed");
    }
    // Wait for child processes to finish
    if(waitpid(pid1, NULL, 0) == -1){
        perror("smash error: waitpid failed");
    }
    if(waitpid(pid2, NULL, 0) == -1){
        perror("smash error: waitpid failed");
    }

}
// RedirectionCommand Class
vector<string> splitStringIntoVec(string str, char delimiter) {
    vector<string> result_vec;
    stringstream ss(str); // a 'stringstream' is like 'string' object that can be read and written to like a file
    string sub; // 'sub' will be used to store each individual sub-string as it's extracted from the input stream

    while(getline(ss, sub, delimiter)) {
        result_vec.push_back(sub);
    }
    return result_vec;
}
RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command(cmd_line) {
    string str;
    string cmd;
    string file_name;

    str.assign(cmd_line); // initialize 'str' with a copy of 'cmd_line'
    size_t position_of_red_sign = str.find(">>"); // 'find' searches for '>' in 'str' and returns its position. if '>' is not found, 'find' returns 'string::npos'

    if (position_of_red_sign == string::npos) {
        // '>' was not found in 'str', search for '>>'
        position_of_red_sign = str.find(">");
        cmd = str.substr(0, position_of_red_sign);
        file_name = str.substr(position_of_red_sign + 1, str.length() - position_of_red_sign - 1);
        _removeBackgroundSign((char*)file_name.c_str());
        to_overwrite = true;
    }
    else {
        // '>' was found in 'str'
        position_of_red_sign = str.find(">>");
        cmd = str.substr(0, position_of_red_sign);
        file_name = str.substr(position_of_red_sign + 2, str.length() - position_of_red_sign - 2);
        _removeBackgroundSign((char*)file_name.c_str());
        to_overwrite = false;

    }
    args_of_cmd = splitStringIntoVec(cmd.c_str(), ' ');
    args_of_file_name = splitStringIntoVec(file_name.c_str(), ' ');
    cmd_line_string = cmd;
}

void RedirectionCommand::execute() {
    string file_name;

    if(args_of_file_name.size()==1){
        file_name = args_of_file_name[0];
    }
    else{
        file_name = args_of_file_name[1];
    }


    int file;
    // IMPORTANT NOTE: consider opening and closing the file to check if any errors happen
    string cmd = args_of_cmd[0];
    // check if the provided cmd is a built_in cmd

    int standard_out_channel = dup(1); // dup(1) creates a copy of the standard output file descriptor
    if (standard_out_channel == -1) {
        perror("smash error: dup failed");
        return;
    }

    if (to_overwrite == true) {
        file = open(file_name.c_str(), O_TRUNC | O_CREAT | O_RDWR, 0655);
        if (file == -1) {
            perror("smash error: open failed");
            return;
        }
    }
    else {
        file = open(file_name.c_str(), O_APPEND | O_CREAT | O_RDWR, 0655);
        if (file == -1) {
            perror("smash error: open failed");
            return;
        }
    }
    int res = close(1);
    if(res == -1){
        perror("smash error: close failed");
    }
    res = dup(file);
    if(res == -1){
        perror("smash error: close failed");
    }

    SmallShell& ss_instance = SmallShell::getInstance();
    ss_instance.executeCommand(cmd_line_string.c_str());
    if (close(1) == -1) {
        perror("smash error: close failed"); // close the CURRENT stdout file discreptor (the file)
        return;
    }
    if (dup2(standard_out_channel, 1) == -1) { // restore the original stdout (replace the current stdout (the file) with the original stdout
        perror("smash error: dup2 failed"); //
        return;
    }



    // the provided cmd is an external cmd


}
// implementations of ChmodCommand methods
ChmodCommand::ChmodCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS] = {nullptr};
    int args_num = _parseCommandLine(cmd_line, args);

    // Check if the command has the correct number of arguments
    if (args_num != 3) {
        cerr << "smash error: chmod: invalid arguments" << endl;
        for (int i = 0; i < args_num; i++) {
            free(args[i]);
        }
        return;
    }
    // Get the new mode and file path from the arguments
    new_mode_Str = args[1];
    file_path = args[2];

    // Free the allocated memory for the arguments
    for (int i = 0; i < args_num; i++) {
        free(args[i]);
    }
}

bool isNumber(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

void ChmodCommand::execute() {
    // Check if the mode provided is a number
    if (!isNumber(new_mode_Str)) {
        cerr << "smash error: chmod: invalid arguments" << endl;
        return;
    }

    // Convert new_mode_Str to an integer
    int new_mode = stoi(new_mode_Str, nullptr, 8);

    // Use the chmod system call to change the file mode
    if (chmod(file_path.c_str(), new_mode) == -1) {
        perror("smash error: chmod failed");
    }
}

SetcoreCommand::SetcoreCommand(const char* cmd_line, vector<string>& args,int argsSize) : BuiltInCommand(cmd_line), args(args),argsSize(argsSize) {}

void SetcoreCommand::execute() {
    if(argsSize != 3) {
        cerr << "smash error: setcore: invalid arguments" << std::endl;
        return;
    }




    if(argsSize == 3) {

        for (unsigned int i = 0; i < args[1].size(); i++) {
            if (!isdigit(args[1][i])) {
                cerr << "smash error: fg: invalid arguments" << endl;
                return;
            }
        }
        for (unsigned int i = 0; i < args[2].size(); i++) {
            if (!isdigit(args[2][i])) {
                cerr << "smash error: fg: invalid arguments" << endl;
                return;
            }
        }
    }


    int core_num = stoi(args[2]);
    int job_id =  stoi(args[1]);

    auto myJob = SmallShell::getInstance().jobs_list->getJobById(job_id);
    if(myJob == NULL) {
        std::cerr << "smash error: setcore: job-id " << job_id << " does not exist" << std::endl;
        return;
    }

    cpu_set_t core;
    CPU_ZERO(&core);
    CPU_SET(core_num,&core);
    int check = sched_setaffinity(myJob->jobPid,sizeof(core),&core);
    if(check == ENAVAIL){
        std::cerr<<"smash error: setcore: invalid core number"<<std::endl;
    }
    else if(check <0){
        perror("smash error: setcore failed");
        return;
    }


}
bool IsValidUnixFilePathSyntax(const std::string& filePath) {
    // Check for empty file path
    if (filePath.empty()) {
        return false;
    }

    // Check for absolute path starting with '/'


    // Check for invalid characters
    const char* invalidChars = "\0";
    if (strpbrk(filePath.c_str(), invalidChars) != nullptr) {
        return false;
    }






    return true;
}
std::string GetFileTypeAux(const std::string& filePath) {
    struct stat fileStat;
    if (lstat(filePath.c_str(), &fileStat) == 0) {
        if (S_ISREG(fileStat.st_mode)) {
            return "regular file";
        } else if (S_ISSOCK(fileStat.st_mode)) {
            return "socket";
        } else if (S_ISFIFO(fileStat.st_mode)) {
            return "FIFO";
        } else if (S_ISDIR(fileStat.st_mode)) {
            return "directory";
        } else if (S_ISBLK(fileStat.st_mode)) {
            return "block device";
        } else if (S_ISLNK(fileStat.st_mode)) {
            return "symbolic link";
        } else if (S_ISCHR(fileStat.st_mode)) {
            return "character device";
        }
    }
    return "Unknown";
}
long long GetFileSize(const std::string& filePath) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        return static_cast<long long>(fileStat.st_size);
    }
    return -1;
}

GetFileTypeCommand::GetFileTypeCommand(const char *cmd_line, vector<string> &args, int argsSize): BuiltInCommand(cmd_line),args(args),argsSize(argsSize) {}

void GetFileTypeCommand::execute() {
    if(argsSize != 2){
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    if(!IsValidUnixFilePathSyntax(args[1])){
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    auto size = GetFileSize(args[1]);
    auto fileType = GetFileTypeAux(args[1]);
    if(size == -1 || fileType == "Unknown"){
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    cout<< args[1] << "'s" << " type is \"" <<  fileType << "\" and takes up " <<  size << " bytes"<< endl;


}

Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
    jobs_list->removeFinishedJobs();
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    vector<string> myCMD;
    string copy(cmd_line);
    // remove background sign from cmd_line
    string CheckRedirection = ">>";
    string Checker(cmd_line);
    int args1 = std::count(Checker.begin(),Checker.end(),'>');
    int args2 = 0;
     size_t i =0;
    int args3 = count(Checker.begin(),Checker.end(),'|');
    while((i = Checker.find(CheckRedirection,i)) != std::string::npos){
        ++args2;
        i +=CheckRedirection.length();
    }

    // split the cmd_line argument into individual words and store them in the myCMD array. The number of words is returned and stored in argsSize.
    int argsSize = newParseCommand(string(cmd_line), myCMD);
    if(args1 == 2 && args2 == 1){
        auto begin = myCMD[0].find(">>");
        auto end = myCMD[argsSize-1].find(">>");
        if(begin != 0 && end != myCMD[argsSize-1].length()-2){
            Command* cmd = new RedirectionCommand(copy.c_str());
            return cmd;
        }
    }
    if(args1 == 1 && args2 == 0){
        auto begin = myCMD[0].find(">");
        auto end = myCMD[argsSize-1].find(">");
        if(begin != 0 && end != myCMD[argsSize-1].length()-1){
            Command* cmd = new RedirectionCommand(copy.c_str());
            return cmd;
        }
    }
    if( args3 == 1){

        auto begin = myCMD[0].find("|");
        auto end = myCMD[argsSize-1].find("|");
        if(begin != 0 && end != myCMD[argsSize-1].length()-1){
            Command* cmd = new PipeCommand(copy.c_str());
            return cmd;
        }

    }// check if the conversion is valid!
    if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line,myCMD , argsSize);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line, myCMD, argsSize);
    }
    else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line, myCMD, argsSize);
    }
    else if(firstWord.compare("fg") == 0){
        return new ForegroundCommand(cmd_line, myCMD, argsSize);
    }
    else if(firstWord.compare("bg") == 0){
        return new BackgroundCommand(cmd_line,myCMD,argsSize);
    }
    else if(firstWord.compare("quit")==0){
        return new QuitCommand(cmd_line,myCMD,argsSize);
    }
    else if(firstWord.compare("setcore")==0){
        return new SetcoreCommand(cmd_line,myCMD,argsSize);
    }
    else if(firstWord.compare("getfiletype")==0){
        return new GetFileTypeCommand(cmd_line,myCMD,argsSize);
    }
    else if(firstWord.compare("chmod") == 0){
    return new ChmodCommand(cmd_line);
}
    else{
        Command *cmd = new ExternalCommand(cmd_line);
        return cmd;
    }


    /*
    if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

//Implementation of the JobsList methods
JobsList::JobEntry::JobEntry(int jobId, string cmd_line, pid_t pid, time_t insertionTime, bool isStopped) :
        job_id(jobId), cmd_line(cmd_line), jobPid(pid), insertion_time(insertionTime), is_stopped(isStopped) {}

JobsList::JobsList() : max_job_id(0),num_of_stopped_jobs(0) {}
void JobsList::killAllJobs() {
    if(jobs_entry_vec.size()==0){
        return;
    }
    for(auto i = jobs_entry_vec.begin(); i != jobs_entry_vec.end();++i){
        if(kill(i->jobPid,SIGKILL) == -1){
            perror("smash error: kill failed");
            return;
        }
    }
}
void JobsList::printJobsList() {
    removeFinishedJobs();
    if(jobs_entry_vec.size() == 0){
        return;
    }
    time_t now;
    time(&now); //get the current time
    for (auto const &job: jobs_entry_vec) {
        int elapsed_seconds = difftime(now, job.insertion_time); //calculate elapsed time since insertion
        cout << "[" << job.job_id << "] " << job.cmd_line << " : " << job.jobPid << " " << elapsed_seconds << " secs";
        if (job.is_stopped) {
            cout << " (stopped)";
        }
        cout << endl;
    }
}

void JobsList::removeFinishedJobs() {
    if(jobs_entry_vec.size() == 0){
        return;
    }
    for(unsigned int i =0 ; i<jobs_entry_vec.size() ; i++){
        int CheckPid = waitpid(jobs_entry_vec[i].jobPid,NULL,WNOHANG);
        if(CheckPid > 0){
            jobs_entry_vec.erase(jobs_entry_vec.begin()+i);
            i--;
        }
    }
    // Iterate through the jobs vector and remove any finished jobs
    /*for (auto it = jobs_entry_vec.begin(); it < jobs_entry_vec.end();++it) {
        // Get a pointer to the current job
        int status = 0;
        pid_t result = waitpid(it->jobPid, &status, WNOHANG | WUNTRACED | WCONTINUED);
      // Check if the process has finished
        if (result == it->jobPid && !WIFCONTINUED(status) && !WIFSTOPPED(status)) { // If the process has finished or killed
            it = jobs_entry_vec.erase(it); // Remove the job from the vector
           // Delete the job from memory
        }

    }*/
    if(jobs_entry_vec.size()== 0){
        max_job_id = 0;
    }else{
        auto Last2 = jobs_entry_vec.end();
        Last2--;
        max_job_id = Last2->job_id;
    }

}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    for (auto &job: jobs_entry_vec) {
        if(job.job_id == jobId) {
            return &job;
        }
    }
    return NULL;
}
void JobsList::addJob(Command* CMD,pid_t pid, bool isStopped) {
    removeFinishedJobs();
    time_t now = time(nullptr);
    if (now == (time_t) - 1) { // if 'time' function fails it returns the special value '(time_t)-1'
        perror("smash error: time failed");
        return;
    }
    if(jobs_entry_vec.size() == 0){
        max_job_id=0;
    }



    //const char* cmd_char = cmd_string.c_str(); // c_str() turns string to const char*
    // command line is empty
    JobsList::JobEntry new_job(max_job_id + 1, CMD->getCmd(), pid, now, isStopped);
    jobs_entry_vec.push_back(new_job); // push the new job to the jobs vector
    auto pointer = jobs_entry_vec.end();
    pointer--;

    max_job_id = pointer->job_id;
}
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    if(jobs_entry_vec.empty()){
        return NULL;
    }
    auto last = jobs_entry_vec.end();
    last--;
    for(; last>= jobs_entry_vec.begin();last--){
        if(last->is_stopped){
            return &*last;
        }
    }
    return NULL;
}