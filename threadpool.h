//
// Created by shanshan on 2022/4/26.
//

#ifndef MYSERVER_THREADPOOL_H
#define MYSERVER_THREADPOOL_H
#include <pthread.h>
#include <list>
#include "locker.h"
#include <exception>
#include <iostream>
template<typename T>
class threadpool{
public:
    threadpool(int thread_number=8,int max_request=10000);
    ~threadpool();
    bool append(T* request);
    void run();
private:
    static void* worker(void* arg);
private:
    //线程数
    int m_thread_number;
    //线程池数组，大小为线程数
    pthread_t * m_threads;
    //请求队列的最大容量
    int m_max_requests;
    //请求队列
    std::list<T*> m_workqueue;
    //互斥锁
    locker m_queuelocker;
    //信号量用来判断是否有任务要处理
    sem m_queuestat;
    //是否结束线程
    bool m_stop;
};
template<typename T>
threadpool<T>::threadpool(int thread_number, int max_request):m_thread_number(thread_number),m_max_requests(max_request),m_stop(false),m_threads(NULL) {
    if(thread_number<=0||max_request<=0) {
        throw std::exception();
    }
    m_threads=new pthread_t[m_thread_number];
    if(!m_threads){
        throw std::exception();
    }
    //创建线程并设置为线程脱离
    for(int i=0;i<thread_number;i++){
        std::cout<<"create the "<<i<<"th thread";
        if(pthread_create(m_threads+i,NULL,worker,NULL)){
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template <typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
    m_stop=true;
}
template<typename T>
bool threadpool<T>::append(T *request) {
    m_queuelocker.lock();
    if(m_workqueue.size()>m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}
template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool=(threadpool*) arg;
    pool->run();
    return pool;
}
template<typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        T* request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request){
            continue;
        }
        request->process();
    }
}
#endif //MYSERVER_THREADPOOL_H
