/* Mohit Tamang
   100152822
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_LINE 1024

//************ Implementations for FIFO ************//

// A structure that represents a FIFO queue
struct Queue 
{ 
    int first,last, size; 
    unsigned capacity; 
    int* array; 
}; 
  
// function to create a fifo, given a size
struct Queue* createQueue(unsigned capacity) 
{ 
    struct Queue* fifo = (struct Queue*) malloc(sizeof(struct Queue)); 
    fifo->capacity = capacity; 
    fifo->first = fifo->size = 0;  
    fifo->last = capacity - 1;  // This is important, see the enqueue 
    fifo->array = (int*) malloc(fifo->capacity * sizeof(int)); 
    return fifo; 
} 
  
// to know when a queue is full 
int isFull(struct Queue* fifo) 
{  
	return (fifo->size == fifo->capacity); 
} 
  
// to know when a quque is empty 
int isEmpty(struct Queue* fifo) 
{  
	return (fifo->size == 0); 
} 

// adds given item to the queue 
void push(struct Queue* fifo, int item) 
{ 
    if (isFull(fifo)) 
        return; 
    fifo->last = (fifo->last + 1)%fifo->capacity; 
    fifo->array[fifo->last] = item; 
    fifo->size = fifo->size + 1; 
} 
  
// pops the first element of the queue
int pop(struct Queue* fifo) 
{ 
    if (isEmpty(fifo)) 
        return INT_MIN; 
    int item = fifo->array[fifo->first]; 
    fifo->first = (fifo->first + 1)%fifo->capacity; 
    fifo->size = fifo->size - 1; 
    return item; 
} 
  
// Function to get front of queue 
int front(struct Queue* fifo) 
{ 
    if (isEmpty(fifo)) 
        return INT_MIN; 
    return fifo->array[fifo->first]; 
} 
  
//*********************************************//

/*
 * Function: num_in_working_set
 * Parameter(s): working_set, num, size of working_set
 * Returns: returns true if it finds num in
 * the working_set
 */ 
bool num_in_working_set(int working_set[],int num,int working_set_size)
{
	int i;
	for(i=0;i<working_set_size;i++)
	{
		if(working_set[i]==num)
		{
			return true;
		}
		else
		{
			continue;
		}
	}
	return false;
}

/*
 * Function: working_set_full
 * Parameter(s): working_set, size of working_set
 * Returns: returns true if working_set is full
 */ 
bool working_set_full(int working_set[],int working_set_size)
{
	int i;
	for(i=0;i<working_set_size;i++)
	{
		if(working_set[i] == -1)
		{
			return false;
		}
		else
		{
			continue;
		}
	}
	return true;
}

/*
 * Function: findMin_index
 * Parameter(s): an array, size of that array
 * Returns: returns the index of the min value from an array
 */ 
int findMin_index(int index[],int size)
{
	int i,min,min_index ;
	min = index[0];
	min_index = 0;
	for(i=1;i<size;i++)
	{
		if(index[i]<min)
		{
			min = index[i];
			min_index = i;
		}
	}
	return min_index;
}

/*
 * Function: findMax_index
 * Parameter(s): an array, size of that array
 * Returns: returns the index of the max value from an array
 */ 
int findMax_index(int index[],int size)
{
	int i,max,max_index ;
	max = index[0];
	max_index = 0;
	for(i=1;i<size;i++)
	{
		if(index[i]>max)
		{
			max = index[i];
			max_index = i;
		}
	}
	return max_index;

}

/*
 * Function: page_fault_FIFO
 * Parameter(s): an array of page requests called page_reference,
 * size of the working_set, size of the page_reference array(count)
 * Returns: total number of page faults that occur using a FIFO
 * (First In First Out) page replacement algorithm
 */
