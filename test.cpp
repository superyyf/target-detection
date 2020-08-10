#include "queue.hpp"
#include<unistd.h>

typedef void *(*THREAD_FUNC)(void *);

pthread_spinlock_t stop_lock;
bool stop;

// 将生成的数据包装到结构体中
// 比如这里的四个数字
struct Data {
    int a;
};

void *generate_data_thread(Queue<Data> *q) {
    Data data;
    int i = 0;
    while (true) {
        pthread_spin_lock(&stop_lock);
        if (stop)
            return NULL;
        pthread_spin_unlock(&stop_lock);
        data.a = i++;
        q->push(std::move(data));
        sleep(1);
    }
    return NULL;
}

void *handle_data_thread(Queue<Data> *q) {
    Data *data;
    while (true) {
        pthread_spin_lock(&stop_lock);
        if (stop)
            return NULL;
        pthread_spin_unlock(&stop_lock);
        data = q->pop();
        if (!data)
            break;
        // 处理过程
        // 例如保存到对应文件
        // fwrite(...)
        printf("%d\n", data->a);
        delete data;
    }
    return NULL;
}

int main(void) {
    Queue<Data> q;
    pthread_spin_init(&stop_lock, PTHREAD_PROCESS_PRIVATE);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, (THREAD_FUNC)generate_data_thread, &q);
    pthread_create(&t2, NULL, (THREAD_FUNC)handle_data_thread, &q);
    //其他处理过程
    //...
    sleep(5);
    pthread_spin_lock(&stop_lock);
    stop = true;
    pthread_spin_unlock(&stop_lock);
    q.cancel();
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_spin_destroy(&stop_lock);
    return 0;
}
