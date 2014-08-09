/** @file locklessque.h

    @brief Lockless 2-thread producer/consumer que

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

/*
    some references:
    http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448
    http://www.ogre3d.org/addonforums/viewtopic.php?f=19&t=10975&start=0
    http://www.cplusplus.com/forum/general/76068/
    http://cbloomrants.blogspot.ca/2009/01/01-25-09-low-level-threading-junk-part.html
*/

#ifndef MOSRC_TOOL_LOCKLESSQUE_H
#define MOSRC_TOOL_LOCKLESSQUE_H

#include <atomic>

namespace MO {


template <class T>
class LocklessQueue
{
    struct Node_
    {
        Node_(T val) : value(val), next(nullptr) { }
        T value;
        Node_ * next;
    };

    // for producer only
    Node_ * first_;
    // shared
    std::atomic<Node_*> divider_, last_;
    std::atomic<unsigned int> count_;

public:

    LocklessQueue()
    {
        // add dummy separator
        first_ = divider_ = last_ = new Node_( T() );
        count_.store(0);
    }

    ~LocklessQueue()
    {
        // release the list
        while( first_ != nullptr )
        {
            Node_ * tmp = first_;
            first_ = tmp->next;
            delete tmp;
        }
    }

    void reset()
    {
        // release the list
        while( first_ != nullptr )
        {
            Node_ * tmp = first_;
            first_ = tmp->next;
            delete tmp;
        }
        // add dummy separator
        first_ = divider_ = last_ = new Node_( T() );
        count_.store(0);
    }

    unsigned int count() const { return count_.load(); }

    /** Pushes data on the queue */
    void produce(const T & t)
    {
        // add the new item
        (*last_).next = new Node_(t);
        // publish it
        last_ = (*last_).next;
        count_++;
        // trim unused nodes
        while( first_ != divider_ )
        {
            Node_ * tmp = first_;
            first_ = first_->next;
            delete tmp;
        }
    }

    /** Gets data from the que if available */
    bool consume(T & result)
    {
        // if queue is nonempty
        if( divider_ != last_ )
        {
            // copy it back
            result = (*divider_).next->value;
            // publish that we took it
            divider_ = (*divider_).next;
            count_--;
            return true;
        }
        // report empty
        return false;
    }
};


#if (0)
template <class Type>
class LocklessQueue
{
public:

    LocklessQueue(uint size)
        : write_ (0),
          read_ (0),
          size_ (nextPowerOfTwo(size)),
          mask_ (size_-1)
    {
        buffer_ = new Type[size_];
    }

    ~LocklessQueue()
    {
        delete [] m_buffer;
    }

    //! push object into the queue.
    bool push(const Type& obj)
    {
        uint next_write = (write_ + 1) & mask_;
        if (next_write == read_)
            return false;

        buffer_[write_] = obj;
        write_ = next_write;

        return true;
    }

    bool pop(Type& obj)
    {
      if (read_ == write_)
          return false;

      obj = buffer_[read_];
      read_ = (read_ + 1) & mask_;

      return true;
    }

private:
   //! buffer to keep the queue.
   Type* buffer_;

   //! head of queue list.
   uint write_, read_, size_, mask_;
};
#endif

} // namespace MO

#endif // MOSRC_TOOL_LOCKLESSQUE_H
