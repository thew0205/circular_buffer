#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

enum CircularBufferError
{
    CIRCULAR_BUFFER_OK = 0,
    CIRCULAR_BUFFER_FULL,
    CIRCULAR_BUFFER_EMPTY
};

template <size_t bufferCapacity = 10>
class CircularBuffer
{
public:
    CircularBuffer()
    {
        memset(buffer_, 0, sizeof(buffer_));
    }

    size_t capacity() const
    {
        return bufferCapacity;
    }

    size_t size() const
    {
        if (tail_ > head_)
        {
            return tail_ - head_;
        }
        else if (tail_ == head_)
        {
            return isFull_ ? bufferCapacity : 0;
        }
        else
        {
            return bufferCapacity + tail_ - head_;
        }
    }

    CircularBufferError push(uint8_t value)
    {
        if (!hasSpace())
        {
            return CIRCULAR_BUFFER_FULL;
        }

        buffer_[tail_] = value;
        advance(&tail_);
        isFull_ = (head_ == tail_);
        return CIRCULAR_BUFFER_OK;
    }
    CircularBufferError pop(int &value)
    {
        if (isEmpty())
        {
            return CIRCULAR_BUFFER_EMPTY;
        }
        value = buffer_[head_];
        advance(&head_);
        isFull_ = false;
        return CIRCULAR_BUFFER_OK;
    }

    bool isEmpty() const
    {
        return (!isFull_ && (head_ == tail_));
    }

    bool hasSpace() const
    {
        return !isFull();
    }
    bool isFull() const
    {
        return isFull_;
    }

private:
    void advance(int *pointer)
    {
        *pointer = (*pointer + 1) % bufferCapacity;
    }
    uint8_t buffer_[bufferCapacity];

    int head_ = 0;
    int tail_ = 0;
    bool isFull_ = false;
};