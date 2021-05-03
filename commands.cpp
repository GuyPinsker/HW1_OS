//		commands.c
//********************************************
#include "commands.h"
#include <list>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
//********************************************

// Name Spaces 
using namespace std;

// Local Constants 
#define NUL '\0'
enum {MAX_HIST_NUM = 50, CHUNK = 100};

// Local classes and Functions
class job
{
	public:
		int ser_num;
		char cmd[MAX_LINE_SIZE];
		int pid;
		int entry_time;
		bool stopped;

		job()
		{
			ser_num = 0;
			cmd[0] = NUL;
			pid = 0;
			entry_time = 0;
			stopped = false;
		}
};

int get_job_pid(list<job> jobs, int id)
{
	for (auto& iter : jobs)
	{
		if (iter.ser_num == id)
		{
			return iter.pid;
		}
	}

	return -1;
}



// Local Variables
char last_cwd[PATH_MAX] = { NUL };
char new_last_cwd[PATH_MAX];
list<char*> hist;
list<job> jobs;

//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************
int ExeCmd(void* jobs, char* lineSize, char* cmdString)
{
	char* cmd; 
	char* args[MAX_ARG];
	char pwd[MAX_LINE_SIZE];
	char* delimiters = " \t\n";  
	int i = 0, num_arg = 0;
	bool illegal_cmd = FALSE; // illegal command
    	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0; 
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters); 
		if (args[i] != NULL) 
			num_arg++; 
 
	}
	
	if (strcmp(cmd, "history"))	// If CMD=history don't insert
	{
		hist.push_back(cmdString);
	}
	if (hist.size() > MAX_HIST_NUM)	//If num of CMD in hist > 50 then pop.
	{
		hist.pop_front();
	}
