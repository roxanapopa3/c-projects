#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LINE_SIZE 300

struct Dir;
struct File;
//Struct for directories
typedef struct Dir
{
    char *name;
    struct Dir* parent;
    struct File* head_children_files;
    struct Dir* head_children_dirs;
    struct Dir* next;
} Dir;
//Struct for files
typedef struct File
{
    char *name;
    struct Dir* parent;
    struct File* next;
} File;
//Struct for Stack
typedef struct Stack
{
    char *name;
    struct Stack *next;
} Stack;
//Function to add elements to stack
void push(Stack **head, char *name)
{
    int len=strlen(name);
    struct Stack *newNode= malloc(sizeof(Stack));
    newNode->name=malloc((len+1));
    strcpy(newNode->name,name);
    newNode->next=NULL;
    newNode->next=(*head);
    (*head)=newNode;
}
//Function to delete elements from stack
char* pop(Stack **head)
{
    if(*head!=NULL)
    {
        struct Stack* temp = *head;
        int len=strlen(temp->name);
        char *rez=malloc((len+1));
        strcpy(rez,temp->name);
        *head=(*head)->next;
        free(temp->name);
        free(temp);
        return rez;
    }
    return NULL;
}
//Function to create a new file
File* makeFile(Dir* parent, char *name)
{
    struct File* newFile = malloc(sizeof(File));
    int len=strlen(name);
    newFile->parent=parent;
    newFile->name=malloc(len+1);
    strcpy(newFile->name,name);
    newFile->next=NULL;
    return newFile;
}
//Function to create a new directory
Dir* makeDir(Dir* parent, char* name)
{
    struct Dir* newDir=malloc(sizeof(Dir));
    int len=strlen(name);
    newDir->parent=parent;
    newDir->head_children_dirs=NULL;
    newDir->head_children_files=NULL;
    newDir->name=malloc(len+1);
    strcpy(newDir->name,name);
    newDir->next=NULL;
    return newDir;
}
//Function to add a new file to the file system
void touch (Dir* parent, char* name)
{
    struct Dir* temp_d=parent->head_children_dirs;
    int gasit_f=0,gasit_d=0;
    if(parent->head_children_files==NULL)
    {
        struct File* newNode=makeFile(parent,name);
        newNode->next=parent->head_children_files;
        parent->head_children_files=newNode;
        return;
    }
    struct File* temp_f=parent->head_children_files;
    if(temp_f->next==NULL)
    {
        if(!strcmp(temp_f->name,name))
        {
            gasit_f=1;
        }
    }
    while(temp_d!=NULL)
    {
        if(!strcmp(temp_d->name,name))
            gasit_d=1;
        temp_d=temp_d->next;
    }
    while(temp_f->next!=NULL)
    {
        if(!strcmp(temp_f->name,name))
            gasit_f=1;
        temp_f=temp_f->next;
    }
    if(gasit_d==1 || gasit_f==1)
    {
        printf("File already exists\n");
        return;
    }
    else
    {
        temp_f->next=makeFile(parent,name);
        temp_f=temp_f->next;
    }

}
//Function to add a new directory to the file system
void mkdir (Dir* parent, char* name)
{
    int gasit_f=0,gasit_d=0;
    if(parent->head_children_dirs==NULL)
    {
        struct Dir* newNode=makeDir(parent,name);
        newNode->next=parent->head_children_dirs;
        parent->head_children_dirs=newNode;
        return;
    }
    struct Dir* temp_d=parent->head_children_dirs;
    struct File* temp_f=parent->head_children_files;
    if(temp_d->next==NULL)
    {
        if(!strcmp(temp_d->name,name))
        {
            gasit_d=1;
        }
    }

    while(temp_d->next!=NULL)
    {
        if(!strcmp(temp_d->name,name))
        {
            gasit_d=1;
        }
        temp_d=temp_d->next;
    }
    while(temp_f!=NULL)
    {
        if(!strcmp(temp_f->name,name))
            gasit_f=1;
        temp_f=temp_f->next;
    }
    if(gasit_d==1 || gasit_f==1)
    {
        printf("Directory already exists\n");
        return;
    }
    else
    {
        temp_d->next=makeDir(parent,name);
        temp_d=temp_d->next;
    }


}
//Function to show all directories
void showDir(Dir* parent)
{
    struct Dir* temp_d=parent->head_children_dirs;
    if(temp_d==NULL)
        return;
    while(temp_d!=NULL)
    {
        printf("%s\n",temp_d->name);
        temp_d=temp_d->next;
    }
}
//Function to show all files
void showFile(Dir* parent)
{
    struct File* temp_f=parent->head_children_files;
    if(temp_f==NULL)
        return;
    while(temp_f!=NULL)
    {
        printf("%s\n",temp_f->name);
        temp_f=temp_f->next;
    }
}
//ls function
void ls (Dir* parent)
{
    struct File* temp_f=parent->head_children_files;
    struct Dir* temp_d=parent->head_children_dirs;
    if(temp_d==NULL && temp_f==NULL){
		return;
}
    showDir(parent);
    showFile(parent);
}
//Function to free the memory of a file
void freeFile(File* target){
	free(target->name);
	free(target);
}
//rm function
void rm (Dir* parent, char* name)
{
    struct File* temp_f=parent->head_children_files;
    struct File* next_f;
    int gasit_f=0;
    if(temp_f==NULL)
    {
        printf("Could not find the file\n");
        return;
    }
    if(!strcmp(parent->head_children_files->name,name))
    {
        next_f=parent->head_children_files;
        parent->head_children_files=parent->head_children_files->next;
	freeFile(next_f);
        return;
    }
    next_f=parent->head_children_files->next;
    while(next_f!=NULL)
    {
        if(!strcmp(next_f->name,name))
        {
            gasit_f=1;
            break;
        }
        temp_f=temp_f->next;
        next_f=next_f->next;
    }
      if(gasit_f==0)
    {
        printf("Could not find the file\n");
        return;
    }
    if(gasit_f==1)
    {
        temp_f->next=next_f->next;
	freeFile(next_f);
    }
}
//Function to delete all files in a directory
void rm_all_files(Dir* parent)
{
    struct File* temp_f=parent->head_children_files;
    struct File* next;
    while(temp_f!=NULL)
    {
        next=temp_f->next;
	free(temp_f->name);
        free(temp_f);
        temp_f=next;
    }
}
//Function that frees the memory of a directory
void freeDir(Dir* target){
	free(target->name);
	free(target);
}
//rmdir function
void rmdir (Dir* parent, char* name)
{
    struct Dir* temp_d=parent->head_children_dirs;
    struct Dir* next_d;
    int gasit_d=0;
    if(parent==NULL || temp_d==NULL)
    {
        printf("Could not find the dir\n");
        return;
    }
    if(!strcmp(parent->head_children_dirs->name,name))
    {
        next_d=parent->head_children_dirs;
        parent->head_children_dirs=parent->head_children_dirs->next;
        rm_all_files(next_d);
	freeDir(next_d);
        return;
    }
    next_d=parent->head_children_dirs->next;
    while(next_d!=NULL)
    {
        if(!strcmp(next_d->name,name))
        {
            gasit_d=1;
            break;
        }
        temp_d=temp_d->next;
        next_d=next_d->next;
    }
      if(gasit_d==0)
    {
        printf("Could not find the dir\n");
        return;
    }
    if(gasit_d==1)
    {
        temp_d->next=next_d->next;
        rm_all_files(next_d);
	freeDir(next_d);
    }

}
//Function that deletes all directories in a directory
void rm_all_dirs(Dir* parent)
{
    struct Dir* temp_d=parent->head_children_dirs;
    struct Dir* next;
    while(temp_d!=NULL)
    {
        next=temp_d->next;
        free(temp_d->name);
	free(temp_d);
        temp_d=next;
    }
    return;
}
//cd function
void cd(Dir** target, char *name)
{
    if(!strcmp(name,".."))
    {
        if((*target)->parent==NULL)
            return;
        else
        {
            (*target)=(*target)->parent;
            return;
        }
    }
    struct Dir* temp_d=(*target)->head_children_dirs;
    int gasit_d=0;
    while(temp_d!=NULL && gasit_d==0)
    {
        if(!strcmp(temp_d->name,name))
        {
            gasit_d=1;
            (*target)=temp_d;
        }
        temp_d=temp_d->next;
    }
    if(gasit_d==0)
    {
        printf("No directories found!\n");
        return;
    }

}
//pwd function
char *pwd (Dir* target)
{
    struct Stack* head=NULL;
    struct Dir* temp_d=target;
    int len,sum=0;
    char* rez;
    while(temp_d->parent!=NULL)
    {
        len=strlen(temp_d->name);
        sum+=len+1;
        push(&head,temp_d->name);
        temp_d=temp_d->parent;
    }
    rez=malloc((sum+7));
    strcpy(rez,"/home/");
    len=strlen(rez);
    rez[len]='\0';
    char *p;
    while(head!=NULL)
    {
	p=pop(&head);
        strcat(rez,p);
	free(p);
        len=strlen(rez);
        rez[len]='/';
        rez[len+1]='\0';
    }
    len=strlen(rez);
    rez[len-1]='\0';
    return rez;
}
//Function that stops the execution and frees all the used memory
void stop (Dir* target) {
    struct Dir* temp_d=target->head_children_dirs;
    struct Dir* next;
    if(target->head_children_dirs==NULL){
        rm_all_files(target);
        free(target->name);
        return;
}
    while(temp_d->head_children_dirs!=NULL){
       temp_d=temp_d->head_children_dirs;
    }
    while(temp_d->parent!=NULL){
	rm_all_dirs(temp_d);
	rm_all_files(temp_d);
	temp_d=temp_d->parent;
}
	rm_all_files(target);
	rm_all_dirs(target);
}
//tree function
void tree (Dir* target, int level)
{
    if(target==NULL)
        return;
    int i;
    struct Dir* temp_d=target->head_children_dirs;
    if(temp_d==NULL)
    {
        printf("%s\n",target->name);
        return;
    }
    while(temp_d->head_children_dirs!=NULL)
    {
        for(i=0; i<4*level; i++)
        {
            printf(" ");
        }
        printf("%s\n",temp_d->name);
        temp_d=temp_d->head_children_dirs;
        level++;
    }
    for(i=0;i<4*level;i++)
        printf(" ");
    printf("%s\n",temp_d->name);
    level++;
    while(temp_d!=NULL)
    {
        struct File* temp_f=temp_d->head_children_files;
        while(temp_f!=NULL)
        {
            for(i=0; i<4*level; i++)
            {
                printf(" ");
            }
            printf("%s\n",temp_f->name);
            temp_f=temp_f->next;
        }
        temp_d=temp_d->parent;
        level--;
    }
}

