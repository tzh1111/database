#include <stdio.h>
#include <stdlib.h>
#include "extmem.h"
#include <string.h>
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
int AND(Buffer *buf){
	int turn=(16%6==0)?16/6:16/6+1;
	unsigned char *bufblkr[10];
	unsigned char *bufblks;
	unsigned char *result;
	unsigned int resultp=0;
	int A,B,C,D;
	int RBLK=1111;
	unsigned int R_next=1;//ADD WHERE R BEGINS
	unsigned int S_next=20;
	printf("turn:%d\n",turn);
	result=getNewBlockInBuffer(buf);
    resultp=0;
	for (int i = 0; i < turn; ++i)
	{
	    S_next=20;
		for (int j = 0; j<6&&R_next!=0; ++j)
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
		while(S_next!=0)
        {
            printf("S-next:%d\n",S_next);
            bufblks=readBlockFromDisk(S_next,buf);
            S_next=ReadFourBytes(bufblks+56);
            for(int r=0;r<6;r++)
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
                            printf("\ncache!r:%d,si:%d,____%d,%d\n",r,si,A,B);
                            *(result+resultp)=A;
                            resultp+=4;
                            *(result+resultp)=B;
                            resultp+=4;
                            if(resultp==56)//满64写回，后继块地址呢？
                            {
                                printf("blocksize:%d",buf->blkSize);
                                *(result+56)=RBLK+1;
                                if(writeBlockToDisk(result,RBLK,&buf)!=0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                                RBLK+=1;
                                freeBlockInBuffer(result,buf);
                                result=getNewBlockInBuffer(buf);
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
}
int main()
{
    Buffer *buf;
    unsigned char *blk;
    if (!(buf=initBuffer(520, 64, &buf)))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("bufdata:%s\n",buf->data);
	AND(&buf);
	return 0;
}
