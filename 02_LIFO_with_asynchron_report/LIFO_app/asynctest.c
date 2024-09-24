
// asynctest.c: use async notification to read stdin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int gotsignal=0;
int datacnt=0;
int end = 0;
void sighandler(int signo)
{
    if (signo==SIGIO)
    {
	gotsignal=1;
	datacnt++;
    }
    return;
}

char buffer[4096];
int main(int argc, char **argv)
{
    int count;
    struct sigaction action;
    int fd=open("/dev/lifo",O_RDONLY|O_NONBLOCK);
    if (!fd)
    {
	exit(1);
	printf("Error opening file\n");
    }
    // set memebrs of  action structure
    memset(&action, 0, sizeof(action)); // set all fileds of the structure to 0 
    action.sa_handler = sighandler; // sets which functions to executre when async is called 
    //action.sa_flags = 0;
    sigaction(SIGIO, &action, NULL);
    printf("pid of a current process is: %d\n", getpid());
    fcntl(fd, F_SETOWN, getpid()); // Sets information about process that need asynchronous notification
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);
    
    while(1) 
    {
  	sleep(8600);
	if (!gotsignal)
	    continue;

	fflush(stdout);
	gotsignal=0;
	count=read(fd, buffer, 4096);
	buffer[count]='\0';
	printf("Read: %s\n",buffer);
	count=read(fd, buffer, 4096);
    }
    close(fd);
}
