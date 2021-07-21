#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Struct that retains the position in the matrix
typedef struct poz
{
    int x,y;
} poz;
//Struct for the tree nodes
typedef struct nod
{
    char tab[3][3];
    char jucator;
    int n,nr;
    struct nod **next;
} tree;
//Struct for the minimax tree nodes
typedef struct tree3
{
    int val,nr_copii,height;
    struct tree3* copii[200];
} tree3;
//Struct for queue
typedef struct Queue
{
    struct nodeq *head, *tail;
    int len;
} Queue;
//Struct for queue nodes
typedef struct nodeq
{
    struct tree3* node;
    struct nodeq* next;
} nodeq;
//Append function
void add_queue(Queue *q, tree3* root)
{
    q->len=q->len+1;
    nodeq* newptr=malloc(sizeof(nodeq));
    newptr->node=root;
    newptr->next=NULL;
    if(q->len==1)
    {
        newptr->next=NULL;
        q->head=newptr;
        q->tail=newptr;
        return;
    }
    q->tail->next=newptr;
    q->tail=newptr;
    q->tail->next=NULL;
    return;
}
//Pop function
void pop_queue(Queue *q)
{
    q->len=q->len-1;
    if(q->head == NULL)
        return;
    else
    {
        nodeq* temp;
        temp=q->head;
        q->head=q->head->next;
        free(temp);
    }
}
//Checks if queue is empty
int isEmpty(Queue *q)
{
    return q->head==NULL;
}
//Free queue
void free_q(Queue **q)
{
    nodeq* temp;
    while(!isEmpty(*q))
    {
        temp=(*q)->head;
        (*q)->head=(*q)->head->next;
        free(temp);
        (*q)->len--;
    }
    free(*q);
}
//Find empty positions in matrix
void find_poz(char t[][3],poz v[])
{
    int i,j,k=0;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            if(t[i][j]=='-')
            {
                v[k].x=i;
                v[k].y=j;
                k++;
            }
}
//Counts the number of free positions in matrix
int get_nr_poz(char t[][3])
{
    int i,j,count=0;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            if(t[i][j]=='-')
                count++;
    return count;
}
//Copy a matrix
void copy_mat(char surs[][3],char dest[][3])
{
    int i,j;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            dest[i][j]=surs[i][j];
}
//Check if game is over
int decizie(char tabla[][3],char jucator)
{
    int i,j,linie=0,col=0,diag=0,count,diag2=0;
    count=get_nr_poz(tabla);
    for(i=0; i<3; i++)
    {
        linie=0;
        col=0;
        for(j=0; j<3; j++)
        {
            if(tabla[i][j]==jucator)
                linie++;
            if(tabla[j][i]==jucator)
                col++;
        }
        if(col==3 || linie==3)
            count=0;
    }
    for(i=0; i<3; i++)
        if(tabla[i][i]==jucator)
            diag++;
    for(i=2; i>=0; i--)
        if(tabla[i][2-i]==jucator)
            diag2++;
    if(diag==3 || diag2==3)
        count=0;
    return count;

}
//Build the game tree
tree *insert(tree *root, char tab[][3],char jucator,int c1, int c2,int nr)
{
    int i=0;
    char temp[3][3];
    poz temp2[9];
    if(nr<0)
        return NULL;
    root=malloc(sizeof(tree));
    root->n=nr;
    root->nr=nr;
    if(nr>0)
    {
        root->next=malloc(nr*sizeof(tree*));
        for(i=0; i<nr; i++)
            root->next[i]=NULL;
    }
    if(c1!=-1 && c2!=-1){
        tab[c1][c2]=jucator;
    	copy_mat(tab,root->tab);
    	}
    else
    	copy_mat(tab,root->tab);
    if(decizie(root->tab, jucator)==0)
    {
        root->nr=0;
        return root;
    }
    find_poz(root->tab,temp2);
    i=0;
    while(i<nr)
    {
        copy_mat(root->tab,temp);
        if(jucator=='X')
            root->next[i]=insert(root->next[i],temp,'O',temp2[i].x,temp2[i].y,nr-1);
        else
            root->next[i]=insert(root->next[i],temp,'X',temp2[i].x,temp2[i].y,nr-1);
        i++;
    }
    return root;
}
//Print a matrix
void print(char t[][3],int nr, FILE* fout){
    int i,j,k;
    for(i=0; i<3; i++)
    {
        for(j=0; j<nr; j++)
            fprintf(fout,"\t");
        for(k=0; k<3; k++)
        {
            fprintf(fout,"%c",t[i][k]);
            if(k!=2)
                fprintf(fout," ");
        }
        fprintf(fout,"\n");
    }
    fprintf(fout,"\n");
}
//Print game tree
void print_t(tree* root,int nr,FILE *fout)
{
    int i;
    print(root->tab,nr-root->n,fout);
    if(root->n!=0)
    	for(i=0; i<root->n; i++)
        	if(root->next[i])
            		print_t(root->next[i],nr,fout);
}
//Free game tree
void free_tree(tree* root)
{
    int i;
    for(i=0; i<root->n; i++)
        if(root->next[i]!=NULL)
            free_tree(root->next[i]);
    if(root->n!=0)
        free(root->next);
    free(root);
}
//Free minimax tree
void free_tree3(tree3* root)
{
    int i;
    for(i=0; i<root->nr_copii; i++)
        if(root->copii[i]!=NULL)
            free_tree3(root->copii[i]);
    free(root);
}
//Check if player wins
int victorie(char tabla[][3],char jucator)
{
    int i,j,linie=0,col=0,diag=0,count=1,diag2=0;
    for(i=0; i<3; i++)
    {
        linie=0;
        col=0;
        for(j=0; j<3; j++)
        {
            if(tabla[i][j]==jucator)
                linie++;
            if(tabla[j][i]==jucator)
                col++;
        }
        if(col==3 || linie==3)
            count=0;
    }
    for(i=0; i<3; i++)
        if(tabla[i][i]==jucator)
            diag++;
    for(i=2; i>=0; i--)
        if(tabla[i][2-i]==jucator)
            diag2++;
    if(diag==3 || diag2==3)
        count=0;
    return count;

}
//Build and-or tree
int and_or(tree** root,char jucator,int h)
{
    char oponent;
    int i,a,ok,ok2;
    if(victorie((*root)->tab,jucator)==0)
    {
        (*root)->jucator='T';
        return 1;
    }
    if(jucator=='X')
        oponent='O';
    else
        oponent='X';
    if(victorie((*root)->tab,oponent)==0)
    {
        (*root)->jucator='F';
        return 0;
    }
    if(get_nr_poz((*root)->tab)==0)
    {
        (*root)->jucator='F';
        return 0;
    }
    ok=0;
    ok2=1;
    for(i=0; i<(*root)->nr; i++)
    {
        a=and_or(&(*root)->next[i],jucator,h+1);
        if(a==1)
        {
            ok=1;
        }
        if(a==0)
        {
            ok2=0;
        }

    }
    if(h%2==0)
    {
        if(ok==1)
        {
            (*root)->jucator='T';
            return 1;
        }
        else
        {
            (*root)->jucator='F';
            return 0;
        }

    }
    if(h%2==1)
    {
        if(ok2==0)
        {
            (*root)->jucator='F';
            return 0;
        }
        else
        {
            (*root)->jucator='T';
            return 1;
        }

    }

}
//Print and-or tree
void print2(tree* root,int n, FILE *fout)
{
    int i,j;
    for(i=0; i<n-root->n; i++)
        fprintf(fout,"\t");
    fprintf(fout,"%c",root->jucator);
    fprintf(fout,"\n");
    for(j=0; j<root->n; j++)
        if(root->next[j])
            print2(root->next[j],n,fout);
}
//Finds min child of node
int minim(tree3 *root)
{
    int mini,i;
    mini=root->copii[0]->val;
    for(i=1; i<root->nr_copii; i++)
        if(root->copii[i]->val < mini)
            mini=root->copii[i]->val;
    return mini;
}
//Finds max child of node
int maxim(tree3 *root)
{
    int maxi,i;
    maxi=root->copii[0]->val;
    for(i=1; i<root->nr_copii; i++)
        if(root->copii[i]->val > maxi)
            maxi=root->copii[i]->val;
    return maxi;
}
//Build minimax tree
void minimax(tree3* root,int height,int tip)
{
    int i;
    	if(height<0)
    		return;
    	if(height>0)
    {
        for(i=0; i<root->nr_copii; i++)
            if(tip==0)
                minimax(root->copii[i],height-1,-1);
            else
               minimax(root->copii[i],height-1,0);
        if(root->nr_copii!=0)
            if(tip==0)
                root->val=maxim(root);
            else
                root->val=minim(root);
    }

}
//Print minimax tree
void print3(tree3* root,int n, FILE* fout)
{
    int i;
    for(i=0; i<n; i++)
        fprintf(fout,"\t");
    fprintf(fout,"%d\n",root->val);
    for(i=0; i<root->nr_copii; i++)
        if(root->copii[i]!=NULL)
            print3(root->copii[i],n+1,fout);
}

