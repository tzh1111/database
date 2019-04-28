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
    char temp[4];
    for(int m=0;m<4;m++)
    {
        temp[m]=*(addr+m);
    }
    int r=atoi(temp);
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

int AND(Buffer *buf,TempArray *temp){//交，尚不能写回文件
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
}

int cha(Buffer *buf,TempArray *temp)
{
    AND(buf,temp);
    int R_next=1;
    int A,B,flag;
    int turn=(16%7==0)?16/7:16/7+1;
    unsigned char *result,bufblkr[10];
	unsigned int resultp=0,RBLK=2222;
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
                A=ReadFourBytes(bufblkr[r]+ri*8);
                B=ReadFourBytes(bufblkr[r]+ri*8+4);
                for(int i=0;i<temp_count;i++)
                {
                    flag=0;
                    if(temp[i].a==A&&temp[i].b==B&&temp[i].fsttmmatch==0)
                    {
                        temp[i].fsttmmatch=1;
                        flag=1;//no write
                        break;
                    }
                }
                if(flag==0)
                {
                    //writetupletoblk;
                    *(result+resultp)=A;
                    resultp+=4;
                    *(result+resultp)=B;
                    resultp+=4;
                    if(resultp==56)//满64写回，后继块地址呢？
                    {
                        RBLK=ReLoadResult(buf,result,&RBLK);
                        resultp=0;
                    }
                }
            }
        }
    }

   // for(blk in S)
        //the same as Rs

    RBLK=ReLoadResult(buf,result,&RBLK);
	resultp=0;

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
    temp=(TempArray *)malloc(sizeof(TempArray)*1000);
	AND(&buf,temp);
	//printf("io次数：%l",buf->numIO);
	return 0;
}
