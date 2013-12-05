/////////////////////////////////////////////////////////////////////////////
//! \file simlib3D.h  3D vector block extension
//
// Copyright (c) 1997-2004 Petr Peringer 
//
// This library is licensed under GNU Library GPL. See the file COPYING.
//

// 
//  the SIMLIB/3D extension
//
// Warning: beta version -- this is the prototype
//

#ifndef __SIMLIB3D_H
#define __SIMLIB3D_H

#ifndef __SIMLIB__
#   error "simlib3D.h: 19: you must include simlib.h first"
#endif
#if __SIMLIB__ < 0x0212 
#   error "simlib3D.h: 22: requires SIMLIB version 2.12 and higher"
#endif

namespace simlib3 {

////////////////////////////////////////////////////////////////////////////
// Value3D --- 3D vector
//
class Value3D {
  double _x, _y, _z;
 public:
  Value3D(double x, double y, double z) : _x(x), _y(y), _z(z) {}
  double x() const { return _x; }
  double y() const { return _y; }
  double z() const { return _z; }

  // using default copy constructor, destructor, and operator =

  Value3D operator + (Value3D b) { return Value3D(_x+b._x, _y+b._y, _z+b._z); }
  Value3D operator - (Value3D b) { return Value3D(_x-b._x, _y-b._y, _z-b._z); }
  void Print() { ::Print(" %g %g %g ", _x, _y, _z); }

  // utilities:
  friend double abs(const Value3D &a);
  friend Value3D operator +(const Value3D& a, const Value3D &b);
  friend Value3D operator -(const Value3D& a, const Value3D &b);
  friend Value3D operator -(const Value3D& a);
  friend Value3D operator *(const Value3D& a, const Value3D &b);
  friend Value3D operator *(const Value3D& a, const double b);
  friend Value3D operator *(const double a, const Value3D& b);
  friend double scalar_product(const Value3D& a, const Value3D &b);
  friend Value3D operator /(const Value3D& a, const double b);

};


////////////////////////////////////////////////////////////////////////////
// aContiBlock3D --- abstract 3D block with single output
//
class aContiBlock3D : public aBlock {  // abstract continuous block
  aContiBlock3D(const aContiBlock3D&); // disable copy ctr
  void operator= (aContiBlock3D&);     // disable assignment
 protected:
  bool isEvaluated;                  // flag for loop checking ###---
 private:
  virtual void Eval() {};            // evaluate without loop detection
 public:
  aContiBlock3D(); 
  ~aContiBlock3D();
  virtual void _Eval();              // evaluate with loop detection
  virtual Value3D Value() = 0;       // output value
  void Print();                      // print value
};

////////////////////////////////////////////////////////////////////////////
// Constant --- vector value that can't be changed
//
class Constant3D : public aContiBlock3D {
  const Value3D value;
 public:
  explicit Constant3D(Value3D x) : value(x) {}
  explicit Constant3D(double x, double y, double z) : value(x,y,z) {}
  virtual Value3D Value () { return value; }      // without Eval
  // virtual Value3D operator () () { return value; }
};

////////////////////////////////////////////////////////////////////////////
// Variable --- vector variable (can be changed)
//
class Variable3D : public aContiBlock3D {
 protected:
  Value3D value;
 public:
  explicit Variable3D(Value3D x=Value3D(0,0,0)) : value(x) {}
  Variable3D &operator= (Value3D x) { value = x; return *this; }
  virtual Value3D Value ()          { return value; }
};

////////////////////////////////////////////////////////////////////////////
// Parameter --- special variable (can't be changed at simulation time)
//
class Parameter3D : public Variable3D {
 public:
  explicit Parameter3D(Value3D x=Value3D(0,0,0)) : Variable3D(x) {}
  Parameter3D &operator= (const Value3D &x);
};

////////////////////////////////////////////////////////////////////////////
// class Input3D --- continuous block connection (transparent reference)
//
class Input3D {                   // small objects, without virtual methods
  aContiBlock3D *bp;
 public:
  Input3D(const Input3D &inp): bp(inp.bp) {}      // copy reference
  Input3D(aContiBlock3D &cb): bp(&cb) {}          // reference to 3D block
  Input3D(aContiBlock3D *cb): bp(cb)  {}          // pointer to 3D block
  Input3D&operator=(const Input3D &in) { bp = in.bp; return *this; }
  Input3D Set(Input3D i)     { Input3D p=bp; bp=i.bp; return p; }
  Value3D Value()            { return bp->Value(); }   // get value
  bool operator ==(void *p)       { return bp==p; }         // for tests only!
};

////////////////////////////////////////////////////////////////////////////
// aContiBlock1 --- continuous blocks vith single input and alg. loop check
//
class aContiBlock3D1 : public aContiBlock3D {
  Input3D input;
 public:
  explicit aContiBlock3D1(Input3D i);
  Value3D InputValue() { return input.Value(); }
  Input3D SetInput(Input3D i) { return input.Set(i); }
};

////////////////////////////////////////////////////////////////////////////
// Expression --- reference to vector block-expression
//
struct Expression3D : public aContiBlock3D1 {
  explicit Expression3D(Input3D i) : aContiBlock3D1(i) {}
  Value3D Value() { return InputValue(); }
};

////////////////////////////////////////////////////////////////////////////
// aContiBlock2 --- continuous blocks vith two inputs and alg. loop check
//
class aContiBlock3D2 : public aContiBlock3D {
  Input3D input1;
  Input3D input2;
 public:
  aContiBlock3D2(Input3D i1, Input3D i2);
  Value3D Input1Value() { return input1.Value(); }
  Value3D Input2Value() { return input2.Value(); }
};

////////////////////////////////////////////////////////////////////////////
// aContiBlock3 --- continuous blocks vith three inputs and alg. loop check
//
class aContiBlock3D3 : public aContiBlock3D2 {
  Input3D input3;
 public:
  aContiBlock3D3(Input3D i1, Input3D i2, Input3D i3);
  Value3D Input3Value() { return input3.Value(); }
};


////////////////////////////////////////////////////////////////////////////
// Adaptor3D --- 3 scalar inputs, one 3D vector output
//
class Adaptor3D : public aContiBlock3D {
  Input x, y, z;
 public:
  Adaptor3D(Input _x, Input _y, Input _z) : x(_x), y(_y), z(_z) {}
  virtual Value3D Value() {
    return Value3D( x.Value(), y.Value(), z.Value() );
  }
};

////////////////////////////////////////////////////////////////////////////
// Integrator3D --- vector integrator
//
class Integrator3D : public aContiBlock3D {
  Integrator _x,_y,_z;          // 1D standard integrators

