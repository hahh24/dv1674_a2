/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Baseline storage: Each R/G/B array uses index y * x_size + x.
// Pixel accessors provide writes; channel getters provide read-only access.

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

    unsigned char r(unsigned x, unsigned y) const;
    unsigned char g(unsigned x, unsigned y) const;
    unsigned char b(unsigned x, unsigned y) const;
    unsigned char& r(unsigned x, unsigned y);
    unsigned char& g(unsigned x, unsigned y);
    unsigned char& b(unsigned x, unsigned y);
};

#endif