
/*

    Description: The program takes input file,containing page reference string with page frame being the first character,from teh commmand line and gives the page
    faults for various page fault algorithm

    
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255   
#define MAX_NUM_ARGUMENTS 5   


void direcInfo();
int LBAToOffset(int32_t sector);
void openFile(char inputCommand[]);
void execCommand();
void stat(char *name);
int32_t clusterSize(int32_t cluster);
char * realName(char *dirname);
int32_t getFirstCluster(char *name);
void get(char *name);
void chngDirec(int32_t cluster);
void closeNow();
void readNow(char*,char*,char*);
void printInfo();
void Hex(int value);



char* token[MAX_NUM_ARGUMENTS]; 
char cmd_str[MAX_COMMAND_SIZE];
FILE *inFile;

char BS_OEMName[8];
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt; 
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0; 
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

int32_t currDirec;
char formattedDirectory[12];
char BPB_Volume[11];
char newName[12];



struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];


// calculating offset to reach root directory
int LBAToOffset(int32_t sector)
{
	if (sector == 0) sector = 2;

	return ((sector - 2) * BPB_BytesPerSec)+(BPB_BytesPerSec*BPB_RsvdSecCnt) + (BPB_NumFATs*BPB_FATSz32*BPB_BytesPerSec);
}


//open the fat32 img 
void openFile(char inputCommand[])
{ 
	inFile = fopen(inputCommand,"r");
    if (inFile == NULL)
    {
        printf("Image does not exist\n");
        return;
    }
    printf("%s opened.\n", inputCommand);

    fseek(inFile, 3, SEEK_SET);
    fread(&BS_OEMName, 8, 1, inFile);

    fseek(inFile, 11, SEEK_SET);
    fread(&BPB_BytesPerSec, 2, 1, inFile);
    fread(&BPB_SecPerClus, 1, 1, inFile);
    fread(&BPB_RsvdSecCnt, 2, 1, inFile);
    fread(&BPB_NumFATs, 1, 1, inFile);
    fread(&BPB_RootEntCnt, 2, 1, inFile);

    fseek(inFile, 36, SEEK_SET);
    fread(&BPB_FATSz32, 4, 1, inFile);

    fseek(inFile, 44, SEEK_SET);
    fread(&BPB_RootClus, 4, 1, inFile);
    currDirec = BPB_RootClus;


     

    int offset = LBAToOffset(currDirec);
    fseek(inFile, offset, SEEK_SET);
    fread(&dir[0], 32, 16, inFile);

}


void execCommand()
{
	
    if (token[0] == NULL)
    {   
        return;
    }

    if (strcmp(token[0], "open") == 0)
    {
        if (inFile != NULL)
        {
            printf("Error: File system image already open.\n");
            return;
        }

        if (token[1] != NULL && inFile == NULL)
        {
            openFile(token[1]);
        }
        else if (token[1] == NULL)
        {
            printf("Error: Name the file you want to open\n");
        }
        return;
    }

    else if (strcmp(token[0], "info") == 0)
    {
        printInfo();
    }

    else if (strcmp(token[0],"ls") == 0)
    {
        direcInfo();

    }
    else if (strcmp(token[0], "stat") == 0)
    {
        stat(token[1]);
    }

    else if (strcmp(token[0], "cd") == 0)
    {
        if (token[1] == NULL)
        {
            printf("Error: Name the directory you want to\n");
            return;
        }
        chngDirec(getFirstCluster(token[1]));
    }

    else if (strcmp(token[0], "get") == 0)
    {
        get(token[1]);
    }

    else if (strcmp(token[0], "read") == 0)
    {
        if (token[1] == NULL || token[2] == NULL || token[3] == NULL)
        {
            printf("Please input valid arguments.\n");
            return;
        }
        readNow(token[1], token[2], token[3]);
    }
    else if (strcmp(token[0], "close") == 0)
    {
        closeNow();
    }

    else if (inFile == NULL)
    {
        printf("Error: File system image must be opened first.\n");
        return;
    }
}


// prints the information of filesystem in hexadecimal and base 10
void printInfo()
{
    printf("BPB_BytesPerSec: %d\n", BPB_BytesPerSec);
    Hex(BPB_BytesPerSec);
    printf("BPB_SecPerClus: %d\n ", BPB_SecPerClus);
    Hex(BPB_SecPerClus);
    printf("BPB_RsvdSecCnt: %d\n", BPB_RsvdSecCnt);
    Hex(BPB_RsvdSecCnt);
    printf("\n");
    printf("BPB_NumFATs: %d\n", BPB_NumFATs);
    Hex(BPB_NumFATs);
    printf("BPB_FATSz32: %d\n", BPB_FATSz32);
    Hex(BPB_FATSz32);
}


//convert the deimal value into hexadecimal value
void Hex(int value)
{
    char hexValue[64];
    int i = 0;
    int j;
    int temp;
    while (value != 0)
    {
        temp = value % 16;
        if (temp < 10)
        {
            hexValue[i] = temp + 48;
            i++;
        }
        else
        {
            hexValue[i] = temp + 55;
            i++;
        }
        
        value = value/16;
    }
    for (j = i - 1; j >= 0; j--)
    {
        printf("%c", hexValue[j]);
        
    }
    printf("\n");
}

// reads the content of the file at position up until number of bytes
void readNow(char *name, char *position, char *bytes)
{
    int index = atoi(position);
    int intbyte = atoi(bytes);

    int cluster = getFirstCluster(name);
    int off = LBAToOffset(cluster);
    fseek(inFile, off + index, SEEK_SET);
    char *byteCount = malloc(intbyte);
    fread(byteCount, intbyte, 1, inFile);
    printf("%s\n", byteCount);
}


//change the directory
void chngDirec(int32_t cluster)
{
    int i;
    if (strcmp(token[1], "..") == 0)
    {
        
        for (i = 0; i < 16; i++)
        {
            if (strncmp(dir[i].DIR_Name, "..", 2) == 0)
            {
                int off = LBAToOffset(dir[i].DIR_FirstClusterLow);
                currDirec = dir[i].DIR_FirstClusterLow;
                fseek(inFile, off, SEEK_SET);
                fread(&dir[0], 32, 16, inFile);
                return;
            }
        }
    }
    int off = LBAToOffset(cluster);
    currDirec = cluster;
    fseek(inFile, off, SEEK_SET);
    fread(&dir[0], 32, 16, inFile);
}

// get the file in the directory you are currently working
void get(char *name)
{
    char *rename = (char *)malloc(strlen(name));
    strncpy(rename, name, strlen(name));
    int cluster = getFirstCluster(rename);
    FILE *outFile = fopen(token[1], "w");
    fseek(inFile, LBAToOffset(cluster), SEEK_SET);
    unsigned char *index = malloc(clusterSize(cluster));
    fread(index, clusterSize(cluster), 1, inFile);
    fwrite(index, clusterSize(cluster), 1, outFile);
    fclose(outFile);
}


// prints the information of file and directory
void stat(char *name)
{
    int i;
    int cluster = getFirstCluster(name);
    int capacity = clusterSize(cluster);
    printf("Size: %d\n", capacity);
    
    for (i = 0; i < 16; i++)
    {
        if (cluster == dir[i].DIR_FirstClusterLow)
        {
            printf("Attr: %d\n", dir[i].DIR_Attr);
            printf("Starting Cluster: %d\n", cluster);
            printf("Cluster High: %d\n", dir[i].DIR_FirstClusterHigh);
        }
    }
    return;
}

//get the size of the cluster that you want to accept
int32_t clusterSize(int32_t cluster)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        if (cluster == dir[i].DIR_FirstClusterLow)
        {
            int size = dir[i].DIR_FileSize;
            return size;
        }
    }
    return -1;
}


//convert shortfile name into exact filename
char * realName(char *dirname)
{
    int i;
    
    memset(newName, ' ', 12);

    char *poken = strtok(dirname, ".");

    if (poken)
    {
        strncpy(newName, poken, strlen(poken));

        poken = strtok(NULL, ".");

        if (poken)
        {
            strncpy((char *)(newName + 8), poken, strlen(poken));
        }
        newName[11] = '\0';

        for (i = 0; i < 11; i++)
        {
            newName[i] = toupper(newName[i]);
        }
    }
    else
    {
        strncpy(newName, dirname, 12);
        newName[11] = '\0';
       
    }
    return newName;
}

//get the size of the first cluster
int32_t getFirstCluster(char *dirname)
{
    

    int i;
    for (i = 0; i < 16; i++)
    {
        char *direc = malloc(sizeof(dir[i].DIR_Name));
        memset(direc, '\0', sizeof(dir[i].DIR_Name));
        memcpy(direc, dir[i].DIR_Name, 11);

        if (strncmp(direc, realName(dirname), 11) == 0)
        {
            int cluster = dir[i].DIR_FirstClusterLow;
            return cluster;
        }
    }

    return -1;
}

//prints the content of the directory when you press ls
void direcInfo()
{
    int i;
    int off = LBAToOffset (currDirec);
    fseek (inFile,off,SEEK_SET);
    if (inFile == NULL)
    {
        printf("Image is not opened.\n");
    }
    for (i = 0; i< 16; i++)
    {
        fread(&dir[i],32,1,inFile);
        if ((dir[i].DIR_Name[0] != (char)0xe5) && (dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
        {
            char *contains = malloc(sizeof(dir[i].DIR_Name));
            memset(contains, '\0', 11);
            memcpy(contains, dir[i].DIR_Name, 11);
            printf("%s\n", contains);
        }
    }
}


//close the fat image
void closeNow()
{
    if (inFile == NULL)
    {
        printf("Error: File system not open.");
        return;
    }

    fclose(inFile);
    
}

int main(int argc, char **argv)
{
	while(1)
	{
		printf("mfs> ");

   		memset(cmd_str, '\0', MAX_COMMAND_SIZE);
	    // Read the command from the commandline.  The
	    // maximum command that will be read is MAX_COMMAND_SIZE
	    // This while command will wait here until the user
	    // inputs something since fgets returns NULL when there
	    // is no input
   	    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
        
    	/* Parse input */

   		int token_count = 0;

    	// Pointer to point to the token
    	// parsed by strsep
    	char *arg_ptr;

    	char *working_str = strdup(cmd_str);

    	// we are going to move the working_str pointer so
   		// keep track of its original value so we can deallocate
   	    // the correct amount at the end
        char *working_root = working_str;

   		memset(&token, '\0', MAX_NUM_ARGUMENTS);

   	   // Tokenize the input strings with whitespace used as the delimiter
    	memset(&token, '\0', sizeof(MAX_NUM_ARGUMENTS));
    	while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    	{
	        token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
	        if (strlen(token[token_count]) == 0)
	        {
	            token[token_count] = NULL;
	        }
	        token_count++;
	    }

    	free(working_root);

        execCommand();
	}

	
}

