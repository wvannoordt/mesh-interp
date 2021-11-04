#ifndef V3_H
#define V3_H
template <typename T> struct v3
{
    T data[3];
    
    v3(T t1, T t2, T t3)
    {
        data[0] = t1;
        data[1] = t2;
        data[2] = t3;
    }
    
    v3(void)
    {
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
    }
    
    v3(const v3<T> & rhs)
    {
        data[0] = rhs.data[0];
        data[1] = rhs.data[1];
        data[2] = rhs.data[2];
    }
    
    T& operator [] (const int i) {return data[i];}
    
    T& operator () (const int i) {return data[i];}
    
    v3<T>& operator = (const T val)
    {
        data[0] = val;
        data[1] = val;
        data[2] = val;
    }
};

#endif