  // special_input transforms 3D values into 3 scalar values
  class special_input : public aContiBlock { 
    Value3D a; 			// temporary value
    Input3D in;                 // 3D input
    int count;                  // # of evaluations
   public:
    special_input(Input3D i) : a(0,0,0), in(i), count(0) {} 
    double Value();             // expects 3 subsequent evaluations
    friend class Integrator3D;
  } in; 

 public:
  Integrator3D(Input3D i, Value3D initial_value);
  Integrator3D(Input3D i);
  Integrator3D(Integrator3D &i, Value3D initial_value=Value3D(0,0,0));
  Integrator3D(); 	// for arrays: implicit input value (0,0,0) 

  Input3D SetInput(Input3D i) { return in.in.Set(i); }

  // ### patch from: xbatrl00@stud.fee.vutbr.cz
  void Init(const Value3D &v) {_x.Init(v.x()); _y.Init(v.y()); _z.Init(v.z());}

  Integrator3D &operator = (const Value3D &a);
  Integrator3D &operator = (Input3D i);

  virtual Value3D Value();      // 3D output
};


////////////////////////////////////////////////////////////////////////////
// Continuous block arithmetic operators 
//

// binary operators:
Input3D operator + (Input3D a, Input3D b); 	// add vectors
Input3D operator - (Input3D a, Input3D b); 	// subtract vectors
Input3D operator * (Input3D a, Input3D b); 	// vector multiplication
Input3D operator * (Input3D a, Input b);   	// vector * scalar
Input3D operator * (Input a, Input3D b);   	// scalar * vector
Input3D operator / (Input3D a, Input b);   	// vector / scalar

// unary operators:
Input3D operator - (Input3D a);                 // unary -

// functions:
Input   Abs(Input3D x);               	        // absolute value of vector x
Input3D UnitVector(Input3D x);      	        // unit vector (abs(output)==1)
Input   ScalarProduct(Input3D x, Input3D y);    // dot product (x.y)

Input Xpart(Input3D a); // get x part of vector block
Input Ypart(Input3D a); // get y part of vector block
Input Zpart(Input3D a); // get z part of vector block

////////////////////////////////////////////////////////////////////////////
//  functions
//

inline void Print(Value3D a) {
  a.Print();
}

}

#endif // __SIMLIB3D_H
