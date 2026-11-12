#include "common.h"

size_t Complex::printTo(Print& p) const
{
  size_t n = 0;
  n += p.print(re, 3);
  n += p.print(' ');
  n += p.print(im, 3);
  n += p.print('i');
  return n;
};

bool Complex::operator == (const Complex &c) const
{
  return (re == c.re) && (im == c.im);
}

bool Complex::operator != (const Complex &c) const
{
  return (re != c.re) || (im != c.im);
}

//
//  NEGATE
//
Complex Complex::operator - () const
{
  return Complex(-re, -im);
}

Complex Complex::operator + (const Complex &c) const
{
  return Complex(re + c.re, im + c.im);
}

Complex Complex::operator - (const Complex &c) const
{
  return Complex(re - c.re, im - c.im);
}

Complex Complex::operator * (const Complex &c) const
{
  float r = re * c.re - im * c.im;
  float i = re * c.im + im * c.re;
  return Complex(r, i);
}

Complex Complex::operator / (const Complex &c) const
{
  float f = 1.0/(c.re * c.re + c.im * c.im);
  float r = (re * c.re + im * c.im) * f;
  float i = (im * c.re - re * c.im) * f;
  return Complex(r, i);
}

Complex& Complex::operator += (const Complex &c)
{
  re += c.re;
  im += c.im;
  return *this;
}

Complex& Complex::operator -= (const Complex &c)
{
  re -= c.re;
  im -= c.im;
  return *this;
}

Complex& Complex::operator *= (const Complex &c)
{
  float r = re * c.re - im * c.im;
  float i = re * c.im + im * c.re;
  re = r;
  im = i;
  return *this;
}

Complex& Complex::operator /= (const Complex &c)
{
  float f = 1.0/(c.re * c.re + c.im * c.im);
  float r = (re * c.re + im * c.im) * f;
  float i = (im * c.re - re * c.im) * f;
  re = r;
  im = i;
  return *this;
}

Complex Complex::c_exp() const
{
  float e = exp(re);
  return Complex(e * cos(im), e * sin(im));
}
