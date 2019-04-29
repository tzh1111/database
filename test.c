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
    if(writeBlockToDisk(result,*(RBLK),buf)!=0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }
    (*RBLK)+=1;
    freeBlockInBuffer(result,buf);
    result=getNewBlockInBuffer(buf);
    return *RBLK;
}

int ProRA(Buffer *buf)//2:5:1
{
    int R_next=1,tempblk=0,flag=0,resultp=0,ri=0,ti=0,A=0,B=0,RBLK=4444;
    unsigned char *bufblkr,*bufblktemp[10],*result;
    const int turn=16;
    //prepare a result blk
    result=getNewBlockInBuffer(buf);
    resultp=0;
    int temp_next=4444;//where results begins
    for(int turni=0;turni<turn;turni++)
    {
        flag=0;
        printf("\nturni:%d,turn:%d\n",turni,turn-1);
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
        for(int tempturn=1,tempturni=0;tempturni<tempturn;tempturni++)
        {
            printf("\tempnturni:%d,tempturn:%d\n",tempturni,tempturn-1);
            for(int ti=0;ti<6&&temp_next!=0;ti++)
            //for(int ti=0;ti<1;ti++)
            {
                if(tempblk==0)
                    break;
                if((bufblktemp[ti]=readBlockFromDisk(temp_next,buf))==NULL)
                {
                    printf("Reading block failed!\n");
                    return -1;
                }
                //temp_next=ReadFourBytes(bufblktemp[ti]+56);
                temp_next++;
            }//each inside turn read 5 blks of temp(for results already exits)
            //compare 1:6 and write to result
            //--------------==B==2
            flag=0;
            for(int rtuple=0;rtuple<7;rtuple++)
            {//only 1 blk of R in buf, for each tuple of rblk
                flag=0;
                for(int tblk=0;tblk<ti;tblk++)
                {//compare with tblks
                    flag=0;
                    for(int ttuple=0;ttuple<ti;ttuple++)
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
                    break;
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
                    //*(result+resultp)=*(bufblkr[r]+ri*8);
                    resultp+=4;
                    // printf("\nA:%s\nB:%s",*(result+resultp-8),*(result+resultp-4));
                    if(resultp==56)//满64写回，后继块地址呢？
                    {
                        RBLK=ReLoadResult(buf,result,&RBLK);
                        resultp=0;
                        tempblk++;
                    }
                }
            }//for each tuple in r
            //compare ends
            for(int f=0;f<ti;f++)
            {
                freeBlockInBuffer(bufblktemp[f],buf);
            }
            tempturn=(tempblk%6==0)?tempblk/6:tempblk/6+1;
        }//turn of temp
        freeBlockInBuffer(bufblkr,buf);
    }//turn of r
    if(resultp!=0)
    {//write to disk though result blk is not full
        for(int t=0;resultp+t<buf->blkSize;t++)
            *(result+resultp+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resultp=0;
    }
    freeBlockInBuffer(result,buf);
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
    ProRA(&buf);
    //unionsr(&buf,temp);
	//cha(&buf,temp,-1);
	//printf("io次数：%l",buf->numIO);
	return 0;
}
