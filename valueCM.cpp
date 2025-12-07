// Connor Melton
#include <iostream>
#include <string>
#include <vector>

#include "val.h"
using namespace std;

bool isNumericStr(string str) {
  // Edge case
  if (str.empty()) {
    return false;
  }

  for (char s : str) {
    if (!isdigit(s) && s != '.') {
      return false;
    }
  }
  return true;
}

// Removes trailing zeros
string toString(double num) {
  string str = "";
  ostringstream ostream;
  ostream << num;
  return ostream.str();
}

Value Value::operator+(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();  // error constructor
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  return Value(static_cast<double>(thisValue + opValue));  // + sign
}

// numeric overloaded subtract op from this
Value Value::operator-(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  return Value(static_cast<double>(thisValue - opValue));  // - sign
}

// Multiply with support for numeric coersion (Perl allows "1" * 1 = 1)
Value Value::operator*(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  return Value(static_cast<double>(thisValue * opValue));  // * sign
}

// numeric overloaded divide this by op
Value Value::operator/(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  // cannot divide by zero
  if (opValue == 0) {
    return Value();
  }
  return Value(static_cast<double>(thisValue / opValue));  // / sign
}

// numeric remaider of integer division of this by op
Value Value::operator%(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }
  // a % b = a - int(a/b) * b
  int remainder = thisValue - int(thisValue / opValue) * opValue;
  return Value(static_cast<double>(remainder));
}

// numeric overloaded equality operator of this with op
Value Value::operator==(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numerics, string numbers, or booleans (valid for ==)
  if (((!this->IsNum() && !isNumericStr(this->Stemp)) ||
       (!op.IsNum() && !isNumericStr(op.Stemp))) &&
      (!this->IsBool() || !op.IsBool())) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  // Support for boolean equality comparison
  if (this->IsBool() && op.IsBool()) {
    thisValue = this->Btemp;
    opValue = op.Btemp;
  }

  return Value(thisValue == opValue);  // == sign
}

// numeric overloaded greater than operator of this with op
Value Value::operator>=(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numerics, string numbers, or booleans (valid for ==)
  if (((!this->IsNum() && !isNumericStr(this->Stemp)) ||
       (!op.IsNum() && !isNumericStr(op.Stemp))) &&
      (!this->IsBool() || !op.IsBool())) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  // Support for boolean equality comparison
  if (this->IsBool() && op.IsBool()) {
    thisValue = this->Btemp;
    opValue = op.Btemp;
  }

  return Value(thisValue >= opValue);  // >= sign
}

// numeric less than
Value Value::operator<(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();
  }

  // Support for multipling numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stod(op.Stemp);
  }

  // convert bc confusion between 0 and boolean for constructor
  return Value(thisValue < opValue);
}

// Numeric exponentiation operation this raised to the power of op
Value Value::Expon(const Value& oper) const {
  double thisValue = this->Ntemp;
  double opValue = oper.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!oper.IsNum() && !isNumericStr(oper.Stemp))) {
    return Value();
  }

  // Support for numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stod(this->Stemp);
  }
  if (isNumericStr(oper.Stemp)) {
    opValue = stod(oper.Stemp);
  }

  return Value(pow(thisValue, opValue));  // exponentiation
}

// string concatenation
Value Value::Catenate(const Value& oper) const {
  string thisValue = this->Stemp;
  string operValue = oper.Stemp;

  // expects numbers and/or string
  if ((!this->IsNum() && !this->IsString()) ||
      (!oper.IsNum() && !oper.IsString())) {
    return Value();
  }

  if (this->GetType() == VNUM) {
    thisValue = toString(this->Ntemp);
  }
  if (oper.GetType() == VNUM) {
    operValue = toString(oper.Ntemp);
  }

  return Value(thisValue + operValue);
}

// string repetition
// Repeat the LHS a RHS # of times
Value Value::Repeat(const Value& oper) const {
  // expects lhs/rhs as number/string
  if ((!this->IsNum() && !this->IsString()) ||
      (!oper.IsNum() && !isNumericStr(oper.Stemp))) {
    return Value();
  }

  int repetitions;
  if (oper.GetType() == VSTRING) {
    repetitions = stod(oper.Stemp);
  } else if (oper.GetType() == VNUM) {
    repetitions = oper.Ntemp;
  }

  string currStr = "";
  for (int i = 0; i < repetitions; i++) {
    // Use numeric string
    if (this->GetType() == VSTRING) {
      currStr += this->Stemp;
    } else if (this->GetType() == VNUM) {
      currStr += toString(this->Ntemp);
    }
  }

  return Value(currStr);
}

// string equality (-eq)
Value Value::SEQ(const Value& oper) const {
  string thisValue = this->Stemp;
  string operValue = oper.Stemp;

  // expects numbers and/or string
  if ((!this->IsNum() && !this->IsString()) ||
      (!oper.IsNum() && !oper.IsString())) {
    return Value();
  }

  if (this->GetType() == VNUM) {
    thisValue = this->Ntemp;
  }
  if (oper.GetType() == VNUM) {
    operValue = oper.Ntemp;
  }

  return Value(thisValue == operValue);
}

// string greater than (-gt) operator of this with op
Value Value::SGT(const Value& oper) const {
  string thisValue = this->Stemp;
  string operValue = oper.Stemp;

  // expects numbers and/or string
  if ((!this->IsNum() && !this->IsString()) ||
      (!oper.IsNum() && !oper.IsString())) {
    return Value();
  }

  if (this->GetType() == VNUM) {
    thisValue = this->Ntemp;
  }
  if (oper.GetType() == VNUM) {
    operValue = oper.Ntemp;
  }

  return Value(thisValue > operValue);  // > sign
}

// string less than operator of this with op
Value Value::SLE(const Value& oper) const {
  string thisValue = this->Stemp;
  string operValue = oper.Stemp;

  // expects numbers and/or string
  if ((!this->IsNum() && !this->IsString()) ||
      (!oper.IsNum() && !oper.IsString())) {
    return Value();
  }

  if (this->GetType() == VNUM) {
    thisValue = this->Ntemp;
  }
  if (oper.GetType() == VNUM) {
    operValue = oper.Ntemp;
  }

  return Value(thisValue < operValue);  // < sign
}

// Logic overloaded Anding of this and op
Value Value::operator&&(const Value& op) const {
  double thisValue = this->Btemp;
  double opValue = op.Btemp;

  // expects booleans
  if (!this->IsBool() || !op.IsBool()) {
    return Value();
  }

  return Value(thisValue && opValue);  // && sign
}

// Logic overloaded Oring of this and op
Value Value::operator||(const Value& op) const {
  double thisValue = this->Btemp;
  double opValue = op.Btemp;

  // expects booleans
  if ((!this->IsBool()) || !op.IsBool()) {
    return Value();
  }

  return Value(thisValue || opValue);  // || sign
}

// Logic overloaded Not of this
Value Value::operator!() const {
  double thisValue = this->Btemp;

  // expects booleans
  if ((!this->IsBool())) {
    return Value();
  }

  return Value(!thisValue);  // ! sign
}
