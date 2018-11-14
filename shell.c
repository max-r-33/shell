#include <signal.h>
#include <stdio.h>
#include "parser/ast.h"
#include "shell.h"

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "msh ➜ ";
}

void run_command(node_t *node)
{
    /* For testing: */
    // print_tree(node);
    
    if (prompt)
        prompt = "msh ➜ ";
    
    if(node->type == NODE_COMMAND) {
      char *program = node->command.program;
      char **argv = node->command.argv;
      int status;
      int pid = fork();

      // printf("program = %s\nargv[0] = %s\nargv[1] = %s\n", program, argv[0], argv[1]);
      // printf("PID = %i", pid);

      if(strcmp(argv[0],"exit") == 0) {
        // printf("HELLO");
        exit(0);
      } else if(pid != 0) {
        // printf("waitpid\n");
        waitpid(-1, &status, 0);
      } else {
        // printf("execve\n");
        execvp(program, argv, 0);
      }
    }
}
