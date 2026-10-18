/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Baseline: Vector copies own separate data; dot takes its input by value.

#if !defined(VECTOR_HPP)
#define VECTOR_HPP

class Vector {
private:
    unsigned size;
    double* data;

public:
    Vector();
    Vector(unsigned size);
    Vector(unsigned size, double* data);
    Vector(const Vector& other);
    ~Vector();

    double magnitude() const;
    double mean() const;
    double normalize() const;
    // Baseline: Passing rhs by value makes a deep copy before this dot product.
    double dot(Vector rhs) const;

    unsigned get_size() const;
    double* get_data();

    // Baseline: Division and subtraction copy this Vector, then change the copy.
    Vector operator/(double div);
    Vector operator-(double sub);
    double operator[](unsigned i) const;
    double& operator[](unsigned i);
};

#endif