int main ()
{
    Dir* parent=malloc(sizeof(Dir));
    parent->head_children_dirs=NULL;
    parent->head_children_files=NULL;
    parent->next=NULL;
    parent->parent=NULL;
    char *comanda=malloc(MAX_INPUT_LINE_SIZE*sizeof(char));
    char *path;
    while(fgets(comanda,MAX_INPUT_LINE_SIZE,stdin)!=NULL)
    {
        int len=strlen(comanda);
        comanda[len-1]='\0';
        char *p;
        p=strtok(comanda," ");
        if(!strcmp(comanda,"touch"))
        {
            p=strtok(NULL," ");
	    strcat(p,"\0");
            touch(parent,p);

        }
        if(!strcmp(comanda,"mkdir"))
        {
            p=strtok(NULL," ");
	    strcat(p,"\0");
            mkdir(parent,p);

        }
        if(!strcmp(comanda,"ls"))
        {
            ls(parent);
        }
        if(!strcmp(comanda,"rm"))
        {
            p=strtok(NULL," ");
            rm(parent,p);

        }
        if(!strcmp(comanda,"rmdir"))
        {
            p=strtok(NULL," ");
            rmdir(parent,p);

        }
        if(!strcmp(comanda,"cd"))
        {
            p=strtok(NULL," ");
            cd(&parent,p);


        }
        if(!strcmp(comanda,"pwd"))
        {
	    path=pwd(parent);
            printf("%s",path);
	    free(path);

        }
        if(!strcmp(comanda,"tree"))
        {
            tree(parent,0);

        }
        if(!strcmp(comanda,"stop"))
        {
	    if(parent->parent!=NULL){
		while(parent->parent!=NULL)
			cd(&parent,"..");
}
            stop(parent);
            break;

        }
        else continue;
    }
    free(comanda);
    free(parent->parent);
    free(parent);
    return 0;
}
