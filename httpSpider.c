#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <arap/inet.h>
#include <sys/socket.h>
#endif 
#include "httpSpider.h"
#include "linkqueue.h"

#define logfile "spiderLog.txt"
const char *httpHeader = "GET %s HTTP/1.0 \r\n" \
             "User-Agent: Mozilla/5.0(compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0 \r\n\r\n";

#ifdef _WIN32
void initSocket()
{
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 0), &ws);
}
void cleanSocket()
{
    WSACleanup();
}
#endif

FILE *flog;
void initSpider()
{
    flog = fopen(logfile, "w");
}
void termSpider()
{
    fflush(flog);
    fclose(flog);
}

#ifdef __linux__
typedef int SOCKET;
#endif
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#ifdef _WIN32
#define close closesocket
#endif

int request(char *host, char *path, int port, char *buffer, int maxb)
{
    SOCKET sokfd;
    struct sockaddr_in sokad;
    int sdlen, rlen;
	
    memset(&sokad, 0, sizeof(sokad));
    sokad.sin_family = AF_INET;     //ipv4
    sokad.sin_port = htons(port);
    if((sokad.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
    {
        puts("ip��Ч��������Ϣ��������־");
        fprintf(flog, "-- ip��Ч��������Ϣ:\n  ����:%s ��Դ:%s �˿�:%d\n\n", host, path, port);
        return -1;
    }
	
    if((sokfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        puts("����socket������ʧ�ܣ�������Ϣ��������־");
        fprintf(flog, "-- ����socket������ʧ�ܣ�������Ϣ:\n  ����:%s ��Դ:%s �˿�:%d\n\n", host, path, port);
        return -1;
    }
    
    if(connect(sokfd, (struct sockaddr *)&sokad, sizeof(sokad)) < 0)
    {
        puts("����ʧ�ܣ�������Ϣ��������־");
        fprintf(flog, "-- ����ʧ�ܣ�������Ϣ:\n  ����:%s ��Դ:%s �˿�:%d\n\n", host, path, port);
    }
    char *rqbuf = (char *)malloc(sizeof(char) * 512);
    sdlen = sprintf(rqbuf, httpHeader, path);
	
    send(sokfd, rqbuf, sdlen, 0);
    free(rqbuf);
	
    rlen = recv(sokfd, buffer, maxb, 0);
    if(rlen < 0)
    {
        puts("��������ʧ�ܣ�������Ϣ��������־");
        fprintf(flog, "-- ��������ʧ�ܣ�������Ϣ:\n  ����:%s ��Դ:%s �˿�:%d\n\n", host, path, port);
        return -1;
    }
    close(sokfd);
    return rlen;
}

void attachPlug(spiderPlug *p, spider *sp)
{
    #ifdef _WIN32
    HINSTANCE hlib = LoadLibrary(p -> plug);
    p -> nativePointer = (long)hlib;
    if(!hlib)
    {
        puts("�������dllʧ��");
        fprintf(flog, "-- �������dllʧ��, ������Ϣ:\n dll·��:%s\n\n", p -> plug);
        //not return, just a warning
    }
    if(!(sp -> analyzer = (analyzerType)GetProcAddress(hlib, p -> func)))
    {
        printf("����:�Ҳ������dll�еķ�������%s\n", p -> func);
        fprintf(flog, "-- ����:�Ҳ������dll�еķ�������%s:\n dll·��:%s\n\n",p -> func, p -> plug);
    }
    p -> attached = true;
    #endif
}
void detachPlug(spiderPlug *p)
{
    #ifdef _WIN32
    free((HINSTANCE)p -> nativePointer);
    #endif
}

//���ĵ���������
#define BUF_MAXSZ 100000
void bfs(spider *sp)
{
    char *data = (char *)malloc(sizeof(char) * BUF_MAXSZ);
    char pathb[256];
    puts("��ʼ����");
    fprintf(flog, "-- ��ʼ����\n\n");
    
    linkQueue q;
    initQueue(&q); 
    ansiString root;
    initAnsiString(&root, "/");
    pushQueue(&q, root);
    
    //��Ҫ���أ��ֵ�����������
    
    while(q.size > 0)
    {
        ansiString curl = popQueue(&q);
        int len = request(sp -> host, curl.buffer, 80, data, BUF_MAXSZ);
        if(len < 0)continue;
        
        //������ҳԴ���룬��չ��һ��ڵ�
        puts("�������: ��ǰҳ��ĳ���");
        
        char *ps = data;
        for(int i = 0; i < len; i++)
        {
            int bk = 1;
            //if(ps[i] == '<')
            //    bk++;
            //else if(ps[i] == '>')
             //   bk--;
            if(bk)      //��html�ı�ǩ����
            {
                if(i+4 >= len)continue;
                if(ps[i] == 'h')                 //peek h
                    if(ps[i+1] == 'r')           //peek r
                        if(ps[i+2] == 'e')       //peek e
                            if(ps[i+3] == 'f')   //peek f
                            {
                                i = i+4;
                                while(ps[i] != '=')i++;
                                while(ps[i] != '\"')i++;  // href = "
                                i++;
                                while(ps[i] == ' ')i++;    //���Զ���ո�
                                
                                //�ҵ�����
                                int j = 0;
                                while(ps[i] != '\"')
                                    pathb[j++] = ps[i++];
                                pathb[j] = 0;
                                puts(pathb);
                            }
            }
        }
        printf("ҳ��%s ������ϣ� ������һ��ҳ��");
        destroyAnsiString(&curl);
       
    }
     
    destroyQueue(&q);
    free(data);
}

int main()
{
    #ifdef _WIN32
    initSocket();
    #endif
    initSpider(); 
	
    spider sp;
    spiderPlug pg;
    
    memset(&pg, 0, sizeof(spiderPlug));
    memset(&sp, 0, sizeof(spider));
    strcpy(pg.func, "analyzer");
    strcpy(pg.plug, "plugin.dll");
    attachPlug(&pg, &sp);
    
    // -----ʵ��׶�
    strcpy(sp.host, "115.28.164.3");
    sp.port = 80;
    bfs(&sp);
    //-------
    
    detachPlug(&pg);
    termSpider();
    #ifdef _WIN32
    cleanSocket();
    #endif
    return 0;
}