
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments
typedef struct Node
  {
    int data;
    struct Node *next;
  } Node;

  typedef struct hist
  {
    char *ams;
    struct hist *next;
  } hist;


// the command written is stored in linked-list 
hist *fush (hist *head, char *cmds)
{
  hist *TempPtr, *NewNode;

  NewNode = (hist*) malloc(sizeof(hist));
  NewNode -> ams = (char*)malloc(sizeof(char *));
  strcpy(NewNode->ams,cmds);
  NewNode ->next = NULL;
  if (head == NULL)
  {
    head = NewNode;
  }

  else
  {
    TempPtr = head;
    //traversing the linked-list
    while(TempPtr->next != NULL)
    {
      TempPtr = TempPtr->next;
    }
    // adds the node when TempPtr->nex = NULL
    TempPtr->next = NewNode;
  }
  return head;
}

// the pids of child are stored in linked-list
Node * push (Node *LLH, pid_t data)
{
  Node *TempPtr, *NewNode;

  NewNode = malloc(sizeof (Node));
  NewNode->data = data;
  NewNode ->next = NULL;
  if (LLH == NULL)
  {
    LLH = NewNode;
  }
  else
  {
    TempPtr = LLH;
    //traversing the linked list
    while (TempPtr->next != NULL)
    {
      TempPtr = TempPtr->next;
    }

    TempPtr->next = NewNode; 
  }

  return LLH;
}

char *figure(hist *head, int num)
{
  int count = 0;
  hist *TempPtr = head;
  while(TempPtr != NULL)
  {
    if (count == num)
    {
      return TempPtr->ams;
    }
    count++;
    TempPtr = TempPtr->next;
  }

  if(num > count)
  {
    return NULL;
  }
   
   return NULL;
}

void displayHist(hist *head)
{
  int count = 0;
  hist *TempPtr = head;
  while(TempPtr != NULL)
  {
    printf("%d: %s", count, TempPtr->ams);
    count++;
    TempPtr = TempPtr->next;
  }
  return;
}

void displayPid(Node *LLH)
{
  int count = 0;
  Node *TempPtr = LLH;
  while(TempPtr != NULL)
  {
    printf("%d: %d\n",count, TempPtr->data);
    count++;
    TempPtr = TempPtr->next;
  }
  return;
}


int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  Node *LLH = NULL;
  hist *head = NULL;
  //char *sub = (char*) malloc( MAX_COMMAND_SIZE );
  //char *gaudel = (char*) malloc(MAX_COMMAND_SIZE);

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;  
    int count = 0;  
                                
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                                                                            
    char *working_str  = strdup( cmd_str );   
    head = fush(head,working_str);
                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
    
    
    if (working_str[0] == '!')
    {
      char *gaudel = figure(head,(int)(working_str[1] - '0'));
      strcpy(working_str,gaudel);
      if (working_str == NULL)
      {
        printf("Command not in history.");
        continue;
      }
    }

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    
    if (token[0] == NULL)
    {
      continue;
    }

    //exits if first token is exit or quit
    if (strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0)
    {
      exit(0);
    }

    //if first token is cd, we do not create child process
    if ((strcmp(token[0],"cd") == 0))
    {
      if(chdir(token[1]) < 0)
      {
        printf("failed to change direct.");
      }

    }

    else if (strcmp(token[0],"showpids") == 0)
    {
      displayPid(LLH);
    }

    else if (strcmp(token[0],"history") == 0)
    {
      displayHist(head);

    }

   else
   {
      pid_t pid = fork();
      count ++;
      if (count > 0 && count <=15)
      {
        LLH = push (LLH, pid);
      }

      if (pid == 0)
      {
        //execvp execute new process based on the argument
        if (execvp(token[0],token)< 0)
        {
           printf("Command not found.\n");
        }
      }
      else 
      {
        int status;
         wait( & status );
     }
      free( working_root );
  }
}
  return 0;
}