int page_fault_FIFO(int page_reference[],int working_set_size,int count)
{
	int i,j,k,l;
	int page_fault = 0;
	int working_set[working_set_size];
	struct Queue* fifo = createQueue(working_set_size); 

	// populating working_set with -1 values
	// to represent a empty working_set
	for(j=0;j<working_set_size;j++)
	{
		working_set[j] = -1;
	}
	for(i=0;i<count;i++)
	{
		int curr_num;
		curr_num = page_reference[i];
		// if curr_num is found in the working set, move onto
		// the next number
		if(num_in_working_set(working_set,curr_num,working_set_size))
		{
			continue;
		}
		// else, we know that a page fault has occured
		else
		{
			page_fault += 1;
			// if working_set is not full
			if(!working_set_full(working_set,working_set_size))
			{
				//add curr_num to the working set
				for(k=0;k<working_set_size;k++)
				{
					if(working_set[k]==-1)
					{
						working_set[k] = curr_num;
						break;
					}
				}
				push(fifo,curr_num);
			}
			// if working_set is full use FIFO to determine which  
			// number in working_set is replaced by curr_num
			else
			{
				for(l=0;l<working_set_size;l++)
				{
					if(working_set[l]==front(fifo))
					{
						working_set[l] = curr_num;
					}
				}
				pop(fifo);
				push(fifo,curr_num);
			}
		}
	}
	printf("Page faults of FIFO:    %d\n",page_fault);
	return page_fault;
}

/*
 * Function: page_fault_LRU_MRU
 * Parameter(s): an array of page requests called page_reference, 
 * size of the working_set, size of the page_reference array(count)
 * Returns: total number of page faults that occur using a LRU
 * (Least Recently Used) page replacement algorithm
 * Description: Also able to calculate page faults using MRU (Most
 * Recently Used Algorithm)
 */
int page_fault_LRU_MRU(int page_reference[],int working_set_size,int count)
{
	int loop;
	int page_fault;
	loop = 0;
	//repeats twice, once for LRU and again for MRU
	while(loop<1)
	{
		page_fault =0;
		int i,j,k,l;
		int index[working_set_size];
		int working_set[working_set_size];

		for(j=0;j<working_set_size;j++)
		{
			working_set[j] = -1;
		}

		for(i=0;i<count;i++)
		{
			int curr_num;
			curr_num = page_reference[i];
			// if curr_num is found in the working set, move onto
			// the next number
			if(num_in_working_set(working_set,curr_num,working_set_size))
			{
				continue;
			}
			// else, we know that a page fault has occured
			else
			{
				page_fault +=1;
				// if working_set is not full
				if(!working_set_full(working_set,working_set_size))
				{
					//add curr_num to the working set
					for(k=0;k<working_set_size;k++)
					{
						if(working_set[k]==-1)
						{
							working_set[k] = curr_num;
							break;
						}
					}
				}
				// if working_set is full use LRU to determine which  
				// number in working_set is replaced by curr_num
				else
				{
					//we know this is LRU as its the first loop
					if(loop==0)
					{
						for(j=0;j<working_set_size;j++)
						{
							for(l=0;l<i;l++)
							{
								if(working_set[j]==page_reference[l])
								{
									index[j] = l;
								}
							}
						}
						//this is to identify LRU
						working_set[findMin_index(index,working_set_size)] = curr_num;
					}
					// this will be for MRU
					if(loop==1)
					{
						for(j=0;j<working_set_size;j++)
						{
							for(l=0;l<i;l++)
							{
								if(working_set[j]==page_reference[l])
								{
									index[j] = l;
								}
							}
						}
						//this is to identify MRU
						working_set[findMax_index(index,working_set_size)] = curr_num;
					}
					
				}
			}
		}
		if(loop==0)
		{
			printf("Page faults of LRU:     %d\n",page_fault);
			
		}
		loop+=1;	
	}
	// uncomment the code below to get value of MRU algorithm
	//printf("Page faults of MRU:     %d\n",page_fault);
}

/*
 * Function: page_fault_MFU
 * Parameter(s): an array of page requests called page_reference, 
 * size of the working_set, size of the page_reference array(count)
 * Returns: total number of page faults that occur using a MFU
 * (Most Frequently Used) page replacement algorithm
 */
