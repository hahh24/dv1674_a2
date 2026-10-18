/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V2 adds writable channel getters for direct blur access.
// Existing read-only getters and pixel accessors remain available.

#include <iostream>

#if !defined(MATRIX_HPP)
#define MATRIX_HPP

class Matrix {
private:
    unsigned char* R;
    unsigned char* G;
    unsigned char* B;

    unsigned x_size;
    unsigned y_size;
    unsigned color_max;

public:
    Matrix();
    Matrix(unsigned dimension);
    Matrix(const Matrix& other);
    Matrix(unsigned char* R, unsigned char* G, unsigned char* B, unsigned x_size, unsigned y_size, unsigned color_max);
    Matrix& operator=(const Matrix other);
    ~Matrix();

    unsigned get_x_size() const;
    unsigned get_y_size() const;
    unsigned get_color_max() const;

    unsigned char const* get_R() const;
    unsigned char const* get_G() const;
    unsigned char const* get_B() const;

    // V2: Previously channel getters were read-only; writes used r/g/b.
    // These three overloads expose existing writable arrays without copying.
    // They support shared row/neighbor indices in blur, reducing repeated accessor
    // work. Callers must use this matrix's own width, stay within its allocation,
    // and keep the matrix alive; these pointers do not transfer ownership.
    unsigned char* get_R();
    unsigned char* get_G();
    unsigned char* get_B();

    unsigned char r(unsigned x, unsigned y) const;
    unsigned char g(unsigned x, unsigned y) const;
    unsigned char b(unsigned x, unsigned y) const;
    unsigned char& r(unsigned x, unsigned y);
    unsigned char& g(unsigned x, unsigned y);
    unsigned char& b(unsigned x, unsigned y);
};

#endif