/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd, "cd") ) 
	{				
		if (num_arg > 2)
		{
			illegal_cmd = TRUE;
		}
		
		else 
		{ 
			if (getcwd(new_last_cwd, PATH_MAX) == NULL) // save current location
			{
				perror("smash error");
			}

			else
			{
				if (!strcmp(args[1], "-")) // if ask to go to last cwd
				{
					if (last_cwd[0] != NUL) // if this is not the first request (we've been to another place)
					{
						chdir(last_cwd);							 // change to last location
						strcpy(last_cwd, new_last_cwd);				 // update last cwd
					}
				}

				else 
				{
					if (0 != chdir(args[1]))
					{
						perror("smash error");
					}

					else
					{
						strcpy(last_cwd, new_last_cwd);				 // update last cwd
					}
				}
			}
		}
	} 	
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		if (num_arg > 1)
		{
			illegal_cmd = TRUE;
		}
		else
		{
			char wd[PATH_MAX];
			if (getcwd(wd, PATH_MAX) == NULL)
			{
				perror("smash error");
			}
			else
				cout << wd << endl;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "jobs")) 
	{
		if (num_arg > 1)
		{
			illegal_cmd = TRUE;
		}
		
		for (auto& iter : jobs)
		{
			int current_time = time(NULL);
			if (iter.stopped)
			{
				cout << "[" << iter.ser_num << "] " << iter.cmd << " : " << iter.pid << " " << current_time - iter.entry_time << " secs (stopped)" << endl;
			}
			else
			{
				cout << "[" << iter.ser_num << "] " << iter.cmd << " : " << iter.pid << " " << current_time - iter.entry_time << " secs" << endl;
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid")) 
	{
		if (num_arg > 1)
		{
			illegal_cmd = TRUE;
		}
		else
			cout << "smash pid is " << getpid() << endl;
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg")) 
	{
		if (num_arg = 1)
		{
			int pid = jobs.back().pid;
			kill(pid, SIGCONT)
		}
		
		else if (num_arg = 2)
		{
			int pid = get_job_pid(jobs, args[1]);
		}

		else
		{
			illegal_cmd = TRUE;
		}
	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
  		
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
   		
	} 
	/*************************************************/
	else if (!strcmp(cmd, "history"))
	{
   		if (num_arg > 1)
		{
			illegal_cmd = TRUE;
		}
		else
			for (auto& iter : hist) cout << iter << endl;
	} 
	/*************************************************/
	else if (!strcmp(cmd, "diff"))
	{
		if (num_arg != 3)
		{
			illegal_cmd = TRUE;
		}

		else
		{
			int fd1 = open(args[1], O_APPEND);
			int fd2 = open(args[2], O_APPEND);
			if (fd1 < 0 || fd2 < 0)
			{
				perror("smash error"); //  bad path/s
			}

			else // good paths
			{
				char text2[CHUNK];
				char text1[CHUNK];
				int bytes1 = 1;
				int bytes2 = 1;
				int res = 0;

				while (bytes1 > 0 && bytes2 > 0)
				{
					bytes1 = read(fd1, text1, CHUNK);
					bytes2 = read(fd2, text2, CHUNK);
					if (bytes1 < 0 || bytes2 < 0)
					{
						perror("smash error"); //  bad reads
						res = -1;
						break;
					}

					if ((string)text1 != (string)text2)
					{
						res = 1;
						break;
					}

				}

				if (res >= 0)
				{
					cout << res << endl;
				}
			}
		}

	}
	/*************************************************/
	else if (!strcmp(cmd, "cp"))
	{
		if (num_arg != 3)
		{
			illegal_cmd = TRUE;
		}

		else
		{
			int src_fd = open(args[1], O_APPEND);
			if (src_fd < 0)
			{
				perror("smash error"); // src file does not exist
			}

			int dst_fd = creat(args[2], S_IRWXU | S_IRWXG);
			else if (dst_fd < 0)
			{
				perror("smash error"); // dst file does not exist/cannot be opened
			}
			
			else
			{

				char src_text[CHUNK];
				int rd_bytes = 1;
				int wr_bytes;

				while (rd_bytes > 0)
				{
					rd_bytes = read(src_fd, src_text, CHUNK);
					if (rd_bytes < 0)
					{
						perror("smash error"); //  bad read
						break;
					}

					wr_bytes = write(dst_fd, src_text, rd_bytes);
					if (wr_bytes < 0 || wr_bytes != rd_bytes)
					{
						perror("smash error"); // bad write
						break;
					}
				}
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		if (num_arg != 3)
		{
			illegal_cmd = TRUE;
		}
		int job_pid = get_job_pid(jobs, args[2]);
		if (job_pid == -1)
		{
			cout << "smash error: > kill " << args[2] << "– job does not exist" << endl;
		}
		int signum = -1 * (int)args[1];
		if (kill(job_pid, signum) != 0)
		{
			cout << "smash error: > kill " << args[2] << "– cannot send signal" << endl;
		}
	}
	/*************************************************/

	else // external command
	{
 		ExeExternal(args, cmdString);
	 	return 0;
	}
	
	if (illegal_cmd == TRUE)
	{
		printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmdString)
{
	int pID;
    	switch(pID = fork()) 
	{
    		case -1: 
					// Add your code here (error)
					perror("smash error");
        	case 0 :
                	// Child Process
               		setpgrp();
					
			        // Add your code here (execute an external command)
					
					execv(cmdString, args);
			
			default:
                	// Add your code here
					
					waitpid(pID, NULL, 0);
	}
}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize)
{
	char ExtCmd[MAX_LINE_SIZE+2];
	char *args[MAX_ARG];
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
		// Add your code here (execute a complicated command)
					
		/* 
		your code
		*/
	} 
	return -1;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, void* jobs)
{

	char* Command;
	char* delimiters = " \t\n";
	char *args[MAX_ARG];
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = NUL;
		// Add your code here (execute a in the background)
		
		char* cmd;
		int i = 0, num_arg = 0;
		cmd = strtok(lineSize, delimiters);
		if (cmd == NULL)
			return 0;
		args[0] = cmd;
		for (i = 1; i < MAX_ARG; i++)
		{
			args[i] = strtok(NULL, delimiters);
			if (args[i] != NULL)
				num_arg++;

		}

		pid_t cpid = fork();

		if (cpid == 0)	// child proc to run in bg
		{

		}
		return 0;
	}
	return -1;
}