int page_fault_MFU(int page_reference[],int working_set_size,int count)
{
	int i,j,k,l;
	int page_fault = 0;
	int working_set[working_set_size];
	int repeats[working_set_size];
	// populating working_set with -1 values
	// to represent a empty working_set
	for(j=0;j<working_set_size;j++)
	{
		working_set[j] = -1;
		repeats[j] = 0;
	}
	for(i=0;i<count;i++)
	{
		int curr_num;
		curr_num = page_reference[i];
		// if curr_num is found in the working set, move onto
		// the next number
		if(num_in_working_set(working_set,curr_num,working_set_size))
		{
			continue;
		}
		// else, we know that a page fault has occured
		else
		{
			page_fault += 1;
			// if working_set is not full
			if(!working_set_full(working_set,working_set_size))
			{
				//add curr_num to the working set
				for(k=0;k<working_set_size;k++)
				{
					if(working_set[k]==-1)
					{
						working_set[k] = curr_num;
						break;
					}
				}
			}
			// if working_set is full use MFU to determine which  
			// number in working_set is replaced by curr_num
			else
			{
				for(j=0;j<working_set_size;j++)
				{
					for(l=0;l<i;l++)
					{
						if(working_set[j]==page_reference[l])
						{
							repeats[j] += 1;
						}
					}
				}
				//printf("%d,",working_set[findMax_index(repeats,working_set_size)]);
				working_set[findMax_index(repeats,working_set_size)] = curr_num;

			}
		}
	}
	printf("Page faults of MFU:     %d\n",page_fault);
	return page_fault;
}

/*
 * Function: page_fault_optimal
 * Parameter(s): an array of page requests called page_reference, 
 * size of the working_set, size of the page_reference array(count)
 * Returns: total number of page faults that occur using a Optimal
 * page replacement algorithm
 */
int page_fault_optimal(int page_reference[],int working_set_size,int count)
{
	int i,j,k,l;
	int page_fault = 0;
	int working_set[working_set_size];
	int index[working_set_size];
	// populating working_set with -1 values
	// to represent a empty working_set
	for(j=0;j<working_set_size;j++)
	{
		working_set[j] = -1;
	}
	for(i=0;i<count;i++)
	{
		int curr_num;
		curr_num = page_reference[i];
		// if curr_num is found in the working set, move onto
		// the next number
		if(num_in_working_set(working_set,curr_num,working_set_size))
		{
			continue;
		}
		// else, we know that a page fault has occured
		else
		{
			page_fault += 1;
			// if working_set is not full
			if(!working_set_full(working_set,working_set_size))
			{
				//add curr_num to the working set
				for(k=0;k<working_set_size;k++)
				{
					if(working_set[k]==-1)
					{
						working_set[k] = curr_num;
						break;
					}
				}
			}
			// if working_set is full, and curr_num is not in the workins_set
			// use FIFO to determine which number in working_set is replaced by curr_num
			else
			{
				int counter;
				counter = 1;
				for(j=0;j<working_set_size;j++)
				{
					for(l=i;l<count;l++)
					{
						if(working_set[j]==page_reference[l])
						{

							index[j] = l;
							// the first index we find, we stop searching
							// and move on to the next number in working-set 
							break;
						}
					}
					// if we search for the number in the page-reference array,
					// but do not find it
					//printf("count:%d,l:%d\n",count,l);
					if(l==count)
					{
						index[j] = count+counter;
						counter++;
					}
				}
				working_set[findMax_index(index,working_set_size)] = curr_num;
			}
		}
	}
	printf("Page faults of Optimal: %d\n",page_fault);
	return page_fault;
}

int main( int argc, char * argv[] ) 
{
  char * line = NULL;
  size_t line_length = MAX_LINE;
  char * filename;
  int page_reference[100];

  FILE * file;

  if( argc < 2 )
  {
    printf("Error: You must provide a datafile as an argument.\n");
    printf("Example: ./fp datafile.txt\n");
    exit( EXIT_FAILURE );
  }

  filename = ( char * ) malloc( strlen( argv[1] ) + 1 );
  line     = ( char * ) malloc( MAX_LINE );

  memset( filename, 0, strlen( argv[1] + 1 ) );
  strncpy( filename, argv[1], strlen( argv[1] ) );

  printf("Opening file %s\n", filename );
  file = fopen( filename , "r");

  if ( file ) 
  {
    while ( fgets( line, line_length, file ) )
    {
      int count;
      count = 0;
      int i;
      char * token;

      token = strtok( line, " ");
      int working_set_size = atoi( token ); 
      printf("\nWorking set size: %d\n\n", working_set_size );
      while( token != NULL )
      {
        token = strtok( NULL, " " );   
        if( token != NULL )
        {
        	page_reference[count++] = atoi(token);
        }
      }
      page_fault_FIFO(page_reference,working_set_size,count);
      page_fault_LRU_MRU(page_reference,working_set_size,count);
      page_fault_MFU(page_reference,working_set_size,count);
      page_fault_optimal(page_reference,working_set_size,count);
      printf("\n");
    }
    free(filename);
    free( line );
    fclose(file);
  }

  return 0;
}
