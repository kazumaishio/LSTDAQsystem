#include <stdio.h>
#include <pthread.h>

#include <iostream>
#include <string>
#include <unistd.h>//for sleep()


typedef struct _thread_arg
{
  int thread_no;
  char *data;
  bool *is_read;
  pthread_mutex_t *mutex;
} thread_arg_t;

using namespace std;

void *Collector(void *arg)
{
  thread_arg_t* targ=(thread_arg_t *)arg;
  while(1)
  {
    char tempdata[128];
    cin >> tempdata;
    cin.clear();
    if(tempdata[0]=='x')break;
    //mutex lock
    pthread_mutex_lock(targ->mutex);
    while(!*(targ->is_read))//==false)
    {
      pthread_mutex_unlock(targ->mutex);
      sleep(1);
      pthread_mutex_lock(targ->mutex);
    }
    strcpy(targ->data,tempdata);
    *(targ->is_read)=false;
    //mutex unlock
    pthread_mutex_unlock(targ->mutex);
  }
}


void *Builder(void *arg)
{
  thread_arg_t* targ = (thread_arg_t *)arg;

  while(1)
  {
    char outdata[128];
    //mutex lock
    pthread_mutex_lock(targ->mutex);
    if(!*(targ->is_read))
    {
      sched_yield(); //スレッド切り替えチャンスを与える！！？
      strcpy(outdata,targ->data);
      *(targ->is_read)=true;
      pthread_mutex_unlock(targ->mutex);
      cout<< "Builder received :"<<outdata<<endl;
    }else{
      pthread_mutex_unlock(targ->mutex);
      sleep(1);
    }
  }
}


int main()
{
  pthread_t handle[2];
  thread_arg_t targ[2];
  char data[128];
  bool is_read;
  int i;
  pthread_mutex_t mutex;

  /*データの初期化*/
  strcpy(data,"");
  is_read=false;
  /*mutexハンドルの初期化*/
  pthread_mutex_init(&mutex, NULL);
  targ[0].thread_no = 0;
  targ[0].is_read = &is_read;
  targ[0].data = data;
  targ[0].mutex = &mutex;
  pthread_create(&handle[0],
                 NULL,
                 &Builder,
                 (void*)&targ[0]);
  targ[1].thread_no = 1;
  targ[1].is_read = &is_read;
  targ[1].data = data;
  targ[1].mutex = &mutex;
  pthread_create(&handle[1],
                 NULL,
                 &Collector,
                 (void*)&targ[1]);
  

  for(i=0;i<2;i++)
    pthread_join(handle[i],NULL);

  /*mutexハンドルの破棄*/
  pthread_mutex_destroy(&mutex);

  return 0;
}
