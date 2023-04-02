#include <neonix/assert.h>
#include <neonix/fs.h>
#include <neonix/stat.h>
#include <neonix/stdio.h>
#include <neonix/stdlib.h>
#include <neonix/string.h>
#include <neonix/syscall.h>
#include <neonix/time.h>

#define MAX_CMD_LEN 256
#define MAX_ARG_NR 16
#define MAX_PATH_LEN 1024
#define BUFLEN 1024

static char cwd[MAX_PATH_LEN];
static char cmd[MAX_CMD_LEN];
static char *args[MAX_ARG_NR];
static char buf[BUFLEN];

static char *envp[] = {
    "HOME=/",
    "PATH=/bin",
    NULL,
};

static char *neonix_logo[] = {
    "\033[0m\t               \033[34m       \033[32m      \033[35m       \033[33m   \033[32m _  \033[35m     \n\0",
    "\033[0m\t               \033[34m _ __  \033[32m ___  \033[35m___  \033[33m_ __ \033[32m(_)\033[35m _  __\n\0",
    "\033[0m\t               \033[34m| '_ \\ \033[32m/ _ \\\033[35m/ _ \\\033[33m| '_ \\\033[32m| |\033[35m\\ \\/ /\n\0",
    "\033[0m\t               \033[34m| | | | \033[32m __/ \033[35m(_) | \033[33m| | \033[32m| | \033[35m>  < \n\0",
    "\033[0m\t               \033[34m|_| |_|\033[32m\\___|\033[35m\\___/\033[33m|_| |_\033[32m|_|\033[35m/_/\\_\\\n\0",
};

extern char *strsep(const char *str);
extern char *strrsep(const char *str);

char *basename(char *name)
{
  char *ptr = strrsep(name);
  if (!ptr[1])
  {
    return ptr;
  }
  ptr++;
  return ptr;
}

void print_prompt()
{
  getcwd(cwd, MAX_PATH_LEN);
  char *ptr = strrsep(cwd);
  if (ptr != cwd)
  {
    *ptr = 0;
  }
  char *base = basename(cwd);
  printf("\033[30;45m[root %s]\033[0m# ", base);
}

void clear()
{
  printf("\x1b[2J\x1b[0;0H");
}

void builtin_logo()
{
  clear();
  for (size_t i = 0; i < 5; i++)
  {
    printf(neonix_logo[i]);
  }
}

void builtin_test(int argc, char *argv[]) {}

void builtin_pwd()
{
  getcwd(cwd, MAX_PATH_LEN);
  printf("%s\n", cwd);
}

void builtin_clear()
{
  clear();
}

static void strftime(time_t stamp, char *buf)
{
  tm time;
  localtime(stamp, &time);
  sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", time.tm_year + 1900, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
}

void builtin_cd(int argc, char *argv[])
{
  chdir(argv[1]);
}

void builtin_mkdir(int argc, char *argv[])
{
  if (argc < 2)
  {
    return;
  }
  mkdir(argv[1], 0755);
}

void builtin_rmdir(int argc, char *argv[])
{
  if (argc < 2)
  {
    return;
  }
  rmdir(argv[1]);
}

void builtin_rm(int argc, char *argv[])
{
  if (argc < 2)
  {
    return;
  }
  unlink(argv[1]);
}

void builtin_date(int argc, char *argv[])
{
  strftime(time(), buf);
  printf("%s\n", buf);
}

void builtin_mount(int argc, char *argv[])
{
  if (argc < 3)
  {
    return;
  }
  mount(argv[1], argv[2], 0);
}

void builtin_umount(int argc, char *argv[])
{
  if (argc < 2)
  {
    return;
  }
  umount(argv[1]);
}

void builtin_mkfs(int argc, char *argv[])
{
  if (argc < 2)
  {
    return;
  }
  mkfs(argv[1], 0);
}

