#ifndef QUEUE_HPP
#define QUEUE_HPP 1

#include <vector>
#include <memory>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

template<typename T>
struct Queue {
    typedef T *value_type;
    T **buf;
    unsigned s, t;
    unsigned cap, size;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool canceled;
private:
    void _push(T *data) {
        pthread_mutex_lock(&lock);
        if (size == cap) {
            assert(s == t);
            auto new_cap = (cap << 1);
            auto new_buf = new value_type[new_cap];
            memmove(new_buf, buf + t, sizeof(value_type) * (cap - t));
            if (t)
                memmove(new_buf + (cap - t), buf, t * sizeof(value_type));
            t = 0;
            s = cap;
            cap = new_cap;
            delete buf;
            buf = new_buf;
        }
        buf[s++] = data;
        if (s == cap)
            s = 0;
        ++size;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
public:

    Queue(unsigned default_size = 1024) {
        buf = new value_type[default_size];
        s = t = size = 0;
        cap = default_size;
        canceled = false;
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
    }

    std::unique_ptr<T> pop() {
        pthread_mutex_lock(&lock);
        while (!size && !canceled)
            pthread_cond_wait(&cond, &lock);
        if (canceled) {
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        value_type ret = buf[t];
        buf[t++] = NULL;
        if (t == cap)
            t = 0;
        --size;
        pthread_mutex_unlock(&lock);
        return std::unique_ptr<T>(ret);
    }

    void push(T &&data) {
        T *val = new T;
        *val = data;
        _push(val);
    }

    void end() {
        _push(NULL);
    }

    ~Queue() {
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
        while (size) {
            auto val = pop();
            delete val;
        }
        delete buf;
    }
};
#endif /* ifndef QUEUE_HPP */
