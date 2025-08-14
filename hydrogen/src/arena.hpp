#pragma once

#include <iostream>

class ArenaAllocator
{
private:
    size_t m_bytes;
    void* m_buffer;
    void* m_offset;
public:
    inline ArenaAllocator(size_t bytes)
        :m_bytes(std::move(bytes))
    {
        m_buffer = malloc(m_bytes);
        m_offset = m_buffer;
    }
    
    template<typename T>
    inline T* alloc(){
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    inline ArenaAllocator(const ArenaAllocator& other) = delete;

    inline ArenaAllocator operator=(const ArenaAllocator& other) = delete;

    inline ~ArenaAllocator(){
        free(m_buffer);
    }
};

