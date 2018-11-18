#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser/ast.h"
#include "shell.h"

void initialize(void) {
    /* This code will be called once at startup */
    if (prompt) {
        prompt = "$msh ➜ ";
    }
}

void run_command(node_t *node) {
    /* For testing: */
    print_tree(node);
    
    if (prompt) {
        prompt = "$msh ➜ ";
    }
    
    if(node->type == NODE_COMMAND) {
      char *program = node->command.program;
      char **argv = node->command.argv;
      int status;
      int pid = fork();
      // printf("program = %s\nargv[0] = %s\nargv[1] = %s\n", program, argv[0], argv[1]);

      if(strcmp(program, "exit") == 0) {
        argv[1] ? exit(atoi(argv[1])) : exit(3);
      } else if(pid != 0) {
        signal(SIGINT, SIG_IGN);
        waitpid(-1, &status, 0);
      } else {
        if(strcmp(program, "cd") == 0) {
          if(!argv[1]) {
            fprintf(stderr, "msh: expected argument to cd command\n");
          } else if(chdir(argv[1]) != 0) {
            perror("msh");
          }
        } else {
          signal(SIGINT, SIG_DFL);
          if(execvp(program, argv) != 0) {
            perror("msh");
          }
        }
      }
    } else if(node->type == NODE_SEQUENCE) {
      run_command(node->sequence.first);
      run_command(node->sequence.second);
    } else if(node->type == NODE_PIPE) {
      // pipe
    }
}