int main(int argc,char *argv[])
{
    int i,j,z,height,nr;
    tree *root=NULL;
    Queue *q;
    char jucator,oponent;
    char tabla[3][3];
    FILE *fin=fopen(argv[2],"r");
    FILE *fout=fopen(argv[3],"w");
    q=malloc(sizeof(Queue));
    q->head=NULL;
    q->tail=NULL;
    q->len=0;
    if(fin==NULL)
        exit(0);
    if(strcmp(argv[1],"-c1")==0 || strcmp(argv[1],"-c2")==0)
    {
        fscanf(fin,"%c ",&jucator);
        if(jucator=='X')
        	oponent='O';
        else oponent='X';
        for(i=0; i<3; i++)
            for(j=0; j<3; j++)
                fscanf(fin,"%c ",&tabla[i][j]);
        z=decizie(tabla,jucator);
        if(strcmp(argv[1],"-c1")==0 )
        {
            root=insert(root,tabla,oponent,-1,-1,z);
            print_t(root,root->n,fout);
            free_tree(root);
        }
        if(strcmp(argv[1],"-c2")==0)
        {
            root=insert(root,tabla,oponent,-1,-1,z);
            and_or(&root,jucator,0);
            print2(root,root->n,fout);
            free_tree(root);
        }
        }
        else if(strcmp(argv[1],"-c3")==0)
        {
            tree3* temp=malloc(sizeof(tree3));
            tree3* p;
            fscanf(fin,"%d ",&height);
            temp->height=height;
            add_queue(q,temp);
            p=temp;
            while(!feof(fin))
            {
                if(fscanf(fin," (%d)",&nr))
                {
                    p=q->head->node;
                    pop_queue(q);
                    p->nr_copii=nr;
                    p->val=0;
                    i=0;
                    while(i<nr)
                    {
                        p->copii[i]=malloc(sizeof(tree3));
                        p->copii[i]->val=0;
                        add_queue(q,p->copii[i]);
                        i++;
                    }
                }
                else if(fscanf(fin,"[%d] ",&nr))
                {
                    p=q->head->node;
                    pop_queue(q);
                    p->nr_copii=0;
                    p->val=nr;
                }
            }
            minimax(temp,height,0);
            print3(temp,0,fout);
            free_tree3(temp);

        }
        free_q(&q);
        fclose(fin);
        fclose(fout);
    }
