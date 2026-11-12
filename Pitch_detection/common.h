#ifndef COMMON_H
#define COMMON_H
//  Complex numbers implementation comes from Rob Tillaart's Arduino library.
// VERSION: 0.3.5
//    DATE: 2013-09-23
//     URL: https://github.com/RobTillaart/Complex
//          http://arduino.cc/playground/Main/ComplexMath
//  Due to the author's warnigs regarding the size of original library, this implementation contains only necessary functions.
#include <Arduino.h>
#include "Printable.h"
#include <math.h>
class Complex: public Printable
{
public:
    Complex(const float r = 0, const float i = 0) : re(r), im(i) {};
    Complex(const Complex &c) : re(c.re), im(c.im) {};


    void set(const float r, const float i ) { re = r; im = i; };
    void setReal(const float r)  { re = r; };
    void setImag(const float i ) { im = i; };
    float real() const { return re; };
    float imag() const { return im; };

    size_t printTo(Print& p) const;

    float modulus() const     { return hypot(re, im); };
    Complex conjugate() const { return Complex(re, -im); };

    bool operator == (const Complex&) const;
    bool operator != (const Complex&) const;


    Complex operator - () const; // negation


    Complex operator + (const Complex&) const;
    Complex operator - (const Complex&) const;
    Complex operator * (const Complex&) const;
    Complex operator / (const Complex&) const;

    Complex& operator += (const Complex&);
    Complex& operator -= (const Complex&);
    Complex& operator *= (const Complex&);
    Complex& operator /= (const Complex&);

    Complex c_exp() const;

  protected:
    float re;
    float im;

    Complex gonioHelper1(const byte) const;
    Complex gonioHelper2(const byte) const;
};

static inline Complex* allocate_complex_array(int n) {
    return (Complex*)calloc(n, sizeof(Complex));
}

#endif