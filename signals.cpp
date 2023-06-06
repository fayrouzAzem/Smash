#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout<<"smash: got ctrl-Z"<<endl;


    int pid=SmallShell::getInstance().getCurrentJobPid();


    const char* currentCommand=SmallShell::getInstance().getCurrentJobCommand().c_str();
    if(pid==-1)
    {
        return;
    }


    if(kill(pid,SIGSTOP) == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    cout<< "smash: process "<< pid <<" was stopped"<<endl;
    auto start = SmallShell::getInstance().jobs_list->jobs_entry_vec.begin();
    auto end = SmallShell::getInstance().jobs_list->jobs_entry_vec.end();
    auto length = SmallShell::getInstance().jobs_list->jobs_entry_vec.size();
    for (auto p = start;
         p < end &&
         (length != 0); p++) {
        if (p->jobPid == pid) {
            p->is_stopped = true;
            p->insertion_time = time(NULL);

            if (p->insertion_time == -1) {
                perror("smash error: time failed");
                return;
            }

            SmallShell::getInstance().setCurrJobId(-1);
            SmallShell::getInstance().setCurrJobPid(-1);
            SmallShell::getInstance().setCurrjobCommand("");
            return;
        }
    }
        Command * command=new ExternalCommand(currentCommand);
    SmallShell::getInstance().jobs_list->addJob(command,pid,true);
    SmallShell::getInstance().setCurrJobId(-1);
    SmallShell::getInstance().setCurrJobPid(-1);
    SmallShell::getInstance().setCurrjobCommand("");


}

void ctrlCHandler(int sig_num) {
    cout<<"smash: got ctrl-C"<<endl;
    int pidId=SmallShell::getInstance().getCurrentJobPid();

    if(pidId>0)
    {
        if(kill(pidId,SIGKILL)==-1)

        {
            perror("smash error: kill failed");
        }
        else
        {
            cout<<"smash: process "<< pidId<<" was killed"<<endl;
        }
        auto start = SmallShell::getInstance().jobs_list->jobs_entry_vec.begin();
        auto end = SmallShell::getInstance().jobs_list->jobs_entry_vec.end();
        auto length = SmallShell::getInstance().jobs_list->jobs_entry_vec.size();
        for (auto p = start;
             p < end &&
             (length != 0); p++) {
        if(p->jobPid == pidId){
            SmallShell::getInstance().jobs_list->jobs_entry_vec.erase(p);
            auto last = SmallShell::getInstance().jobs_list->jobs_entry_vec.end();

            if(SmallShell::getInstance().jobs_list->jobs_entry_vec.size() != 0)
            {
                last --;
                SmallShell::getInstance().jobs_list->max_job_id = last->job_id;
            }
            else{
                SmallShell::getInstance().jobs_list->max_job_id =0;
            }
        }
        }
        SmallShell::getInstance().setCurrJobId(-1);
        SmallShell::getInstance().setCurrjobCommand("");
        SmallShell::getInstance().setCurrJobPid(-1);
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}