#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser/ast.h"
#include "shell.h"

#define STDIN   0
#define STDOUT  1
#define PIPE_RD 0
#define PIPE_WR 1

void initialize(void) {
    /* This code will be called once at startup */
    if (prompt) {
        prompt = "$msh ➜ ";
    }
}

void run_command(node_t *node) {
    /* For testing: */
    // print_tree(node);
    
    if (prompt) {
        prompt = "$msh ➜ ";
    }
    
    if(node->type == NODE_COMMAND) {
      char *program = node->command.program;
      char **argv = node->command.argv;
      int status;
      
      if(strcmp(program, "exit") == 0) {
        argv[1] ? exit(atoi(argv[1])) : exit(3);
      } else if(strcmp(program, "cd") == 0) {
        if(chdir(argv[1]) != 0) {
          perror("cd");
        }
      } else if(strcmp(program, "set") == 0) {
        if(argv[1] && argv[2]) {
          setenv(argv[1], argv[2], 1);
          printf("%s=%s", argv[0], argv[1]);
        } else {
          perror("setenv");
        }
      } else if(strcmp(program, "unset") == 0) {
        if(argv[1]) {
          unsetenv(argv[1]);
          printf("%s=%s", argv[1], argv[1]);
        } else {
          perror("unsetenv");
        }
      } else {
        int pid = fork();
        if(pid != 0) {
          signal(SIGINT, SIG_IGN);
          waitpid(-1, &status, 0);
        } else {
          signal(SIGINT, SIG_DFL);
          // printf("program = %s\nargv[0] = %s\nargv[1] = %s\n", program, argv[0], argv[1]);
          if(execvp(program, argv) != 0) {
            perror("msh");
          }
        }
      }
    } else if(node->type == NODE_SEQUENCE) {
      run_command(node->sequence.first);
      run_command(node->sequence.second);
    } else if(node->type == NODE_PIPE) {
      node_t **argv = node->pipe.parts;
      char **firstArgs = argv[0]->command.argv;
      char **secondArgs = argv[1]->command.argv;
      char *first = argv[0]->command.program;
      char *second = argv[1]->command.program;
      
      // printf("executing %s\n", firstArgs[0]);
      // printf("executing %s\n", secondArgs[0]);
      // firstArgs[1] ? printf("first argument: %s", firstArgs[1]) : printf("");
      // secondArgs[1] ? printf("second argument: %s", secondArgs[1]) : printf("");
      
      pid_t pid;
      int fds[2];
      pipe(fds);
      pid = fork();
            
      if(pid == 0) {
          dup2(fds[PIPE_WR], STDOUT);
          close(fds[PIPE_RD]);
          close(fds[PIPE_WR]);
          execlp(first, first, firstArgs[1], firstArgs[2], (char*) NULL);
          exit(1);
      } else { 
          pid = fork();
          if(pid == 0) {
              dup2(fds[PIPE_RD], STDIN);
              close(fds[PIPE_WR]);
              close(fds[PIPE_RD]);
              execlp(second, second, secondArgs[1], secondArgs[2], (char*) NULL);
              exit(1);
          } else {
              close(fds[PIPE_RD]);
              close(fds[PIPE_WR]);
              waitpid(pid, NULL, 0);
          }
      }
    }
}
