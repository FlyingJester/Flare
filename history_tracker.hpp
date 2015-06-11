#pragma once
//! @file
//! @brief     A simple container to store history while erasing old entries to conserve space
//! @author    FlyingJester
//! @date      2015
//! @copyright BSD 3-Clause License

#include <deque>
#include <string>
#include <algorithm>
#include <numeric>

namespace Pluto {

//! See the src/benchmark/history_tracker directory for some speed and memory demo programs.
//! In general, for simply adding to the tracker and not erasing, it should be faster than
//! a list but slower than a vector. So pretty close to a raw std::deque.
//! The real benefit is in the size checking it does.
template<class T, void(*Deleter)(T), size_t (*Sizer)(size_t, T), size_t max_size = 0xFFFF>
class HistoryTracker {
    
    std::deque<T> stack;
    unsigned stack_size;

public:

    ~HistoryTracker(){
        std::for_each(stack.begin(), stack.end(), Deleter);
    }
    
    typedef T value_type;
    typedef value_type& reference;
    typedef value_type const & const_reference;
    typedef unsigned long size_type;

    bool empty() const {
        return stack.empty();
    }

    size_t size() const {
        return stack.size();
    }

    T &back() {
        return stack.back();
    }

    void push_back(T item){
        stack_size += Sizer(0, item);

        while((!empty()) && stack_size>max_size){
            T that = stack.front();
            stack_size -= Sizer(0, that);
            Deleter(that);
            stack.pop_front();
        }

        stack.push_back(item);

    }
    void pop_back(){
        stack_size -= Sizer(0, stack.back());
        stack.pop_back();
    }

    T pop() {
        T const s = back();
        pop_back();
        return s;
    }

    T operator[] (size_type i) const { return stack[i]; }

    void clearFrom(size_type i) {
        typename std::deque<T>::iterator from = (stack.rbegin()+i).base();
        stack_size -= std::accumulate(from, stack.end(), 0u, Sizer);
        
        std::for_each(from, stack.end(), Deleter);
        stack.erase(from, stack.end());
    }

    void clear() { 
        std::for_each(stack.begin(), stack.end(), Deleter);
        stack_size = 0;
        stack.clear();
    }

};

}
