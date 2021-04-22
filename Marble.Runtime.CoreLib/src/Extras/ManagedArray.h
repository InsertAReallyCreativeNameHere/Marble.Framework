#pragma once

#include <inc.h>
#include <iterator>

namespace Marble
{
    template<typename T>
    struct coreapi ManagedArray final
    {
        ManagedArray(const uint& length = 0)
        {
            this->array = new T[length];
            this->arrlen = new uint;
            *this->arrlen = length;
        }
        ManagedArray(const uint& length, const T& fill)
        {
            this->array = new T[length];
            this->arrlen = new uint;
            *this->arrlen = length;

            std::fill(this->array, this->array + *this->arrlen, fill);
        }
        ManagedArray(const ManagedArray<T>& copyconstr)
        {
            this->arrlen = new uint;
            *this->arrlen = *copyconstr.arrlen;
            this->array = new T[*this->arrlen];
            for (uint i = 0; i < *this->arrlen; i++)
                this->array[i] = copyconstr.array[i];
        }
        ManagedArray<T>& operator=(const ManagedArray<T>& rhs)
        {
            delete[] this->array;
            *this->arrlen = *rhs.arrlen;
            this->array = new T[*this->arrlen];
            for (uint i = 0; i < *this->arrlen; i++)
                this->array[i] = rhs.array[i];
            return *this;
        }
        ~ManagedArray()
        {
            delete[] this->array;
            delete this->arrlen;
        }

        T& operator[](const uint& index)
        {
            return this->array[index];
        }
        uint length() const
        {
            return *this->arrlen;
        }

        T* data()
        {
            return this->array;
        }
        void reset(const uint& length)
        {
            delete[] this->array;
            *this->arrlen = length;
            this->array = new T[*this->arrlen];
        }
        void reset(const uint& length, const T& fill)
        {
            delete[] this->array;
            *this->arrlen = length;
            this->array = new T[*this->arrlen];
            
            #pragma omp parallel for default(shared) num_threads(2)
            for (uint i = 0; i < *this->arrlen; i++)
                this->array[i] = fill;
        }

        T* begin()
        {
            return this->array;
        }
        T* end()
        {
            return this->array[this->arrlen] + 1;
        }
    private:
        T* array = nullptr;
        uint* arrlen = nullptr;
    };
}
