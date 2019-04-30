#include <stdio.h>
#include <stdlib.h>
#include "extmem.h"
#include <string.h>

typedef struct TempArray{
    int a;
    int b;
    int fsttmmatch;//first time match?0/1:true/false
}TempArray;
int temp_count=0;
//READ DATA FROM DISK TO TEMPARRAY

int ReadFourBytes(unsigned char *addr)
{
    char tempbyte[4];
    for(int m=0;m<4;m++)
    {
        tempbyte[m]=*(addr+m);
    }
    int r=atoi(tempbyte);
    return r;
}


int ReLoadResult(Buffer *buf,unsigned char *result,unsigned int* RBLK)
{//从result写到RBLK
   // printf("blocksize:%d",buf->blkSize);
    sprintf(result+60,"%d",*(RBLK)+1);
    if(writeBlockToDisk(result,*(RBLK),buf)!=0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }
    *(result- 1) = BLOCK_AVAILABLE;
    buf->numFreeBlk++;
    buf->numIO++;
    (*RBLK)+=1;
    freeBlockInBuffer(result,buf);
    result=getNewBlockInBuffer(buf);
    return *RBLK;
}

int sortinside(Buffer *buf)
{
    TempArray temp[10];
    int ti=0;
    unsigned char *blkloaded;//addr
    int R_next=1;
    for(int blki=0;blki<16;blki++)
    {
        if((blkloaded=readBlockFromDisk(R_next,buf))==NULL)
        {
            printf("Reading block failed!\n");
            return -1;
        }
        R_next=ReadFourBytes(blkloaded+56);
        for(int tuple=0;tuple<7;tuple++)
        {
            temp[tuple].a=ReadFourBytes(blkloaded+tuple*8);
            temp[tuple].b=ReadFourBytes(blkloaded+tuple*8+4);
        }
        for(int i=0;i<7;i++)//sort(this blk (temp) by tuple.a, larger);
        for(int j=i+1;j<7;j++)
        {
            if(temp[i].a>temp[j].a)
            {//swap(i,j)
                int t=0;
                t=temp[i].a;
                temp[i].a=temp[j].a;
                temp[j].a=t;
                t=temp[i].b;
                temp[i].b=temp[j].b;
                temp[j].b=t;
            }
        }
        for(int i=0;i<7;i++)
        {
            sprintf(blkloaded+i*8,"%d",temp[i].a);
            sprintf(blkloaded+i*8+4,"%d",temp[i].b);
        }
        //writebacktodisk;
        sprintf(blkloaded+60,"%d",blki+602);
        if(writeBlockToDisk(blkloaded,blki+601,buf)!=0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        freeBlockInBuffer(blkloaded,buf);
    }
}

int MergeSort(Buffer *buf)
{
    TempArray temp[10];
    int ti=0,blki=0;
    const int VERYLARGE=10000;
    unsigned char *bufblkr[10],*bufblkresult=getNewBlockInBuffer(buf);//addr
    int R_next=601,A,B,C,D,resulttuple=0,RBLK=800;
    for(int turn=0;turn<3;turn++)
    {
        for(blki=0;blki<7&&R_next<=616;blki++)//maybe less than 7
        {
            if((bufblkr[blki]=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next++;
        }//read 7 blks of r
        for(int road=0;road<blki;road++)
        {
            temp[road].a=ReadFourBytes(bufblkr[road]);
            temp[road].b=ReadFourBytes(bufblkr[road]);
            temp[road].fsttmmatch=0;
        }//init 7 roads
        resulttuple=0;
        for(int all=0;all<7*blki;all++)
        {
            int min=VERYLARGE;
            int minroad=0;
            for(int road=0;road<blki;road++)
            {
                if(temp[road].a<min)
                {
                    min=temp[road].a;
                    minroad=road;
                }
            }//find min in 7 roads
            sprintf(bufblkresult+resulttuple*8,"%s","");
            sprintf(bufblkresult+resulttuple*8+4,"%s","");
            sprintf(bufblkresult+resulttuple*8,"%d",temp[minroad].a);
            sprintf(bufblkresult+resulttuple*8+4,"%d",temp[minroad].b);
            resulttuple++;
            if(resulttuple>=7)
            {
                RBLK=ReLoadResult(buf,bufblkresult,&RBLK);
                resulttuple=0;
            }
           // writetemp[minroad]toblk
            temp[minroad].fsttmmatch++;
            if(temp[minroad].fsttmmatch>=7)
                temp[minroad].a=VERYLARGE;
            else
            {
                temp[minroad].a=ReadFourBytes(bufblkr[minroad]+temp[minroad].fsttmmatch*8);
                temp[minroad].b=ReadFourBytes(bufblkr[minroad]+temp[minroad].fsttmmatch*8+4);
            }
                //readtuple(minroad*8)to temp[minroad]
        }
        for(int f=0;f<7;f++)
        {
            freeBlockInBuffer(bufblkr[f],buf);
        }
        printf("\n-======-\n");
    }
}

//sort for binary search
int BinarySearch()
{

}

int LinearSearch(Buffer *buf)
{
    int R_next=1,resultp=0,value=0,rblk=0,A=0,RBLK=5555;
    const int turn=(16%7==0)?16/7:16/7+1;
    unsigned char *bufblkr[10],*result;
    printf("\ninput value:");
    scanf("%d",&value);
    result=getNewBlockInBuffer(buf);
    for(int turni=0;turni<turn;turni++)
    {
        for(rblk=0;rblk<6&&R_next!=0;rblk++)
        {
            printf("R_next:%d\n",R_next);
			if((bufblkr[rblk]=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next=ReadFourBytes(bufblkr[rblk]+56);
        }//read 7 blks of r, rest 1 blk be result
        for(int rblki=0;rblki<rblk;rblki++)
        {
            for(int rtuple=0;rtuple<7;rtuple++)
            {
                A=ReadFourBytes(bufblkr[rblki]+rtuple*8);
                if(A==value)
                {
                    for(int m=0;m<4;m++)
                        *(result+resultp+m)=*(bufblkr[rblki]+rtuple*8+m);
                    resultp+=4;
                    for(int m=0;m<4;m++)
                        *(result+resultp+m)=*(bufblkr[rblki]+rtuple*8+m+4);
                    resultp+=4;
                    if(resultp==56)//满64写回，后继块地址呢？
                    {
                        //RBLK=ReLoadResult(buf,result,&RBLK);
                        resultp=0;
                        sprintf(result+60,"%d",RBLK+1);
                        if(writeBlockToDisk(result,RBLK,buf)!=0)
                        {
                            perror("Writing Block Failed!\n");
                            return -1;
                        }
                        RBLK++;
                    }
                }
            }
        }
        //free 7 blks
        for(int f=0;f<rblk;f++)
        {
            freeBlockInBuffer(bufblkr[f],buf);
        }
    }
    //store the left results
    if(resultp!=0)
    {//write to disk though result blk is not full
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
    //free result blk
    freeBlockInBuffer(result,buf);
}

int ProRA(Buffer *buf)//2:5:1
{
    int R_next=1,tempblk=0,flag=0,resultp=0,ri=0,ti=0,A=0,B=0,RBLK=4444;
    unsigned char *bufblkr,*bufblktemp[10],*result;
    //prepare a result blk
    result=getNewBlockInBuffer(buf);
    resultp=0;
    int temp_next=4444;//where results begins
    for(int turni=0;turni<16;turni++)
    {
        flag=0;
        printf("\nblki of r:%d\n",turni);
        if(R_next!=0)
        {//read 1 blks of R
            if((bufblkr=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next=ReadFourBytes(bufblkr+56);
            printf("R_next:%d\n",R_next);
        }
        //compare 1:6 and write to result
        //--------------==B==2
        flag=0;
        for(int rtuple=0;rtuple<7;rtuple++)
        {//only 1 blk of R in buf, for each tuple of rblk
            flag=0;
            for(int tblk=0;tblk<tempblk;tblk++)
            {//compare with tblks
                flag=0;
                for(int ttuple=0;ttuple<14;ttuple++)
                {//each tuple in tblk
                    flag=0;
                    A=ReadFourBytes(bufblkr+rtuple*8);
                    B=ReadFourBytes(bufblktemp[tblk]+ttuple*4);
                    if(A==B)
                    {
                        flag=1;
                        printf("\ncatch!%d,%d\n",A,B);
                        break;
                    }
                }
                if(flag==1)
                    break;
            }//cmp with temp
            if(flag==1)
                continue;
            //compare with tuple in result
            for(int rsttuple=0;rsttuple<resultp/4;rsttuple++)
            {//each tuple in tblk
                flag=0;
                A=ReadFourBytes(bufblkr+rtuple*8);
                B=ReadFourBytes(result+rsttuple*4);
                if(A==B)
                {
                    flag=1;
                    printf("\ncatch!%d,%d\n",A,B);
                    break;
                }
            }//cmp with result
            if(flag==0)
            {//write tuple to blk;
                for(int m=0;m<4;m++)
                *(result+resultp+m)=*(bufblkr+rtuple*8+m);
                resultp+=4;
                if(resultp==56)//满64写回，后继块地址呢？
                {
                    //RBLK=ReLoadResult(buf,result,&RBLK);
                    resultp=0;
                    bufblktemp[tempblk]=result;
                    sprintf(result+60,"%d",RBLK+1);
                    if(writeBlockToDisk(result,RBLK,buf)!=0)
                    {
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    result=getNewBlockInBuffer(buf);
                    RBLK++;
                    tempblk++;
                }
            }
        }//for each tuple in r
        //compare ends
        for(int f=0;f<ti;f++)
        {
            freeBlockInBuffer(bufblktemp[f],buf);
        }
        freeBlockInBuffer(bufblkr,buf);
    }//16blks of R ok
    if(resultp!=0)
    {//write to disk though result blk is not full
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
    freeBlockInBuffer(result,buf);
    for(int f=0;f<tempblk;f++)
    {
        freeBlockInBuffer(bufblktemp[f],buf);
    }
}


int AND(Buffer *buf,TempArray *temp){
    temp_count=0;
	const int turn=(16%6==0)?16/6:16/6+1;
	unsigned char *bufblkr[10],*bufblks, *result;
	unsigned int resultp=0;
	int A,B,C,D;
	unsigned int RBLK=1111, R_next=1, S_next=20;

	result=getNewBlockInBuffer(buf);

	for (int i = 0; i < turn; ++i)
	{
	    S_next=20;
	    int j=0;
		for (; j<6&&R_next!=0; ++j)
		{
		    //WHERE READ FROM DISK,
		    //AUTOMATICALLY GET NEW BLK FROM BUF
		    printf("R_next:%d\n",R_next);
			if((bufblkr[j]=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next=ReadFourBytes(bufblkr[j]+56);
		}
		printf("\nj:%d\n",j);
		while(S_next!=0)
        {
            printf("S-next:%d\n",S_next);
            bufblks=readBlockFromDisk(S_next,buf);
            S_next=ReadFourBytes(bufblks+56);
            for(int r=0;r<j;r++)
            {
                for(int ri=0;ri<7;ri++)
                {
                    A=ReadFourBytes(bufblkr[r]+ri*8);
                    B=ReadFourBytes(bufblkr[r]+ri*8+4);
                   // printf("\nA:%d,B:%d\n",A,B);
                    for(int si=0;si<7;si++)//一个blk八个元组
                    {
                        C=ReadFourBytes(bufblks+si*8);
                        D=ReadFourBytes(bufblks+si*8+4);
                     //   printf("\nC:%d,D:%d\n",C,D);
                        if(A==C&&B==D)
                        {
                            printf("\ncache!r:%d,si:%d,____%d,%d,%d,%d\n",r,si,A,B,C,D);
                            temp[temp_count].a=A;
                            temp[temp_count].b=B;
                            temp[temp_count].fsttmmatch=0;
                            temp_count++;
                            for(int t=0;t<4;t++)
                                *(result+resultp+t)=*(bufblks+si*8+t);
                            resultp+=4;
                            for(int t=0;t<4;t++)
                                *(result+resultp+t)=*(bufblks+si*8+4+t);
                            resultp+=4;
                            if(resultp==56)//满64写回，后继块地址呢？
                            {
                                RBLK=ReLoadResult(buf,result,&RBLK);
                                resultp=0;
                            }
                        }//if(A==B&&C==D)
                    }
                }
            }//6blks of r
            freeBlockInBuffer(bufblks,buf);
        }//if s_next!=0;
        for(int f=0;f<6;f++)
        {
            freeBlockInBuffer(bufblkr[f],buf);
        }
	}//for each turn
	//最后存
	if(resultp!=0)
    {
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
    freeBlockInBuffer(result,buf);
}

int cha(Buffer *buf,TempArray *temp,int outchoose)
{
    int cnt=0;
    AND(buf,temp);
    int R_next=1;
    int A,B,flag;
    int turn=(16%7==0)?16/7:16/7+1;
    unsigned char *result,*bufblkr[10];
	unsigned int resultp=0,RBLK=2222;
	result=getNewBlockInBuffer(buf);
	int choose;
	if(outchoose!=0&&outchoose!=1)
    {
        printf("R-S:0/S-R:1\nplease choose:");
        scanf("%d",&choose);
        printf("\n---%d----\n",choose);
    }
    else
        choose=outchoose;
	if(choose==0)
	{printf("\nR-S\n");
    for(int r=0;r<turn;r++)
    {
        for (int j = 0; j<7&&R_next!=0; ++j)//READ 7 BLKS FROM DISK EACH TIME
        {
            //WHERE READ FROM DISK,
            //AUTOMATICALLY GET NEW BLK FROM BUF
            printf("R_next:%d\n",R_next);
            if((bufblkr[j]=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next=ReadFourBytes(bufblkr[j]+56);
            for(int ri=0;ri<7;ri++)
            {
                A=ReadFourBytes(bufblkr[j]+ri*8);
                B=ReadFourBytes(bufblkr[j]+ri*8+4);
                for(int i=0;i<temp_count;i++)
                {
                    flag=0;
                    if(temp[i].a==A&&temp[i].b==B&&temp[i].fsttmmatch==0)
                    {
                        temp[i].fsttmmatch=1;
                        cnt++;
                        printf("\n=====%d:%d=====%d:%d====\n",temp[i].a,temp[i].b,A,B);
                        flag=1;//no write
                        break;
                    }
                }
                if(flag==0)
                {
                    //writetupletoblk;
                    for(int m=0;m<4;m++)
                    *(result+resultp+m)=*(bufblkr[j]+ri*8+m);
                    //*(result+resultp)=*(bufblkr[r]+ri*8);
                    resultp+=4;
                    for(int m=0;m<4;m++)
                    *(result+resultp+m)=*(bufblkr[j]+ri*8+4+m);
                    resultp+=4;
                   // printf("\nA:%s\nB:%s",*(result+resultp-8),*(result+resultp-4));
                    if(resultp==56)//满64写回，后继块地址呢？
                    {
                        RBLK=ReLoadResult(buf,result,&RBLK);
                        resultp=0;
                    }
                }
            }
        }
        for(int f=0;f<7;f++)
        {
            freeBlockInBuffer(bufblkr[f],buf);
        }
    }
    if(resultp!=0)
    {
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
    printf("\n____%d_____cnt\n",cnt);}

   // for(blk in S)
        //the same as Rs
    else if(choose==1)
    {/*for(int tempi=0;tempi<temp_count;tempi++)
        temp[tempi].fsttmmatch=0;
    cnt=0;*/
    RBLK=3333;
    R_next=20;
    resultp=0;
    turn=(32%7==0)?32/7:32/7+1;
    printf("\nS-R\n");
    for(int r=0;r<turn;r++)
    {
        for (int j = 0; j<7&&R_next!=0; ++j)//READ 7 BLKS FROM DISK EACH TIME
        {
            //WHERE READ FROM DISK,
            //AUTOMATICALLY GET NEW BLK FROM BUF
            printf("S_next:%d\n",R_next);
            if((bufblkr[j]=readBlockFromDisk(R_next,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }
            R_next=ReadFourBytes(bufblkr[j]+56);
            for(int ri=0;ri<7;ri++)
            {
                A=ReadFourBytes(bufblkr[j]+ri*8);
                B=ReadFourBytes(bufblkr[j]+ri*8+4);
                for(int i=0;i<temp_count;i++)
                {
                    flag=0;
                    if(temp[i].a==A&&temp[i].b==B&&temp[i].fsttmmatch==0)
                    {
                        temp[i].fsttmmatch=1;
                        printf("\n=====%d:%d=====%d:%d====\n",temp[i].a,temp[i].b,A,B);
                        cnt++;
                        flag=1;//no write
                        break;
                    }
                }
                if(flag==0)
                {
                    //writetupletoblk;
                    for(int m=0;m<4;m++)
                    *(result+resultp+m)=*(bufblkr[j]+ri*8+m);
                    //*(result+resultp)=*(bufblkr[r]+ri*8);
                    resultp+=4;
                    for(int m=0;m<4;m++)
                    *(result+resultp+m)=*(bufblkr[j]+ri*8+4+m);
                    resultp+=4;
                   // printf("\nA:%s\nB:%s",*(result+resultp-8),*(result+resultp-4));
                    if(resultp==56)//满64写回，后继块地址呢？
                    {
                        RBLK=ReLoadResult(buf,result,&RBLK);
                        resultp=0;
                    }
                }
            }
        }
        for(int f=0;f<7;f++)
        {
            freeBlockInBuffer(bufblkr[f],buf);
        }
    }
    if(resultp!=0)
    {
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
	printf("\n-------%d--------\n",cnt);}
	else
        printf("\nerror choice!\n");
    freeBlockInBuffer(result,buf);
}

int unionsr(Buffer *buf,TempArray *temp)
{
    cha(buf,temp,1);
    printf("\nresult in : (blk1:blk16)+(blk3333:)\n");
}

int main()
{
    Buffer *buf;
    TempArray *temp;
    unsigned char *blk;
    if (!(buf=initBuffer(520, 64, &buf)))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    temp=(TempArray *)malloc(sizeof(TempArray)*100);
    memset(temp, 0, sizeof(TempArray)*100);
    //ProRA(&buf);
    //unionsr(&buf,temp);
   // AND(&buf,temp);
	//cha(&buf,temp,-1);
	//LinearSearch(&buf);
	sortinside(&buf);
	MergeSort(&buf);
	//printf("io次数：%l",buf->numIO);
	return 0;
}
