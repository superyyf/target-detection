#ifndef CQUEUE_HPP
#define CQUEUE_HPP
#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

/**
 * @class   Queue CQueue.h Code\inc\CQueue.h
 *
 * @brief   线程安全队列实现
 * *        因为有std::mutex和std::condition_variable类成员,所以此类不支持复制构造函数也不支持赋值操作符(=)
 *
 * @author  IRIS_Chen
 * @date    2019/10/10
 *
 * @tparam  T   Generic type parameter.
 */
template <class T>

/**
 * @class   CQueue CQueue.h Code\inc\CQueue.h
 *
 * @brief   Queue of cs.
 *
 * @author  IRIS_Chen
 * @date    2019/10/17
 */

class CQueue
{
    protected:
    // Data
    std::queue<T> _queue;   ///< 存储数据的真实队列, 不是线程安全的
    private:
    typename std::queue<T>::size_type _size_max;    ///< 队列的最大长度
    // Thread gubbins
    std::mutex _mutex;  ///<  线程操作 锁
    std::condition_variable _fullQue;   ///< 队列满了的信号 
    std::condition_variable _empty; ///< 队列为空的信号

    // Exit
    // 原子操作
    std::atomic_bool _quit; ///< { false };     // 退出信号
    std::atomic_bool _finished; ///< { false }; // 完成信号 // 表示不再继续输入数据

    public:

    /**
     * @fn  CQueue::CQueue(const size_t size_max)
     *
     * @brief   初始化队列长度,并将退出标志和 满信号标志置空
     *
     * @author  IRIS_Chen
     * @date    2019/10/17
     *
     * @param   size_max    队列的最长尺寸
     */

    CQueue(const size_t size_max) :_size_max(size_max) {
        _quit = ATOMIC_VAR_INIT(false);
        _finished = ATOMIC_VAR_INIT(false);
    }

    /**
     * @fn  CQueue::CQueue(CONST CQueue&) = delete;
     *
     * @brief   不允许拷贝构造函数
     *
     * @author  IRIS_Chen
     * @date    2019/10/17
     *
     * @param   parameter1  The first parameter
     */

    CQueue(CONST CQueue&) = delete; ///< 不允许拷贝构造函数
    /**
     * @fn  CQueue::~CQueue()
     *
     * @brief   Finalizes an instance of the CQueue class  销毁队列, 退出线程 清除数据 // 存在问题
     *
     * @author  IRIS_Chen
     * @date    2019/11/8
     */

    ~CQueue()
    {
        Quit();
        while (_queue.size())
            ;
    }
    /**
     * @fn  bool CQueue::Push(T& data)
     *
     * @brief   队列中加入新的 对象  根据情况决定 满信号之后 新数据丢弃或者等待
     *
     * @author  IRIS_Chen
     * @date    2019/10/10
     *
     * @param [in,out]  data    The data to Push.
     *
     * @return  True if it succeeds, false if it fails.
     */

    bool Push(T& data)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_quit && !_finished)
        {
            if (_queue.size() < _size_max)
            {
                _queue.push(std::move(data));
                //_queue.Push(data);
                _empty.notify_all();
                return true;
            }
            else
            {
                // wait的时候自动释放锁，如果wait到了会获取锁
                // _fullQue.wait(lock);
                return false;   ///< 如果满了 这里不进行等待 避免出现问题
            }
        }

        return false;
    }

    /**
     * @fn  bool CQueue::Pop(T &data)
     *
     * @brief   返回队列最前面的元素 并且弹出 // 如果空 如果finish 则直接返回fasle 否则 等待队列加入元素
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     *
     * @param [in,out]  data    The data to Pop.
     *
     * @return  True if it succeeds, false if it fails.
     */

    bool Pop(T &data)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_quit)
        {
            if (!_queue.empty())                // 队列非空
            {
                //data = std::move(_queue.front());
                data = _queue.front();
                _queue.pop();

                _fullQue.notify_all();       // 通知所有 由于满队无法加入的线程
                return true;
            }
            else if (_queue.empty() && _finished)   // 队列为空 且不再加入
            {
                return false;
            }
            else
            {
                // _empty.wait(lock);          // 等待队列加入元素
                return false;   ///< 不等待元素加入数据
            }
        }
        return false;
    }

    /**
     * @fn  std::shared_ptr<T> CQueue::Pop(void)
     *
     * @brief   弹出一个元素 直接返回  出错无法报错
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     *
     * @return  The previous top-of-stack object.
     */

    std::shared_ptr<T> Pop(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        std::shared_ptr<T> res = nullptr;
        while (!_quit)
        {
            if (!_queue.empty())                // 队列非空
            {
                //data = std::move(_queue.front());
                res = std::make_shared<T>(_queue.front());
                _queue.pop();

                _fullQue.notify_all();       // 通知所有 由于满队无法加入的线程

                return res;
            }
            else if (_queue.empty() && _finished)   // 队列为空 且不再加入
            {
                return res;     // 无数据进入 智能返回一个空指针 (可能出错)
            }
            else
            {
                _empty.wait(lock);          // 等待队列加入元素
            }
        }
        return false;
    }

    /**
     * @fn  void CQueue::Finished()
     *
     * @brief   The queue has Finished accepting input 标识队列完成输入 不再继续输入
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     */

    void Finished()
    {
        _finished = true;
        _empty.notify_all();
    }

    /**
     * @fn  void CQueue::Quit()
     *
     * @brief   Quits this CQueue  退出队列, 无法再加入压入或者弹出数据
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     */

    void Quit()
    {
        _quit = true;
        _empty.notify_all();
        _fullQue.notify_all();
    }

    /**
     * @fn  int CQueue::Length()
     *
     * @brief   Gets the Length  返回队列目前长度
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     *
     * @return  An int.
     */

    int Length()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return static_cast<int>(_queue.size());
    }

    /**
     * @fn  int CQueue::Size()
     *
     * @brief   Gets the Size 返回当前队列长度
     *
     * @author  IRIS_Chen
     * @date    2019/10/14
     *
     * @return  An int.
     */
    int Size()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return static_cast<int>(_queue.size());
    }

    /**
     * @fn  bool CQueue::empty(void)
     *
     * @brief   判断是否为空
     *
     * @author  IRIS_Chen
     * @date    2019/10/17
     *
     * @return  True if it succeeds, false if it fails
     */

    bool Empty(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return (0 == _queue.size());
    }

    /**
     * @fn  bool CQueue::Clear(void)
     *
     * @brief   清空队列
     *
     * @author  IRIS_Chen
     * @date    2019/10/17
     *
     * @return  True if it succeeds, false if it fails
     */

    bool Clear(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_queue.empty ())
        {
            Pop();  // 依次弹出数据
        }
        return true;
    }
};
#endif
