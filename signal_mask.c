/* CMPS2300 signal masking demo  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes */
void usage(void);
void unix_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
void print_sigset_t(sigset_t *set);

int main(int argc, char **argv) 
{
    char c; /* to store command line option below */
    int sleepytime = 10; /* default is to sleep 10 seconds unless specified */
    int aftertime = 120; /* default is to stay in loop for 120 seconds unless specified */
    int block_sigint, block_sigtstp, block_sigchld;
    block_sigint = block_sigtstp = block_sigchld = 0; /* all four will default to zero/false */
    sigset_t mask;       /* this will be our signal mask */

    /* Parse command line arguments with getopt */
    while ((c = getopt(argc, argv, "hs:a:itc")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
	case 's':             /* how many seconds to sleep before masking */
            sleepytime = strtol(optarg, NULL, 10);
	    break;
	case 'a':
	    aftertime = strtol(optarg, NULL, 10);
	    break;
        case 'i':             /* block SIGINT */
            block_sigint = 1;
	    break;
        case 't':             /* block SIGTSTP */
	    block_sigtstp = 1;
	    break;
	case 'c':             /* block SIGCHLD */
	    block_sigchld = 1;
	    break;
	default:
            usage(); /* print usage message / help */
	}
    }

    printf("Starting PID %d with no signal mask...\nSleeping %d seconds...\n", getpid(), sleepytime);

    sleep(sleepytime);

    printf("Now setting signal mask with sigprocmask.\n");

	if (sigemptyset(&mask) < 0)
	    unix_error("sigemptyset error");
	if (sigaddset(&mask, SIGCHLD)) 
	    unix_error("sigaddset error");
	if (block_sigint && sigaddset(&mask, SIGINT)) 
	    unix_error("sigaddset error");
	if (block_sigtstp && sigaddset(&mask, SIGTSTP)) 
	    unix_error("sigaddset error");
	if (block_sigchld && sigaddset(&mask, SIGCHLD))
        unix_error("sigaddset error");
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
	    unix_error("sigprocmask error");

    printf("As of now, signals are blocked. Will now sleep for %d seconds. \n", aftertime);
//    print_sigset_t(&mask);

    while(aftertime > 0) {
	    sleep(1); /* do nothing */
	    aftertime--; /* until aftertime reaches 0 */
	}

	printf("Done. Exiting.\n");
	exit(0);
}

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: signal_mask [-hvp] [-s <seconds>]\n");
    printf("   -h             print this message\n");
    printf("   -s <seconds>   how many seconds to sleep before applying mask\n");
    printf("   -i             block SIGINT\n");
    printf("   -t             block SIGTSTP\n");
    printf("   -c             block SIGCHLD\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/* see what the sigset looks like */
void print_sigset_t(sigset_t *set)
{
	int i;

	i = SIGRTMAX;
	do {
		int x = 0;
		i -= 4;
		if (sigismember(set, i+1)) x |= 1;
		if (sigismember(set, i+2)) x |= 2;
		if (sigismember(set, i+3)) x |= 4;
		if (sigismember(set, i+4)) x |= 8;
		printf("%x", x);
	} while (i >= 4);
	printf("\n");
}
