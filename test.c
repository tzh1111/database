#include <stdio.h>
#include <stdlib.h>
#include "extmem.h"
#include <string.h>

typedef struct TempArray{
    int a;
    int b;
    int fsttmmatch;//first time match?0/1:true/false
}TempArray;
//READ DATA FROM DISK TO TEMPARRAY
int temp_count=0;
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

int ReLoadResult_abc(Buffer *buf,unsigned char *result,unsigned int* RBLK)
{//从result写到RBLK
   // printf("blocksize:%d",buf->blkSize);
/*    sprintf(result+56,"%s","    ");
    sprintf(result+60,"%d",*(RBLK)+1);*/
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

int ReLoadResult(Buffer *buf,unsigned char *result,unsigned int* RBLK)
{//从result写到RBLK
   // printf("blocksize:%d",buf->blkSize);
    sprintf(result+56,"%s","    ");
    sprintf(result+60,"%d",*(RBLK)+1);
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

int NestLoopJoin(Buffer *buf)
{//R:S:result---6:1:1
    int S_next=20,A,B,temp_count;
    unsigned char *bufblks,*result;
    TempArray temp[100];
    result=getNewBlockInBuffer(buf);
    int resulti=0,RBLK=900;
    while(S_next<=51)
    {
        if((bufblks=readBlockFromDisk(S_next,buf))==NULL)
        {
            printf("Reading block failed!\n");
            return -1;
        }
        S_next++;
       // S_next=ReadFourBytes(bufblks+52);
        for(int tuple=0;tuple<7;tuple++)
        {
             A=ReadFourBytes(bufblks+tuple*8);
             B=ReadFourBytes(bufblks+tuple*8+2);
             temp_count=LinearSearch(buf,5,A,temp);
             for(int tempi=0;tempi<temp_count;tempi++)
             {
                 sprintf(result+12*resulti,"%d",A);//resulti:0-4
                 sprintf(result+12*resulti+4,"%d",B);
                 sprintf(result+12*resulti+8,"%d",temp[tempi].b);
                 resulti++;
                 if(resulti>=5)
                 {
                     RBLK=ReLoadResult(buf,result,&RBLK);
                     resulti=0;
                 }
             }
        }
        freeBlockInBuffer(bufblks,buf);
    }
    if(resulti!=0)
    {
        for(int t=0;resulti+t<buf->blkSize;t++)
            *(result+resulti+t)=0;//stuff with 0(ascii)
        RBLK=ReLoadResult(buf,result,&RBLK);
        resulti=0;
    }
    freeBlockInBuffer(result,buf);
}

int sortinside(Buffer *buf,int choose)//ok
{
    TempArray temp[10];
    int ti=0;
    unsigned char *blkloaded;//addr
    int R_next;
    int allblk, RBLK;
    if(choose==0)
        {allblk=16;RBLK=600;R_next=1;}
    else
        {allblk=32;RBLK=700;R_next=20;}
    for(int blki=0;blki<allblk;blki++)
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
        sprintf(blkloaded+56,"%s","    ");
        sprintf(blkloaded+60,"%d",blki+RBLK+2);
        if(writeBlockToDisk(blkloaded,blki+RBLK+1,buf)!=0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        freeBlockInBuffer(blkloaded,buf);
    }
}

int MergeSort(Buffer *buf,int choose)//ok(800/850)
{
    TempArray temp[10];
    int ti=0,blki=0;
    const int VERYLARGE=10000;
    unsigned char *bufblkr[10],*bufblkresult=getNewBlockInBuffer(buf);//addr
    int R_next,A,B,C,D,resulttuple=0,RBLK,allturn,rmax;
    if(choose==0)
    {R_next=601;RBLK=800;allturn=3;rmax=616;}
    else
    {R_next=701;RBLK=850;allturn=5;rmax=732;}
    sortinside(buf,choose);
    for(int turn=0;turn<allturn;turn++)
    {
        for(blki=0;blki<7&&R_next<=rmax;blki++)//maybe less than 7
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
            temp[road].b=ReadFourBytes(bufblkr[road]+4);
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
            sprintf(bufblkresult+resulttuple*8,"%s","    ");
            sprintf(bufblkresult+resulttuple*8+4,"%s","    ");
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
    freeBlockInBuffer(bufblkresult,buf);
}

int MergeSortPlus(Buffer *buf,int choose)//每条路上一般有7块了，或者更少
{//记录每条路上总共的块数，每条路上现在的块号，现在存在temp里的tuple，最后落实到tuple
    //tuple结束换下一个tuple，若下一个tuple=8，换下一个块，若下一个块大于等于总共块数量，置这一路为无穷大
    const int VERYLARGE=10000;
    int TotalBlkInRoad[10],NowBlkInRoad[10],TupleInRoad[10],resulttuple=0;
    int R_next,Allblks,RBLK=1000;
    TempArray temp[10];
    unsigned char *BufBlkRoad[10],*result;//addr
    if(choose==0)//R
        {Allblks=16;R_next=800;RBLK=1000;}
    else
        {Allblks=32;R_next=850;RBLK=1050;}
    MergeSort(buf,choose);
    result=getNewBlockInBuffer(buf);
    //road0:800-806, road1:807-813, road2:814-815
    for(int roadi=0;roadi<7;roadi++)//init
    {
        TotalBlkInRoad[roadi]=(Allblks>=7)?7:Allblks;
        Allblks-=TotalBlkInRoad[roadi];
        if(TotalBlkInRoad[roadi]==0)
        {
            temp[roadi].a=VERYLARGE;
            continue;
        }
        if((BufBlkRoad[roadi]=readBlockFromDisk(R_next+roadi*7,buf))==NULL)
        {
            printf("Reading block failed!\n");
            return -1;
        }
        NowBlkInRoad[roadi]=0;
        TupleInRoad[roadi]=0;
        temp[roadi].a=ReadFourBytes(BufBlkRoad[roadi]);
        temp[roadi].b=ReadFourBytes(BufBlkRoad[roadi]+4);
        TupleInRoad[roadi]++;
    }//init
    while(1)
    {
        int min=VERYLARGE;
        int minroad=0;
        for(int roadi=0;roadi<7;roadi++)
        {
            if(temp[roadi].a<min)
            {
                min=temp[roadi].a;
                minroad=roadi;
            }
        }//find min in 7 roads
        if(min==VERYLARGE)//all roads are nothing left
            break;
        //write the min tuple of 7 roads to result
        sprintf(result+resulttuple*8,"%s","    ");
        sprintf(result+resulttuple*8+4,"%s","    ");
        sprintf(result+resulttuple*8,"%d",temp[minroad].a);
        sprintf(result+resulttuple*8+4,"%d",temp[minroad].b);
        resulttuple++;//next tuple to write
        if(resulttuple>=7)//if result blk is full, reload
        {
            RBLK=ReLoadResult(buf,result,&RBLK);
            resulttuple=0;
        }// writetemp[minroad]toblk
        if(TupleInRoad[minroad]>=7)//min road has no tuple left
        {
            if(NowBlkInRoad[minroad]<TotalBlkInRoad[minroad]-1)//if has blk to load
            {
                NowBlkInRoad[minroad]++;
                //load next blk for minroad
                freeBlockInBuffer(BufBlkRoad[minroad],buf);//before read, free it
                if((BufBlkRoad[minroad]=readBlockFromDisk(R_next+minroad*7+NowBlkInRoad[minroad],buf))==NULL)
                {
                    printf("Reading block failed!\n");
                    return -1;
                }
                TupleInRoad[minroad]=0;
            }
            else
            {temp[minroad].a=VERYLARGE;continue;}//nothing left , set max and continue next turn of cmp
        }
        //read tuple for minroad
        temp[minroad].a=ReadFourBytes(BufBlkRoad[minroad]+TupleInRoad[minroad]*8);
        temp[minroad].b=ReadFourBytes(BufBlkRoad[minroad]+TupleInRoad[minroad]*8+4);
        TupleInRoad[minroad]++;//next tuple to read
    }
    int F=((Allblks%7==0)?Allblks/7:Allblks/7+1);
    for(int f=0;f<F;f++)
    {
        freeBlockInBuffer(BufBlkRoad[f],buf);
    }
    freeBlockInBuffer(result,buf);
}

int FindAllEqual(Buffer *buf,char *blknow,int tuplenow,int lowblk,int highblk,int value,TempArray *result)
{
    int temp=0,tuplecatch=tuplenow-1;
    int resultp=1;
    while(1)
    {
        if(tuplecatch<0)
        {
            if((blknow=readBlockFromDisk(lowblk+(highblk-lowblk)/2,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }//load mid blk
            tuplecatch=6;
        }
        if((temp=ReadFourBytes(blknow+(tuplecatch)*8))==value)
        {
            result[resultp].a=temp;
            result[resultp].b=ReadFourBytes(blknow+(tuplecatch)*8+4);
            tuplecatch--;
            printf("\n===%d,%d===\n",result[resultp].a,result[resultp].b);
            resultp++;
        }
        else
            break;
    }
    temp=0;
    tuplecatch=tuplenow+1;
    while(1)
    {
        if(tuplecatch>=7)
        {
            if((blknow=readBlockFromDisk(lowblk+(highblk-lowblk)/2,buf))==NULL)
            {
                printf("Reading block failed!\n");
                return -1;
            }//load mid blk
            tuplecatch=6;
        }
        if((temp=ReadFourBytes(blknow+(tuplecatch)*8))==value)
        {
            result[resultp].a=temp;
            result[resultp].b=ReadFourBytes(blknow+(tuplecatch)*8+4);
            tuplecatch++;
            resultp++;
            printf("\n===%d,%d===\n",result[resultp].a,result[resultp].b);
        }
        else
            break;
    }
}

//begin at:1000/1050
//sort for binary search
int BinarySearch(Buffer *buf,int choose,int value)
{
    MergeSortPlus(buf,choose);
    unsigned char *blknow;
    int lowblk=0,lowtuple=0,highblk,hightuple=6;
    TempArray tuplenow,result[100];//tuple value  and tuple location
    if(choose==0)
        {lowblk=1000;highblk=1015;}
    else
        {lowblk=1050;highblk=1081;}
    if((blknow=readBlockFromDisk(lowblk+(highblk-lowblk)/2,buf))==NULL)
    {
        printf("Reading block failed!\n");
        return -1;
    }
    //load mid tuple
    while(1)
    {
        tuplenow.fsttmmatch=lowtuple+(hightuple-lowtuple)/2;
        tuplenow.a=ReadFourBytes(blknow+tuplenow.fsttmmatch*8);
        tuplenow.b=ReadFourBytes(blknow+tuplenow.fsttmmatch*8+4);
        if(value<tuplenow.a)
        {
            if(ReadFourBytes(blknow+lowtuple*8)<value)
            {//in this blk lower tuples
                hightuple=tuplenow.fsttmmatch;
            }
            if(value<ReadFourBytes(blknow+lowtuple*8))
            {//not in this blk, but lower blk
                lowtuple=0;
                hightuple=6;
                highblk=lowblk+(highblk-lowblk)/2;
                if((blknow=readBlockFromDisk(lowblk+(highblk-lowblk)/2,buf))==NULL)
                {
                    printf("Reading block failed!\n");
                    return -1;
                }//load mid blk
            }
            else
            {
                //find the fst lower then value(blknow,lowtuple)
                result[0].a=tuplenow.a;
                result[0].b=tuplenow.b;
                FindAllEqual(buf,blknow,tuplenow.fsttmmatch,lowblk,highblk,value,result);
                break;
            }
        }
        if(tuplenow.a<value)
        {
            if(ReadFourBytes(blknow+hightuple*8)<value)
            {//in this blk lower tuples
                lowtuple=0;
                hightuple=6;
                highblk=lowblk+(highblk-lowblk)/2;
                if((blknow=readBlockFromDisk(lowblk+(highblk-lowblk)/2,buf))==NULL)
                {
                    printf("Reading block failed!\n");
                    return -1;
                }//load mi
            }
            if(value<ReadFourBytes(blknow+hightuple*8))
            {//not in this blk, but lower blk
                lowtuple=tuplenow.fsttmmatch;
            }
            else
            {
                //find the fst lower then value(blknow,hightuple)
                result[0].a=tuplenow.a;
                result[0].b=tuplenow.b;
                FindAllEqual(buf,blknow,tuplenow.fsttmmatch,lowblk,highblk,value,result);
                break;
            }
        }
        if(tuplenow.a==value)
        {
            //find the fst lower then value(blknow,tuplenow)
            result[0].a=tuplenow.a;
            result[0].b=tuplenow.b;
            FindAllEqual(buf,blknow,tuplenow.fsttmmatch,lowblk,highblk,value,result);
                break;
        }
    }
}

int LinearSearch(Buffer *buf, int blkforr, int value, TempArray *temp)
{
    int R_next=1,resultp=0,rblk=0,A=0,RBLK=5555;
    const int turn=(16%blkforr==0)?16/blkforr:16/blkforr+1;
    unsigned char *bufblkr[10],*result;
    int temp_count=0;
    if(value==-1)
    {
        printf("\ninput value:");
        scanf("%d",&value);
    }
    result=getNewBlockInBuffer(buf);
    for(int turni=0;turni<turn;turni++)
    {
        for(rblk=0;rblk<blkforr&&R_next!=0;rblk++)
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
                    temp[temp_count].a=A;
                    temp[temp_count].b=ReadFourBytes(bufblkr[rblki]+rtuple*8+4);
                    temp_count++;
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
    return temp_count;
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
                                RBLK=ReLoadResult_abc(buf,result,&RBLK);
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
        RBLK=ReLoadResult_abc(buf,result,&RBLK);
        resultp=0;
    }
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
                        RBLK=ReLoadResult_abc(buf,result,&RBLK);
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
        RBLK=ReLoadResult_abc(buf,result,&RBLK);
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
                        RBLK=ReLoadResult_abc(buf,result,&RBLK);
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
        RBLK=ReLoadResult_abc(buf,result,&RBLK);
        resultp=0;
    }
	printf("\n-------%d--------\n",cnt);}
	else
        printf("\nerror choice!\n");

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
    //AND(&buf,temp);
	//cha(&buf,temp,-1);
	//LinearSearch(&buf,7,-1,temp);
	//sortinside(&buf,1);
	//MergeSort(&buf,1);
	//NestLoopJoin(&buf);
	//printf("io次数：%l",buf->numIO);
	//MergeSortPlus(&buf,1);
	/*int value;
	scanf("%d",&value);
	BinarySearch(&buf,0,value);*/
	return 0;
}
