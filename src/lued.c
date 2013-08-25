/* Copyright (C) 2013 Calvin Beck

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>


void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <program> [args]\n", program_name);
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Too few arguments!\n");
	usage(argv[0]);

	return -1;
    }

    /* Need to fork - parent process traces, child is traced */
    pid_t pid = fork();

    if (0 == pid) {
	/* Child process asks to be traced, and execs the desired program. */
	ptrace(PT_TRACE_ME, pid, NULL, 0);
	execvp(argv[1], &argv[1]);

	fprintf(stderr, "Execution of \"%s\" failed!\n", argv[1]);
	return -1;
    }

    int wait_status;

    while (1) {
	int wait_pid = wait(&wait_status);

	if (wait_pid == pid) {
	    if (WIFSTOPPED(wait_status)) {
		/* Just step again */
		ptrace(PT_STEP, pid, NULL, 0);
	    }
	    else if (WIFEXITED(wait_status)) {
		printf("Process \"%s\" exited normally.\n", argv[1]);
		return 0;
	    }
	    else {
		fprintf(stderr, "Something strange happened: %X\n", wait_status);
		return -1;
	    }
	}
    }

    return 0;
}
