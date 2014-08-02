#include <nds.h>

#ifndef Fixed_H
#define Fixed_H

namespace gx {

//note: in the interest of speed, most of these functions
//are inlined.

//T *must* be an integral type. It's purpose for templating
//is simply to function on ints of any size.

template <typename T, int F>
class Fixed {
    template<typename T2, int F2>
    friend class Fixed;

    public:
        Fixed() {
            data = 0;
        }
        
        ~Fixed() {}
        
        //assignment from other types
        Fixed(const int& other) {*this = other;}
        Fixed<T, F>& operator=(const int& other) {data = other << F; return *this;}
        
        //comparison
        bool operator==(const Fixed<T, F>& other) const {return data == other.data;}
        bool operator< (const Fixed<T, F>& other) const {return data <  other.data;}
        bool operator> (const Fixed<T, F>& other) const {return data >  other.data;}
        bool operator<=(const Fixed<T, F>& other) const {return data <= other.data;}
        bool operator>=(const Fixed<T, F>& other) const {return data >= other.data;}
        bool operator!=(const Fixed<T, F>& other) const {return !(data == other.data);}
        
        //comparison with ints
        bool operator==(const int& other) const {return data == other << F;}
        bool operator< (const int& other) const {return data <  other << F;}
        bool operator> (const int& other) const {return data >  other << F;}
        bool operator<=(const int& other) const {return data <= other << F;}
        bool operator>=(const int& other) const {return data >= other << F;}
        bool operator!=(const int& other) const {return !(data == other << F);}
        
        //comparison with floats
        bool operator==(const float& other) const {return data == (Fixed<T, F>)other;}
        bool operator< (const float& other) const {return data <  (Fixed<T, F>)other;}
        bool operator> (const float& other) const {return data >  (Fixed<T, F>)other;}
        bool operator<=(const float& other) const {return data <= (Fixed<T, F>)other;}
        bool operator>=(const float& other) const {return data >= (Fixed<T, F>)other;}
        bool operator!=(const float& other) const {return !(data == (Fixed<T, F>)other);}
        
        
        //Add / Subtract
        Fixed<T, F>  operator+ (const Fixed<T, F>& other) {Fixed<T,F> r; r.data = data + other.data; return r;}
        Fixed<T, F>& operator+=(const Fixed<T, F>& other) {data += other.data; return *this;}       
        Fixed<T, F>  operator- (const Fixed<T, F>& other) {Fixed<T,F> r; r.data = data - other.data; return r;}
        Fixed<T, F>& operator-=(const Fixed<T, F>& other) {data -= other.data; return *this;}
        
        //Multiply / Divide
        Fixed<T, F>  operator* (const Fixed<T, F>& other) {Fixed<T,F> r; r.data = (data * other.data) >> F; return r;}
        Fixed<T, F>& operator*=(const Fixed<T, F>& other) {data = (data * other.data) >> F; return *this;}
        Fixed<T, F>  operator/ (const Fixed<T, F>& other) {Fixed<T,F> r; r.data = (data << F) / (other.data); return r;}
        Fixed<T, F>& operator/=(const Fixed<T, F>& other) {data = (data << F) / (other.data); return *this;}
        
        //type conversion
        operator int() const {return data >> F;}
        operator float() const {return ((float)data) / (float)(1 << F);}
        
        //voodoo magic
        template <typename T2, int F2>
        Fixed(const Fixed<T2, F2>& other) {*this = other;}

        template <typename T2, int F2>
        Fixed<T, F>& operator=(const Fixed<T2, F2>& other) {
            data = (
                F > F2 ?
                    other.data<<AbsoluteDifference<F, F2>::value :
                    other.data>>AbsoluteDifference<F, F2>::value
            );
            return *this;
        }

        T data;
        
    private:        
        //What is this I don't even
        
        //This voodoo here is to make the compiler shush-- this metafunction
        //returns the absolute value of the *difference* of V and V2, which we
        //can use to ensure that a shift is always compiled with a non-negative
        //value, silencing annoying compiler warnings.
        template<int V, int V2>
        struct AbsoluteDifference {
            enum {
                value = AbsoluteDifference<V - 1, V2 - 1>::value
            };
        };

        template<int V>
        struct AbsoluteDifference<V,0> {
            enum {
                value = V
            };
        };

        template<int V2>
        struct AbsoluteDifference<0,V2> {
            enum {
                value = V2
            };
        };
};

template <typename T, int F>
Fixed<T, F> sqrt(Fixed<T, F> value)
{
    //note: this is DS specific currently. Not sure if it needs to be ported
    //to other platforms. NOT
    
    
}

}

#endif