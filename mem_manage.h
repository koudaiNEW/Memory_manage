
// 模板类，实现一个固定内存管理队列
// 1. 实现功能
// 1.1 从队列头和队列尾添加元素
// 1.2 从队列头和队列尾部删除元素
// 1.3 获取队列第N个元素
// 1.4 给出当前队列长度
// 2. 实现要求
// 2.1 FixMemory， 使用固定内存实现上述功能
// 2.2 线程安全

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

template <class T>
class mem_queue {
  private:
    bool init_success = false;
    int list_max = 0;
    int list_count = 0;
    T * list = nullptr; 
    pthread_spinlock_t * lock = nullptr; 

 public:
  //***************************************************************
  //*  @brief      构造函数
  //*  @param[in]  cout, 指定队列空间最大长度
  //*  @note       在构造函数中分配内存空间
  //*  @Sample:    mem_queue queue(10);  // 创建一个最多10个元素的队列
  //**************************************************************/
  mem_queue() {
    if(pthread_spin_init(lock, PTHREAD_PROCESS_SHARED) != 0){
      std::cout << "[mem_queue] Init, lock init failed" << std::endl;
    }
  }

  //***************************************************************
  //*  @brief      析构函数
  //*  @note       回收资源
  //**************************************************************/
  ~mem_queue() {
    if(list != nullptr){
      free(list);
    }
    if(lock != nullptr){
      pthread_spin_destroy(lock);
    }
  }

  //***************************************************************
  //*  @brief          队列初始化
  //*  @param[in]      cout, 指定队列空间最大长度
  //*  @note           失败返回false
  //**************************************************************/
  bool Init(int count = 10) {
    pthread_spin_lock(lock);
    if(init_success){
      std::cout << "[mem_queue] Init, reintialize" << std::endl;
      goto out;
    }
    if(count <= 0){
      std::cout << "[mem_queue] Init, list length error, [in]count: " << std::dec << count << std::endl;
      goto out;
    }
    list_max = count;
    list = (T *)malloc(sizeof(T) * list_max);
    if(list == nullptr){
      std::cout << "[mem_queue] Init, memory application failed" << std::endl;
      goto out;
    }
    init_success = true;
    out:
    pthread_spin_unlock(lock);
    return init_success;
  }

  //***************************************************************
  //*  @brief      在队列末尾插入一个元素
  //*  @param[in]  item, 需要加入的元素
  //*  @note       如果当前队列满，返回false，插入成功返回true
  //*  @Sample:    bool ret = queue.PushBack(item);
  //**************************************************************/
  bool PushBack(const T& item) {
    bool back = false;

    if(!init_success){
      std::cout << "[mem_queue] PushBack, list not initialized" << std::endl;
      return false;
    }
    pthread_spin_lock(lock);
    if(list_count >= list_max){
      std::cout << "[mem_queue] PushBack, list full" << std::endl;
    }else{
      list[list_count] = item;
      list_count++;
      back = true;
    }
    pthread_spin_unlock(lock);
    return back;
  }

  //***************************************************************
  //*  @brief      在队列头插入一个元素
  //*  @param[in]  item, 需要加入的元素
  //*  @note       如果当前队列满，返回false，插入成功返回true
  //*  @Sample:    bool ret = queue.PushFront(item);
  //**************************************************************/
  bool PushFront(const T& item) {
    bool back = false;

    if(!init_success){
      std::cout << "[mem_queue] PushFront, list not initialized" << std::endl;
      return false;
    }
    pthread_spin_lock(lock);
    if(list_count >= list_max){
      std::cout << "[mem_queue] PushBack, list full" << std::endl;
    }else{
      for(int i = list_count; i > 0; i--){
        list[i] = list[i - 1];
      }
      list[0] = item;
      list_count++;
      back = true;
    }
    pthread_spin_unlock(lock);
    return back;
  }

  //***************************************************************
  //*  @brief          将队列尾部元素删除并将数据写入item
  //*  @param[in/out]  item, 将pop出的数据写入item;
  //*  @note           如果当前队列为空，返回false
  //**************************************************************/
  bool PopBack(T* item) {
    bool back = false;

    if(!init_success){
      std::cout << "[mem_queue] PopBack, list not initialized" << std::endl;
      return false;
    }
    pthread_spin_lock(lock);
    if(!list_count){
      std::cout << "[mem_queue] PopBack, list empty" << std::endl;
    }else{
      *item = list[list_count - 1];
      list_count--;
      back = true;
    }
    pthread_spin_unlock(lock);
    return back;
  }

  //***************************************************************
  //*  @brief          将队列头部元素删除并将数据写入item
  //*  @param[in/out]  item, 将pop出的数据写入item;
  //*  @note           如果当前队列为空，返回false
  //**************************************************************/
  bool PopFront(T* item) {
    bool back = false;

    if(!init_success){
      std::cout << "[mem_queue] PopFront, list not initialized" << std::endl;
      return false;
    }
    pthread_spin_lock(lock);
    if(!list_count){
      std::cout << "[mem_queue] PopBack, list empty" << std::endl;
    }else{
      *item = list[0];
      list_count--;
      for(int i = 0; i < list_count; i++){
        list[i] = list[i + 1];
      }
      back = true;
    }
    pthread_spin_unlock(lock);
    return back;
  }

  //***************************************************************
  //*  @brief          获取第index个元素的值
  //*  @param[in]      index, 需要获取第几个元素的值
  //*  @param[in/out]  item， 将值写入item
  //*  @note           如果当前队列长度小于index，返回false
  //**************************************************************/
  bool GetItem(int index, T* item) {
    bool back = false;

    if(!init_success){
      std::cout << "[mem_queue] GetItem, list not initialized" << std::endl;
      return false;
    }
    pthread_spin_lock(lock);
    if(index > list_count || index <= 0){
      std::cout << "[mem_queue] GetItem, [in]index out of range" << std::endl;
    }else{
      *item = list[index - 1];
      back = true;
    }
    pthread_spin_unlock(lock);
    return back;
  }

  //***************************************************************
  //*  @brief          返回当前队列长度
  //*  @Sample         int len = queue.Count();
  //**************************************************************/
  int Count() {
    int back;

    pthread_spin_lock(lock);
    back = list_count;
    pthread_spin_unlock(lock);
    return back;
  }
};

