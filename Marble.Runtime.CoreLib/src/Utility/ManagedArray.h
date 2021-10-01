#pragma once

#include <algorithm>

namespace Marble
{
    template<typename T>
    struct ManagedArray final
    {
        inline ManagedArray(size_t length = 0) : array(new T[length]), arrlen(length)
        {
        }
        inline ManagedArray(size_t length, const T& fill) : array(new T[length]), arrlen(length)
        {
            std::fill(this->array, this->array + this->arrlen, fill);
        }
        inline ManagedArray(const ManagedArray<T>& other) : array(new T[other.arrlen]), arrlen(other.arrlen)
        {
            for (size_t i = 0; i < this->arrlen; i++)
                this->array[i] = other.array[i];
        }
        inline ManagedArray(ManagedArray<T>&& other) : array(other.array), arrlen(other.arrlen)
        {
            other.array = nullptr;
            other.arrlen = 0;
        }
        inline ManagedArray<T>& operator=(const ManagedArray<T>& other)
        {
            delete[] this->array;
            this->arrlen = other.arrlen;
            this->array = new T[this->arrlen];
            for (size_t i = 0; i < this->arrlen; i++)
                this->array[i] = other.array[i];
            return *this;
        }
        inline ManagedArray<T>& operator=(ManagedArray<T>&& other)
        {
            delete[] this->array;

            this->arrlen = other.arrlen;
            this->array = other.array;
            for (size_t i = 0; i < this->arrlen; i++)
                this->array[i] = other.array[i];

            other.array = nullptr;
            other.arrlen = 0;

            return *this;
        }
        inline ~ManagedArray()
        {
            delete[] this->array;
        }

        inline T& operator[](size_t index)
        {
            return this->array[index];
        }
        inline size_t length() const
        {
            return this->arrlen;
        }

        inline T* data()
        {
            return this->array;
        }
        inline void reset(size_t length)
        {
            delete[] this->array;
            this->arrlen = length;
            this->array = new T[this->arrlen];
        }
        inline void reset(size_t length, const T& fill)
        {
            delete[] this->array;
            this->arrlen = length;
            this->array = new T[this->arrlen];
            std::fill(this->array, this->array + this->arrlen, fill);
        }

        inline T* begin()
        {
            return this->array;
        }
        inline T* end()
        {
            return this->array + this->arrlen;
        }
        inline const T* cbegin() const
        {
            return this->array;
        }
        inline const T* cend() const
        {
            return this->array + this->arrlen;
        }
    private:
        T* array = nullptr;
        size_t arrlen;
    };
}
