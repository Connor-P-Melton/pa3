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

// Multiply with support for numeric coersion (Perl allows "1" * 1 = 1)
Value Value::operator*(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();  // error constructor
  }

  // Support for multipling numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stoi(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stoi(op.Stemp);
  }

  // convert bc confusion between 0 and boolean for constructor
  return Value(static_cast<double>(thisValue * opValue));
}

// numeric less than
Value Value::operator<(const Value& op) const {
  double thisValue = this->Ntemp;
  double opValue = op.Ntemp;

  // expects two numbers or string numbers
  if ((!this->IsNum() && !isNumericStr(this->Stemp)) ||
      (!op.IsNum() && !isNumericStr(op.Stemp))) {
    return Value();  // error constructor
  }

  // Support for multipling numeric strings
  if (isNumericStr(this->Stemp)) {
    thisValue = stoi(this->Stemp);
  }
  if (isNumericStr(op.Stemp)) {
    opValue = stoi(op.Stemp);
  }

  // convert bc confusion between 0 and boolean for constructor
  return Value(thisValue < opValue);
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
    repetitions = stoi(oper.Stemp);
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
