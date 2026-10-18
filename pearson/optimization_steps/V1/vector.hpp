/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 borrows dot input and makes value operators const.

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
    // V1: Previously rhs was copied. We now borrow it read-only to avoid that
    // copy; rhs must stay alive and contain at least as many elements as this vector.
    double dot(const Vector& rhs) const;

    unsigned get_size() const;
    double* get_data();

    // V1: Division and subtraction still return modified copies. Adding const
    // permits read-only use because the source is unchanged; const itself is
    // not a separately proven speed optimization.
    Vector operator/(double div) const;
    Vector operator-(double sub) const;
    double operator[](unsigned i) const;
    double& operator[](unsigned i);
};

#endif