static int dupfile(int argc, char **argv, fd_t dupfd[3])
{
  for (size_t i = 0; i < 3; i++)
  {
    dupfd[i] = EOF;
  }

  int outappend = 0;
  int errappend = 0;

  char *infile = NULL;
  char *outfile = NULL;
  char *errfile = NULL;

  for (size_t i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], "<") && (i + 1) < argc)
    {
      infile = argv[i + 1];
      argv[i] = NULL;
      i++;
      continue;
    }
    if (!strcmp(argv[i], ">") && (i + 1) < argc)
    {
      outfile = argv[i + 1];
      argv[i] = NULL;
      i++;
      continue;
    }
    if (!strcmp(argv[i], ">>") && (i + 1) < argc)
    {
      outfile = argv[i + 1];
      argv[i] = NULL;
      outappend = O_APPEND;
      i++;
      continue;
    }
    if (!strcmp(argv[i], "2>") && (i + 1) < argc)
    {
      errfile = argv[i + 1];
      argv[i] = NULL;
      i++;
      continue;
    }
    if (!strcmp(argv[i], "2>>") && (i + 1) < argc)
    {
      errfile = argv[i + 1];
      argv[i] = NULL;
      errappend = O_APPEND;
      i++;
      continue;
    }
  }

  if (infile != NULL)
  {
    fd_t fd = open(infile, O_RDONLY | outappend | O_CREAT, 0755);
    if (fd == EOF)
    {
      printf("open file %s failure\n", infile);
      goto rollback;
    }
    dupfd[0] = fd;
  }
  if (outfile != NULL)
  {
    fd_t fd = open(outfile, O_WRONLY | outappend | O_CREAT, 0755);
    if (fd == EOF)
    {
      printf("open file %s failure\n", outfile);
      goto rollback;
    }
    dupfd[1] = fd;
  }
  if (errfile != NULL)
  {
    fd_t fd = open(errfile, O_WRONLY | errappend | O_CREAT, 0755);
    if (fd == EOF)
    {
      printf("open file %s failure\n", errfile);
      goto rollback;
    }
    dupfd[2] = fd;
  }
  return 0;

rollback:
  for (size_t i = 0; i < 3; i++)
  {
    if (dupfd[i] != EOF)
    {
      close(dupfd[i]);
    }
  }
  return EOF;
}

pid_t builtin_command(char *filename, char *argv[], fd_t infd, fd_t outfd, fd_t errfd, pid_t *pgid)
{
  int status;

  pid_t pid = fork();
  if (pid)
  {
    if (infd != EOF)
    {
      close(infd);
    }
    if (outfd != EOF)
    {
      close(outfd);
    }
    if (errfd != EOF)
    {
      close(errfd);
    }
    if (*pgid == 0)
    {
      *pgid = pid;
    }
    return pid;
  }
  if (infd != EOF)
  {
    fd_t fd = dup2(infd, STDIN_FILENO);
    close(infd);
  }
  if (outfd != EOF)
  {
    fd_t fd = dup2(outfd, STDOUT_FILENO);
    close(outfd);
  }
  if (errfd != EOF)
  {
    fd_t fd = dup2(errfd, STDERR_FILENO);
    close(errfd);
  }

  // 设置进程组 pgid
  setpgid(0, *pgid);

  int i = execve(filename, argv, envp);
  exit(i);
}

void builtin_exec(int argc, char *argv[])
{
  bool p = true;
  int status;

  char **bargv = NULL;
  char *name = buf;

  fd_t dupfd[3];
  if (dupfile(argc, argv, dupfd) == EOF)
    return;

  fd_t infd = dupfd[0];
  fd_t pipefd[2];
  int count = 0;
  pid_t pgid = 0;

  for (int i = 0; i < argc; i++)
  {
    if (!argv[i])
    {
      continue;
    }
    if (!p && !strcmp(argv[i], "|"))
    {
      argv[i] = NULL;
      int ret = pipe(pipefd);
      builtin_command(name, bargv, infd, pipefd[1], EOF, &pgid);
      count++;
      infd = pipefd[0];
      int len = strlen(name) + 1;
      name += len;
      p = true;
      continue;
    }
    if (!p)
    {
      continue;
    }

    stat_t statbuf;
    sprintf(name, "/bin/%s.out", argv[i]);
    if (stat(name, &statbuf) == EOF)
    {
      printf("osh: command not found: %s\n", argv[i]);
      return;
    }
    bargv = &argv[i + 1];
    p = false;
  }

  int pid = builtin_command(name, bargv, infd, dupfd[1], dupfd[2], &pgid);
  for (size_t i = 0; i <= count; i++)
  {
    pid_t child = waitpid(-1, &status);
    // printf("child %d exit\n", child);
  }
}

