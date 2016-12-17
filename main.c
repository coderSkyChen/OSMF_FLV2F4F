#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#define BYTE unsigned char
#define FLVHEAD_LEN 26
#define LETTER_MAX_LEN 256
#define MAXLINE 1024
long long getlen(int input[4])
{//将四个字节的16数进制转为一个10进制数，input中每一个元素代表一个字节
    int fir=0,sec=0,mici=7;
    long long res=0;
    for(int i=0;i<4;i++)
    {
        fir=input[i]/16;
        sec=input[i]%16;
        //printf("%d %d ",fir,sec);
        res+=fir*pow(16,mici)+sec*pow(16,mici-1);
        //printf("%lld\n",res);
        fir=0;sec=0;mici-=2;
    }
    return res;
}
int String2Bytes(unsigned char* szSrc, unsigned char* pDst, int nDstMaxLen)
{//将字符串转化为字节数组，便于将真正的16数写入文件

    if(szSrc == NULL)
    {
        return 0;
    }
    int iLen = strlen((char *)szSrc);
    if (iLen <= 0 || iLen%2 != 0 || pDst == NULL || nDstMaxLen < iLen/2)
    {
        return 0;
    }

    iLen /= 2;
   // strupr((char *)szSrc);
    for (int i=0; i<iLen; i++)
    {
        int iVal = 0;
        unsigned char *pSrcTemp = szSrc + i*2;
        sscanf((char *)pSrcTemp, "%02x", &iVal);
        pDst[i] = (unsigned char)iVal;
    }

    return iLen;
}
int getIndex_BF(BYTE *S,long long S_len,BYTE *T,int pos)
{ // 返回子串T在主串S中第pos个字符之后的位置。若不存在，则函数值为0。
    // 其中，T非空，1≤pos≤StrLength(S)。算法4.5
    long long i,j;
    i=pos;
    j=0;
    while(i<S_len&&j<4)
        if(S[i]==T[j]) // 继续比较后继字符
        {
            ++i;
            ++j;
        }
        else // 指针后退重新开始匹配
        {
            i=i-j+1;
            j=0;
        }
        if(j=4)
            return i-4;
        else
            return 0;
}

static int getIdex_Sunday(unsigned char *src, long long len_s,unsigned char *des)
{
    int i, pos = 0;
    int len_d;
    int alphabet[LETTER_MAX_LEN] = {0};

    if(src == NULL || des == NULL)
        return -1;

    //len_s = strlen(src);
    len_d = 4;

    for(i = 0; i < LETTER_MAX_LEN; i++)
        alphabet[i] = len_d;

    for(i = 0; i < len_d; i++)
        alphabet[des[i]] = len_d - i - 1;

    for(pos = 1; pos <= len_s - len_d; ) {
        for(i = pos - 1; i - pos + 1 < len_d; i++) {
            if(src[i] != des[i - pos + 1])
                break;
        }

        if((i - pos + 1) == len_d)
            return pos;
        else
            pos += alphabet[src[pos + len_d - 1]] + 1;
    }

    return -1;
}
int osmf_parse(unsigned char* buf,long long total_len,long long  *content_len,int* offset)
{
        if(buf==NULL||total_len==0)
        return 0;

        BYTE flag[]="mdat";
        int pos=0;
        int len[4];     //存代表长度的字节

        BYTE flvhead[]="464c5601050000000900000000";
        BYTE flvbinary[FLVHEAD_LEN];
        FILE* pf2 = fopen("out1-fast.flv","w+");

        String2Bytes(flvhead,flvbinary,FLVHEAD_LEN);//hex2bytes

        //pos=getIndex_BF(buf,total_len,flag,0);
        pos=getIdex_Sunday(buf,total_len,flag)-1;

        for(int i=0;i<4;i++)//获取长度字节数组
            len[i]=buf[pos-4+i];

        *content_len=getlen(len);
        *offset=(int)(&buf[pos+4]-buf);


        fwrite(flvbinary,FLVHEAD_LEN/2,1,pf2);
        fwrite(&buf[pos+4],*content_len,1,pf2);
        fclose(pf2);
        return 1;
}

int main()
{
        int offset=0;
        long long conten_len=0;
        FILE* pf = fopen("1.f4f","rb");

        if (NULL==pf)
        {
                fprintf(stderr,"can not open the specified file ");
                return 1;
        }
        fseek(pf,0,SEEK_END);
        long long total_len = ftell(pf);
        fseek(pf,0,SEEK_SET);
        BYTE* pBuf = (BYTE*)calloc(total_len,sizeof(BYTE));
		memset(pBuf,0,total_len);
		long long get_len = total_len;
		long long offsets = 0;
		while (offsets<total_len)
        	{
                int read_len = fread(pBuf+offsets,sizeof(char),get_len,pf);
                offsets += read_len;
                get_len -= read_len;
            }
        offsets=0;
        fclose(pf);

        osmf_parse(pBuf,total_len,&conten_len,&offset);

        printf("%d %d ok",conten_len,offset);


	return 0;
}
