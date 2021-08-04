#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int runCommand(const char *cmd, char *out, int outlen, char *err, int errlen) {

	int opipe[2], epipe[2];
	pid_t pid;

	if (pipe(opipe) < 0) {
		perror("pipe error");
		return -1;
	}

	if (pipe(epipe) < 0) {
		perror("pipe error");
		return -1;
	}

        if ((pid = fork()) < 0) {
                perror("fork error");
		return -1;
        } else if (pid == 0) {
		close(opipe[0]);
		close(epipe[0]);
		if (dup2(opipe[1], STDOUT_FILENO) < 0) {
			perror("dup2 stdout");
			return -1;
		}
		if (dup2(epipe[1], STDERR_FILENO) < 0) {
			perror("dup2 stdout");
			return -1;
		}
		execlp("/bin/sh", "runcommand", "-c", cmd, (char *) 0);
		return -1;
	} else {
		size_t n;
		char buf[BUFSIZ];
		int total = 0;
		int i;
		close(opipe[1]);
		close(epipe[1]);
		if ((pid = wait(&i)) < 0) {
			perror("wait");
			return -1;
		}
		while ((n = read(epipe[0], buf, BUFSIZ)) > 0) {
			total += n;
			if (total <= errlen) {
				strlcat(err, buf, errlen);
			}
		}
		while ((n = read(opipe[0], buf, BUFSIZ)) > 0) {
			total += n;
			if (total <= outlen) {
				strlcat(out, buf, outlen);
			}
			bzero(buf, BUFSIZ);
		}
	}
	return 0;
}

int
main(int argc, char **argv) {
	char out[BUFSIZ], err[BUFSIZ];
	runCommand("ls -l", out, BUFSIZ, err, BUFSIZ);
	printf("stdout:\n%s", out);
	printf("\n\nstderr:\n%s", err);
}
