#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

// print string to stderr
void	err(char *s)
{
	while (*s)
		write(2, s++, 1);
}

// chage dir
int	cd(char **av, int i)
{
	if (i != 2)
		return err("error: cd: bad arguments\n"), 1;
	if (chdir(av[1]) == -1)
		return err("error: cd: cannot change directory to "), err(av[1]), err("\n"), 1;
	return 0;
}

// Function to set pipe
// end == 1 sets stdout to act as write end of our pipe
// end == 0 sets stdin to ac as read end of our pipe
void set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1 ))
		err("error: fatal\n"), exit(1);
}

// execute a comand
int exec(char **av, int i, char **envp)
{
	int has_pipe; 
	int fd[2];
	int pid; 
	int status;

	has_pipe = av[i] && !strcmp(av[i], "|");
	if (!has_pipe && !strcmp(*av, "cd"))
		return cd(av, i);
	if (has_pipe && pipe(fd) == -1)
		err("error: fatal\n"), exit(1); 
	if ((pid = fork()) == -1)
		err("error: fatal\n"), exit(1);
	if (!pid)
	{
		av[i] = 0;
		set_pipe(has_pipe, fd, 1);
		if (!strcmp(*av, "cd"))
			exit(cd(av, i));
		execve(*av, av, envp);
		err("error: cannot execute "), err(*av), err("\n"), exit(1);
	}
	waitpid(pid, &status, 0);
	set_pipe(has_pipe, fd, 0);
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int ac, char **av, char **envp)
{
	(void)ac;
	int i = 0;
	int status = 0;

	while (av[i])
	{
		av += i + 1;
		i = 0;
		while(av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;
		if (i)
			status = exec(av, i, envp);
	}
	return status;
}
