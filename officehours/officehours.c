
#include <stdbool.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4

/* TODO */
/* Add your synchronization variables here */

/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */
pthread_mutex_t class_lock; //mutex lock 
pthread_mutex_t break_lock;
sem_t class_seat;
static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;
static int consequetive_studenta;
static int consequetive_studentb;
static int student_classA ;
static int student_classB ;
bool prof_take_break;


typedef struct 
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
  int class;
} student_info;

/* Called at beginning of simulation.  
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename) 
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
  consequetive_studenta = 0;
  consequetive_studentb = 0;
  prof_take_break = false;
  student_classA = 0;
  student_classB = 0;
  sem_init(&class_seat, 0, MAX_SEATS);
  /* Initialize your synchronization variables (and 
   * other variables you might use) here
   */
  

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL) 
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) && 
           i < MAX_STUDENTS ) 
  {
    i++;
  }

  fclose(fp);
  return i;
}

/* Code executed by professor to simulate taking a break  
 */
static void take_break() 
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  //printf("Break time over.\n");
  assert( students_in_office == 0 );
  students_since_break = 0; 
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students. 
 */
void *professorthread(void *junk) 
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1) 
  {
    prof_take_break = false;//professor is not on a break
    //break time conditions
    if(students_since_break >= professor_LIMIT && students_in_office == 0)
    {
      prof_take_break = true;
      take_break();
    }
  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter() 
{
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */ 
  bool permission_for_classa;
  bool loop;
  loop = false;  
  permission_for_classa = false;
  while(!loop) // until permission is granted, keep asking
  {
    pthread_mutex_lock(&class_lock);
    student_classA += 1;
    if(!prof_take_break && students_since_break < professor_LIMIT ) // checks if professor is on a break
    {
      if(students_in_office < MAX_SEATS )// && students_since_break < professor_LIMIT ) 
      {
        if(consequetive_studenta < 5 || student_classB == 0 )
        {
          if(classb_inoffice == 0 )
          {
            /* if all conditions to enter the office are satisfied.
               permission is granted*/
            permission_for_classa = true;
          }
        }  
      }
    }
    if(permission_for_classa)
    { 
      consequetive_studenta += 1;
      consequetive_studentb = 0;
      students_in_office += 1;
      classa_inoffice += 1;
      //pthread_mutex_lock(&break_lock);
      students_since_break += 1;
      //pthread_mutex_unlock(&break_lock);
      student_classA -=1;
      loop = true;
    }
    pthread_mutex_unlock(&class_lock);
  }
  sem_wait(&class_seat);    
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter() 
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */ 
  bool permission_for_classb;
  bool loop;
  loop = false;  
  permission_for_classb = false;
  while(!loop) // until permission is granted, keep asking
  {
    pthread_mutex_lock(&class_lock);
    student_classB +=1;
    if(!prof_take_break && students_since_break < professor_LIMIT ) // checks if professor is on a break
    {
      if(students_in_office < MAX_SEATS )// && students_since_break < professor_LIMIT) 
      {
        if(consequetive_studentb < 5 || student_classA ==  0)
        {
          if(classa_inoffice == 0 )
          {
            /* if all conditions to enter the office are satisfied.
               permission is granted*/
            permission_for_classb = true;
          }
        }  
      }
    }
    if(permission_for_classb)
    {
      consequetive_studenta = 0;
      consequetive_studentb += 1;
      students_in_office += 1;
      classb_inoffice += 1;
      //pthread_mutex_lock(&break_lock);
      students_since_break += 1;
      //pthread_mutex_unlock(&break_lock);
      student_classB -=1;
      loop = true;
    }
    pthread_mutex_unlock(&class_lock);
  }
  sem_wait(&class_seat);    
}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void ask_questions(int t) 
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave() 
{
  pthread_mutex_lock(&break_lock);
  students_in_office -= 1;
  classa_inoffice -= 1;
  pthread_mutex_unlock(&break_lock);
  sem_post(&class_seat);
}

/* Code executed by a class B student when leaving the office.
 * Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave() 
{
  pthread_mutex_lock(&break_lock);
  students_in_office -= 1;
  classb_inoffice -= 1;
  pthread_mutex_unlock(&break_lock);
  sem_post(&class_seat);
}

/* Main code for class A student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();  

  printf("Student %d from class A leaves the office\n", s_info->student_id);
 //printf("\nstudents_since_break:%d, students_in_office:%d\n\n",students_since_break,students_in_office);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si) 
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();        

  printf("Student %d from class B leaves the office\n", s_info->student_id);
  //printf("\nstudents_since_break:%d, students_in_office:%d\n\n",students_since_break,students_in_office);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
 */
int main(int nargs, char **args) 
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2) 
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0) 
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);
  // PROFESSOR_THREAD
  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result) 
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }
  // STUDENT_THREADS
  for (i=0; i < num_students; i++) 
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
      
    }
    else // student_type == CLASSB
    {
      
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
      //printf("\nstudentsinofficebe:%d\n",students_in_office);
    }

    if (result) 
    {
      printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++) 
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);
  sem_destroy(&class_seat);
  printf("Office hour simulation done.\n");
  return 0;
}

