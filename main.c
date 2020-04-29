#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/syscall.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#define __USE_GNU
#include<sched.h>
#define MXN 100
#define MXM 1000000

char s[5],name[MXN][35];
int n,R[MXN],T[MXN],sT[MXN],pid[MXN],ind[MXN];
int *lock,*num,fd;
int a[MXN],rid[MXM],ct=0;
int crid[MXM],ccnt[MXM],cct;

void unit_time(){
    volatile unsigned long i;
    for(i=0;i<1000000UL;i++);
}

void idle_time(int iter){
    for(int i=0;i<iter;i++) unit_time();
}

void set_priority(int cpid,int priority){
    struct sched_param param;
    param.sched_priority=priority;
    assert(sched_setscheduler(cpid,SCHED_FIFO|SCHED_RESET_ON_FORK,&param)==0);
}

void child(int iter,long ists,long istns){
    long sts=ists,stns=istns,eds,edns;
    int cpid=getpid(),pre=-1;
    for(int i=0;i<iter;i++){
        unit_time();
        while(*lock!=cpid);
        if(i+1==iter) break;
        if(i==pre+*num){
            pre=i;
            set_priority(0,1);
            *lock=-1;
        }
    }
    assert(syscall(333,&eds,&edns)==0);
    assert(syscall(334,getpid(),sts,stns,eds,edns)==0);
    close(fd);
    *lock=-1;
    exit(0);
}

int create_child(int iter){
    long sts,stns;
    assert(syscall(333,&sts,&stns)==0);
    int cpid=fork();
    assert(cpid>=0);
    if(cpid==0){
        child(iter,sts,stns);
    }
    return cpid;
}

void run_child(int cpid,int cnum){
    *num=cnum; *lock=cpid;
    set_priority(cpid,80);
    while(*lock!=-1);
    *num=0;
}

void input(){
    scanf("%s%d",s,&n);
    for(int i=0;i<n;i++)
        scanf("%s%d%d",name[i],&R[i],&T[i]);
}

void Rsort(){
    for(int i=0;i<n;i++) ind[i]=i;
    for(int i=0;i<n;i++){
        for(int j=0;j+1<n-i;j++){
            if(R[ind[j]]>R[ind[j+1]]){
                ind[j]^=ind[j+1];
                ind[j+1]^=ind[j];
                ind[j]^=ind[j+1];
            }
        }
    }
}

void init(){
    cpu_set_t cpus;
    CPU_ZERO(&cpus);CPU_SET(0,&cpus);
    set_priority(0,10);
    fd=shm_open("shm_tmp",O_RDWR|O_CREAT,0);
    assert(fd>=0);
    assert(shm_unlink("shm_tmp")==0);
    assert(ftruncate(fd,sizeof(int)*2)==0);
    lock=(int*)mmap(NULL,sizeof(int)*2,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    num=lock+1;
    assert(lock!=(int*)-1);
    input();
    Rsort();
    for(int i=0;i<n;i++) sT[i]=T[i];
}

void output(){
    for(int i=0;i<n;i++)
        printf("%s %d\n",name[i],pid[i]);
}

void finish(){
    set_priority(0,1);
    for(int i=0;i<n;i++){
        int status;
        assert(waitpid(pid[i],&status,0)==pid[i]);
        assert(status==0);
    }
    assert(munmap(lock,sizeof(int)*2)==0);
    close(fd);
    output();
}

void simulate(){
    int cur,pre;
    for(int i=0,j=0;i<ct;i++){
        while(j<n&&R[ind[j]]==i){
            if(i!=pre){
                crid[cct]=cur;
                ccnt[cct]=i-pre;
                pre=i; cct++;
            }
            crid[cct]=ind[j];
            ccnt[cct]=0;
            cct++; j++;
        }
        if(rid[i]!=cur){
            if(i!=pre){
                crid[cct]=cur;
                ccnt[cct]=i-pre;
                pre=i; cct++;
            }
            cur=rid[i];
        }
    }
    crid[cct]=cur;
    ccnt[cct]=ct-pre;
    cct++;
    for(int i=0;i<cct;i++){
        if(ccnt[i]==0) pid[crid[i]]=create_child(sT[crid[i]]);
        else if(crid[i]==-1) idle_time(ccnt[i]);
        else run_child(pid[crid[i]],ccnt[i]);
    }
}

void FIFO(){
    int j,ql=0,qr=0,cur=-1,r=n;
    for(ct=0,j=0;r;ct++){
        while(j<n&&R[ind[j]]==ct) a[qr++]=ind[j++];
        if(cur==-1&&ql<qr) cur=a[ql++];
        if(cur==-1) rid[ct]=-1;
        else{
            rid[ct]=cur;
            if((--T[cur])==0){
                cur=-1;
                r--;
            }
        }
    }
}

void RR(){
    int j,sz=0,cur=-1,q=0,r=n;
    for(ct=0,j=0;r;ct++){
        while(j<n&&R[ind[j]]==ct) a[sz++]=ind[j++];
        if(cur==-1&&sz){
            cur=a[0]; q=500;
        }
        if(cur==-1) rid[ct]=-1;
        else{
            rid[ct]=cur;
            --T[cur]; --q;
            if(T[cur]==0||q==0){
                for(int k=1;k<sz;k++)
                    a[k-1]=a[k];
                if(T[cur]) a[sz-1]=cur;
                else r--,sz--;
                cur=-1;
            }
        }
    }
}

void SJF(){
    int j,ql=0,qr=0,cur=-1,r=n;
    for(ct=0,j=0;r;ct++){
        while(j<n&&R[ind[j]]==ct){
            int k=qr-1;
            while(k>=ql&&T[ind[j]]<T[a[k]]){
                a[k+1]=a[k];
                k--;
            }
            a[k+1]=ind[j++];
            qr++;
        }
        if(cur==-1&&ql<qr) cur=a[ql++];
        if(cur==-1) rid[ct]=-1;
        else{
            rid[ct]=cur;
            if((--T[cur])==0){
                cur=-1;
                r--;
            }
        }
    }
}

void PSJF(){
    int j,ql=0,qr=0,cur=-1,r=n;
    for(ct=0,j=0;r;ct++){
        while(j<n&&R[ind[j]]==ct){
            int k=qr-1;
            while(k>=ql&&T[ind[j]]<T[a[k]]){
                a[k+1]=a[k];
                k--;
            }
            a[k+1]=ind[j++];
            if(k+1==ql&&cur!=-1&&T[a[ql]]<T[cur]){
                cur^=a[ql];
                a[ql]^=cur;
                cur^=a[ql];
            }
            qr++;
        }
        if(cur==-1&&ql<qr) cur=a[ql++];
        if(cur==-1) rid[ct]=-1;
        else{
            rid[ct]=cur;
            if((--T[cur])==0){
                cur=-1;
                r--;
            }
        }
    }
}

int main(){
    init();
    if(strcmp(s,"FIFO")==0) FIFO();
    else if(strcmp(s,"RR")==0) RR();
    else if(strcmp(s,"SJF")==0) SJF();
    else if(strcmp(s,"PSJF")==0) PSJF();
    else assert(0);
    simulate();
    finish();
    return 0;
}