static void execute(int argc, char *argv[])
{
  char *line = argv[0];
  if (!strcmp(line, "test"))
  {
    return builtin_test(argc, argv);
  }
  if (!strcmp(line, "logo"))
  {
    return builtin_logo();
  }
  if (!strcmp(line, "pwd"))
  {
    return builtin_pwd();
  }
  if (!strcmp(line, "clear"))
  {
    return builtin_clear();
  }
  if (!strcmp(line, "exit"))
  {
    int code = 0;
    if (argc == 2)
    {
      code = atoi(argv[1]);
    }
    exit(code);
  }
  if (!strcmp(line, "cd"))
  {
    return builtin_cd(argc, argv);
  }
  if (!strcmp(line, "mkdir"))
  {
    return builtin_mkdir(argc, argv);
  }
  if (!strcmp(line, "rmdir"))
  {
    return builtin_rmdir(argc, argv);
  }
  if (!strcmp(line, "rm"))
  {
    return builtin_rm(argc, argv);
  }
  if (!strcmp(line, "date"))
  {
    return builtin_date(argc, argv);
  }
  if (!strcmp(line, "mount"))
  {
    return builtin_mount(argc, argv);
  }
  if (!strcmp(line, "umount"))
  {
    return builtin_umount(argc, argv);
  }
  if (!strcmp(line, "mkfs"))
  {
    return builtin_mkfs(argc, argv);
  }

  return builtin_exec(argc, argv);
}

void readline(char *buf, u32 count)
{
  assert(buf != NULL);
  char *ptr = buf;
  u32 idx = 0;
  char ch = 0;
  while (idx < count)
  {
    ptr = buf + idx;
    int ret = read(STDIN_FILENO, ptr, 1);
    if (ret == -1)
    {
      *ptr = 0;
      return;
    }
    switch (*ptr)
    {
    case '\n':
    case '\r':
      *ptr = 0;
      ch = '\n';
      write(STDOUT_FILENO, &ch, 1);
      return;
    case '\b':
      if (buf[0] != '\b')
      {
        idx--;
        ch = '\b';
        write(STDOUT_FILENO, &ch, 1);
      }
      break;
    case '\t':
      continue;
    default:
      write(STDOUT_FILENO, ptr, 1);
      idx++;
      break;
    }
  }
  buf[idx] = '\0';
}

static int cmd_parse(char *cmd, char *argv[])
{
  assert(cmd != NULL);

  char *next = cmd;
  int argc = 0;
  int quot = false;
  while (*next && argc < MAX_ARG_NR)
  {
    while (*next == ' ' || (quot && *next != '"'))
    {
      next++;
    }
    if (*next == 0)
    {
      break;
    }

    if (*next == '"')
    {
      quot = !quot;

      if (quot)
      {
        next++;
        argv[argc++] = next;
      }
      else
      {
        *next = 0;
        next++;
      }
      continue;
    }

    argv[argc++] = next;
    while (*next && *next != ' ')
    {
      next++;
    }

    if (*next)
    {
      *next = 0;
      next++;
    }
  }

  argv[argc] = NULL;
  return argc;
}

int main()
{
  setsid();
  memset(cmd, 0, sizeof(cmd));
  memset(cwd, 0, sizeof(cwd));

  clear();
  // builtin_logo();

  while (true)
  {
    print_prompt();
    readline(cmd, sizeof(cmd));
    if (cmd[0] == 0)
    {
      continue;
    }
    int argc = cmd_parse(cmd, args);
    if (argc < 0 || argc >= MAX_ARG_NR)
    {
      continue;
    }
    execute(argc, args);
  }
  return 0;
}
