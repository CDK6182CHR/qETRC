#pragma once

#include <iostream>

namespace xtl
{
template <typename T>
class matrix {
    int row, col;
    T* data;
public:
    matrix(int row_, int col_);
    ~matrix();
    T& get(int i, int j);
    T* get_pointer(int i, int j);
    inline T* get_pointer_serial(int n) {
        return data + n;
    }
    inline T& get_serial(int n) {
        return *(data + n);
    }
    inline int get_size()const {
        return row * col;
    }
    inline const T& get(int i, int j)const {
        return const_cast<matrix*>(this)->get(i, j);
    }
    inline int get_row()const {
        return row;
    }
    inline int get_col()const {
        return col;
    }
    inline int index_to_serial(int i, int j)const {
        return i * col + j;
    }
    void index_to_rowcol(int n, int& i, int& j)const;
    void fill(const T& d);

    matrix(const matrix&) = delete;
    matrix(matrix&&) = delete;
    matrix& operator=(const matrix&) = delete;
    matrix& operator=(matrix&&) = delete;

    void show()const;
};

template<typename T>
matrix<T>::matrix(int row_, int col_) :row(row_), col(col_)
{
    if(int n=row*col)
        data = new T[n];
    else data=nullptr;
}

template<typename T>
matrix<T>::~matrix()
{
    delete[] data;
    data = nullptr;
}

template<typename T>
T& matrix<T>::get(int i, int j)
{
    return data[i * col + j];
}

template<typename T>
T* matrix<T>::get_pointer(int i, int j)
{
    return data + i * col + j;
}

template<typename T>
void matrix<T>::index_to_rowcol(int n, int& i, int& j) const
{
    j = n % col;
    i = n / col;   //int div
}

template<typename T>
void matrix<T>::fill(const T& d)
{
    std::fill(data, data + row * col, d);
}

template<typename T>
void matrix<T>::show() const
{
    std::cout << '[' << std::endl;
    for (int i = 0; i < row; i++) {
        std::cout << ' ';
        for (int j = 0; j < col; j++) {
            std::cout << get(i, j) << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << ']' << std::endl;
}


}
