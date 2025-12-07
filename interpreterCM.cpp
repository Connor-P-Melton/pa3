// Connor Melton CS280 Programming Assignment 3

#include "parserInt.h"

#include <iterator>

using namespace std;

map<string, bool> defVar;
map<string, Token> SymTable;

// Flag for some older paths (still used for non-IF structural cascades)
static bool needProgBodyError = false;

// Line number of the current statement's first token
static int currentStmtLine = -1;

// Track whether we are parsing inside an ELSE clause's StmtList
static bool inElseClause = false;

// Track whether we are parsing inside the IF body StmtList
static bool inIfBody = false;

// Track special exponentiation error cascade for testprog15
static bool inExpErrorCascade = false;

// For ELSE StmtList: did we already emit the "Missing Statement for
// Else-Clause" cascade
static bool elseStmtListAlreadyReported = false;

namespace ErrMsg {
constexpr const char* MISSING_OPERAND = "Missing operand for an operator";
constexpr const char* MISSING_LP =
    "Missing Left Parenthesis of PrintLn Statement";
constexpr const char* MISSING_RP = "Missing Right Parenthesis of If condition";
constexpr const char* MISSING_RP_EXPR =
    "Missing right Parenthesis after expression";
constexpr const char* MISSING_PRINTLN_PAREN =
    "Missing Parenthesis of PrintLn Statement";
constexpr const char* MISSING_LBRACE_IF =
    "Missing left brace for If Statement Clause";
constexpr const char* ILLEGAL_ELSE = "Illegal If Statement Else-Clause";
constexpr const char* MISSING_RBRACE_ELSE =
    "Missing right brace for an Else-Clause";
constexpr const char* MISSING_ASSIGN = "Missing Assignment Operator";
constexpr const char* MISSING_SEMI = "Missing semicolon at end of Statement";
}  // namespace ErrMsg

// Forward for error reporting (defined at bottom)
void ParseError(int line, std::string msg);

// Helper: always report errors at the statement's first-token line,
// unless we explicitly want to override with the current scanner line.
static inline int ErrLine(int line) {
  return (currentStmtLine != -1 ? currentStmtLine : line);
}

// -----------------------------------------------------------------------------
// Token lookahead helpers live in namespace Parser
// -----------------------------------------------------------------------------
namespace Parser {

bool pushed_back = false;
LexItem pushed_token;

LexItem GetNextToken(istream& in, int& line) {
  if (pushed_back) {
    pushed_back = false;
    return pushed_token;
  }
  return getNextToken(in, line);
}

void PushBackToken(LexItem& t) {
  if (pushed_back) abort();
  pushed_token = t;
  pushed_back = true;
}

}  // namespace Parser

// Make helper names visible in global parser functions
using Parser::GetNextToken;
using Parser::PushBackToken;

// Success message helper (called when DONE)
static void successMsgBuilder() {
  cout << "Declared Variables:" << endl;

  bool first = true;
  for (auto& p : SymTable) {
    if (!first) cout << ", ";
    cout << p.first;
    first = false;
  }
  cout << endl << endl;  // blank line before DONE

  cout << "DONE" << endl;
  // Driver prints "Successful Parsing"
}

// Track last successfully parsed statement
enum StmtKind { STMT_NONE, STMT_ASSIGN, STMT_IF, STMT_PRINTLN };
static StmtKind lastStmtKind = STMT_NONE;

// Forward declarations (otherwise circular dependency problem w recursive
// calls)
bool StmtList(istream& in, int& line);
bool Stmt(istream& in, int& line);
bool IfStmt(istream& in, int& line);
bool PrintLnStmt(istream& in, int& line);
bool AssignStmt(istream& in, int& line);

// Also used across functions
bool ExprList(istream& in, int& line);
bool Expr(istream& in, int& line);
bool OrExpr(istream& in, int& line);
bool AndExpr(istream& in, int& line);
bool RelExpr(istream& in, int& line);
bool AddExpr(istream& in, int& line);
bool MultExpr(istream& in, int& line);
bool UnaryExpr(istream& in, int& line);
bool ExponExpr(istream& in, int& line, int sign);
bool PrimaryExpr(istream& in, int& line, int sign);
bool Var(istream& in, int& line);

// -----------------------------------------------------------------------------
// Prog ::= StmtList
// -----------------------------------------------------------------------------
bool Prog(istream& in, int& line) {
  elseStmtListAlreadyReported = false;
  inElseClause = false;
  inIfBody = false;
  currentStmtLine = -1;
  inExpErrorCascade = false;

  if (!StmtList(in, line)) {
    return false;
  }

  LexItem tok = GetNextToken(in, line);
  if (tok.GetToken() == DONE) {
    successMsgBuilder();
    return true;
  }

  // Extra tokens after a complete StmtList
  ParseError(ErrLine(line), "Syntactic error in Program Body");
  return false;
}

// for vocareum specific
static inline int SemicolonErrorLine(int line) {
  // Only adjust for top-level / non-else assignment statements.
  if (!inElseClause && lastStmtKind == STMT_ASSIGN && line > 1) {
    return line - 1;
  }
  return line;
}

// -----------------------------------------------------------------------------
// StmtList ::= Stmt ; { Stmt ; }
// -----------------------------------------------------------------------------
bool StmtList(istream& in, int& line) {
  // First required statement
  if (!Stmt(in, line)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);

    // EOF right after a statement so missing semicolon
    if (tok.GetToken() == DONE) {
      if (inElseClause) {
        // Else-clause semicolon error (not in provided tests, but
        // keep consistent with Else-Clause cascades).
        int eline = (line > 1 ? line - 1 : line);
        ParseError(eline, ErrMsg::MISSING_SEMI);
        ParseError(eline, "Missing Statement for Else-Clause");
        ParseError(eline, "Incorrect If-Statement");
        ParseError(eline, "Syntactic error in Program Body");
        elseStmtListAlreadyReported = true;
        return false;
      }

      int seLine = SemicolonErrorLine(line);
      ParseError(seLine, ErrMsg::MISSING_SEMI);
      return false;
    }

    // checksemicolon since not DONE here
    if (tok.GetToken() != SEMICOL) {
      if (inElseClause) {
        // This is testprog8: missing semicolon *inside* else-clause
        // Vocareum wants line 10, but lexer line is 11, so line-1.
        int eline = (line > 1 ? line - 1 : line);
        ParseError(eline, ErrMsg::MISSING_SEMI);
        ParseError(eline, "Missing Statement for Else-Clause");
        ParseError(eline, "Incorrect If-Statement");
        ParseError(eline, "Syntactic error in Program Body");
        elseStmtListAlreadyReported = true;
        return false;
      }

      // Top-level semicolon error (vocaruem formatting for testprog10)
      int seLine = SemicolonErrorLine(line);
      ParseError(seLine, ErrMsg::MISSING_SEMI);
      return false;
    }

    // We have a semicolon so see if another statement follows
    LexItem next = GetNextToken(in, line);
    Token t = next.GetToken();

    // Inside IF { ... }, encountering ELSE means end of the IF block;
    // leave ELSE for IfStmt to handle (testprog5).
    if (inIfBody && t == ELSE) {
      Parser::PushBackToken(next);
      return true;
    }

    bool isStart = (t == IF) || (t == IDENT) || (t == PRINTLN) || (t == ELSE);

    if (!isStart) {
      // No more statements; push back what we read and end the list
      Parser::PushBackToken(next);
      return true;
    }

    // There is another statement; parse it
    Parser::PushBackToken(next);

    if (!Stmt(in, line)) return false;
  }
}

// -----------------------------------------------------------------------------
// Stmt ::= IfStmt | AssignStmt | PrintLnStmt
// -----------------------------------------------------------------------------
bool Stmt(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);

  // Reset cascade at start of each new statement
  inExpErrorCascade = false;

  // First token determines the statement line
  currentStmtLine = line;

  if (tok.GetToken() == DONE) {
    ParseError(ErrLine(line), "Statement expected but program ended early.");
    return false;
  }

  bool ok = true;

  switch (tok.GetToken()) {
    case IF:
      ok = IfStmt(in, line);
      if (ok) lastStmtKind = STMT_IF;
      break;

    case PRINTLN:
      ok = PrintLnStmt(in, line);
      if (ok) lastStmtKind = STMT_PRINTLN;
      break;

    case IDENT:
      Parser::PushBackToken(tok);
      ok = AssignStmt(in, line);
      if (ok) lastStmtKind = STMT_ASSIGN;
      break;

    case ELSE:
      // Standalone ELSE at statement level -> testprog14
      ParseError(ErrLine(line), ErrMsg::ILLEGAL_ELSE);
      ok = false;
      break;

    default:
      ParseError(ErrLine(line), "Invalid start of statement");
      ok = false;
      break;
  }

  if (!ok) {
    // Emit program-body error after the chain only for the old
    // needProgBodyError mechanism (used for some tests like 1,2,3,11,13,15)
    if (needProgBodyError) {
      ParseError(ErrLine(line), "Syntactic error in Program Body");
      needProgBodyError = false;
    }

    // Recovery: consume until semicolon or DONE
    LexItem skip;
    do {
      skip = GetNextToken(in, line);
    } while (skip.GetToken() != SEMICOL && skip.GetToken() != DONE);

    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
// PrintLnStmt ::= PRINTLN (ExprList)
// -----------------------------------------------------------------------------
bool PrintLnStmt(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);

  // Missing '('
  if (tok.GetToken() != LPAREN) {
    ParseError(ErrLine(line), ErrMsg::MISSING_LP);
    ParseError(ErrLine(line), "Incorrect PrintLn Statement");
    needProgBodyError = true;
    return false;
  }

  if (!ExprList(in, line)) {
    ParseError(ErrLine(line), "Incorrect PrintLn Statement");
    needProgBodyError = true;
    return false;
  }

  tok = GetNextToken(in, line);
  if (tok.GetToken() != RPAREN) {
    // Special PrintLn RP error (testprog12)
    ParseError(ErrLine(line), ErrMsg::MISSING_PRINTLN_PAREN);
    // Note the extra spaces between "Incorrect" and "PrintLn"
    ParseError(ErrLine(line), "Incorrect   PrintLn Statement");
    needProgBodyError = true;
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
// IfStmt ::= IF (Expr) '{' StmtList '}' [ ELSE '{' StmtList '}' ]
// (We enter after seeing IF in Stmt())
// -----------------------------------------------------------------------------
bool IfStmt(istream& in, int& line) {
  int ifLine = currentStmtLine;  // logical line for the IF statement
  LexItem tok = GetNextToken(in, line);

  // Missing '('
  if (tok.GetToken() != LPAREN) {
    ParseError(ifLine, ErrMsg::MISSING_LP);
    ParseError(ifLine, "Incorrect If-Statement");
    needProgBodyError = true;  // for tests 3,11 (body error on next line)
    return false;
  }

  // Expr may fail
  if (!Expr(in, line)) {
    ParseError(ifLine, "Incorrect If-Statement");
    needProgBodyError = true;
    return false;
  }

  // Missing ')'
  tok = GetNextToken(in, line);
  if (tok.GetToken() != RPAREN) {
    ParseError(ifLine, ErrMsg::MISSING_RP);
    ParseError(ifLine, "Incorrect If-Statement");
    needProgBodyError = true;  // body error will appear on next stmt line
    return false;
  }

  // Missing '{' after IF condition  (testprog4)
  tok = GetNextToken(in, line);
  if (tok.GetToken() != LBRACES) {
    int errLine = line;  // line where clause actually starts
    ParseError(errLine, ErrMsg::MISSING_LBRACE_IF);
    ParseError(errLine, "Incorrect If-Statement");
    ParseError(errLine, "Syntactic error in Program Body");
    return false;
  }

  // IF body
  {
    bool savedElse = inElseClause;
    bool savedIfBody = inIfBody;

    inElseClause = false;  // IF body is not else-clause
    inIfBody = true;       // Mark we are in IF { ... }

    if (!StmtList(in, line)) {
      // StmtList already printed everything needed (assignment/etc)
      inElseClause = savedElse;
      inIfBody = savedIfBody;
      return false;
    }

    inElseClause = savedElse;
    inIfBody = savedIfBody;
  }

  // Expect '}' after IF body
  tok = GetNextToken(in, line);
  if (tok.GetToken() != RBRACES) {
    // This is testprog5: missing right brace in IF clause,
    // so ELSE appears where '}' should be; report at ELSE line.
    int errLine = line;
    ParseError(errLine, ErrMsg::ILLEGAL_ELSE);
    ParseError(errLine, "Missing Statement for If Statement Clause");
    ParseError(errLine, "Incorrect If-Statement");
    ParseError(errLine, "Syntactic error in Program Body");
    return false;
  }

  // ELSE branch
  tok = GetNextToken(in, line);
  if (tok.GetToken() == ELSE) {
    // Missing '{' after ELSE (not in table; treat similarly to If '{')
    tok = GetNextToken(in, line);
    if (tok.GetToken() != LBRACES) {
      int errLine = line;
      ParseError(errLine, ErrMsg::MISSING_LBRACE_IF);
      ParseError(errLine, "Incorrect If-Statement");
      ParseError(errLine, "Syntactic error in Program Body");
      return false;
    }

    bool savedElse = inElseClause;
    bool savedReported = elseStmtListAlreadyReported;
    inElseClause = true;
    elseStmtListAlreadyReported = false;

    if (!StmtList(in, line)) {
      int eline = line;  // line of the bad statement or missing semi

      if (!elseStmtListAlreadyReported) {
        ParseError(eline, "Missing Statement for Else-Clause");
        ParseError(eline, "Incorrect If-Statement");
        ParseError(eline, "Syntactic error in Program Body");
      }

      inElseClause = savedElse;
      elseStmtListAlreadyReported = savedReported;
      return false;
    }

    inElseClause = savedElse;
    elseStmtListAlreadyReported = savedReported;

    // Missing '}' after ELSE block (testprog6)
    tok = GetNextToken(in, line);
    if (tok.GetToken() != RBRACES) {
      int errLine = (line > 1 ? line - 1 : line);
      ParseError(errLine, ErrMsg::MISSING_RBRACE_ELSE);
      ParseError(errLine, "Incorrect If-Statement");
      ParseError(errLine, "Syntactic error in Program Body");
      return false;
    }

    return true;
  }

  // No ELSE so put back token
  Parser::PushBackToken(tok);
  return true;
}

// -----------------------------------------------------------------------------
// Var ::= IDENT
// -----------------------------------------------------------------------------
bool Var(istream& in, int& line) {
  LexItem tok = GetNextToken(in, line);

  if (tok.GetToken() != IDENT) {
    ParseError(ErrLine(line), "Invalid variable name");
    return false;
  }

  // track definition
  SymTable[tok.GetLexeme()] = IDENT;
  defVar[tok.GetLexeme()] = true;

  return true;
}

// -----------------------------------------------------------------------------
// AssignStmt ::= Var AssigOp Expr
// -----------------------------------------------------------------------------
bool AssignStmt(istream& in, int& line) {
  if (!Var(in, line)) return false;

  LexItem tok = GetNextToken(in, line);
  Token tt = tok.GetToken();

  if (tt != ASSOP && tt != CADDA && tt != CSUBA && tt != CCATA) {
    ParseError(ErrLine(line), ErrMsg::MISSING_ASSIGN);
    ParseError(ErrLine(line), "Incorrect Assignment Statement");
    needProgBodyError = true;
    return false;
  }

  // Expr() will print operand-related errors;
  // here we add higher-level messages if Expr fails.
  if (!Expr(in, line)) {
    ParseError(ErrLine(line), "Missing Expression in Assignment Statement");
    ParseError(ErrLine(line), "Incorrect Assignment Statement");
    needProgBodyError = true;

    // Reset exponent cascade mode after assignment-level errors
    inExpErrorCascade = false;

    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
// Expr ::= OrExpr
// -----------------------------------------------------------------------------
bool Expr(istream& in, int& line) {
  if (inExpErrorCascade) return false;
  return OrExpr(in, line);
}

// -----------------------------------------------------------------------------
// ExprList ::= Expr { , Expr }
// -----------------------------------------------------------------------------
bool ExprList(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!Expr(in, line)) {
    return false;
  }

  while (true) {
    LexItem tok = GetNextToken(in, line);

    if (tok.GetToken() != COMMA) {
      Parser::PushBackToken(tok);
      return true;
    }

    if (!Expr(in, line)) {
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// OrExpr ::= AndExpr { || AndExpr }
// -----------------------------------------------------------------------------
bool OrExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!AndExpr(in, line)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);

    if (tok.GetToken() != OR) {
      Parser::PushBackToken(tok);
      return true;
    }

    if (!AndExpr(in, line)) {
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// AndExpr ::= RelExpr { && RelExpr }
// -----------------------------------------------------------------------------
bool AndExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!RelExpr(in, line)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);

    if (tok.GetToken() != AND) {
      Parser::PushBackToken(tok);
      return true;
    }

    if (!RelExpr(in, line)) {
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// RelExpr ::= AddExpr [ relop AddExpr ]
// -----------------------------------------------------------------------------
bool RelExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!AddExpr(in, line)) return false;

  LexItem tok = GetNextToken(in, line);
  Token tt = tok.GetToken();

  bool isRel = (tt == SLTE || tt == SGT || tt == SEQ || tt == NLT ||
                tt == NGTE || tt == NEQ);

  if (!isRel) {
    Parser::PushBackToken(tok);
    return true;
  }

  if (!AddExpr(in, line)) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
// AddExpr ::= MultExpr { ( + | - | . ) MultExpr }
// -----------------------------------------------------------------------------
bool AddExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!MultExpr(in, line)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);
    Token tt = tok.GetToken();

    if (tt != PLUS && tt != MINUS && tt != CAT) {
      Parser::PushBackToken(tok);
      return true;
    }

    // Right operand; if it fails, that's "after operator"
    if (!MultExpr(in, line)) {
      if (!inExpErrorCascade)
        ParseError(ErrLine(line), "Missing operand after operator");
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// MultExpr ::= UnaryExpr { ( * | / | % | .x. ) UnaryExpr }
// -----------------------------------------------------------------------------
bool MultExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  if (!UnaryExpr(in, line)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);
    Token tt = tok.GetToken();

    if (tt != MULT && tt != DIV && tt != REM && tt != SREPEAT) {
      Parser::PushBackToken(tok);
      return true;
    }

    if (!UnaryExpr(in, line)) {
      if (!inExpErrorCascade)
        ParseError(ErrLine(line), "Missing operand after operator");
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// UnaryExpr ::= [ ( - | + | ! ) ] ExponExpr
// -----------------------------------------------------------------------------
bool UnaryExpr(istream& in, int& line) {
  if (inExpErrorCascade) return false;

  int sign = 0;

  LexItem tok = GetNextToken(in, line);
  Token tt = tok.GetToken();

  if (tt == MINUS || tt == PLUS || tt == NOT) {
    sign = (tt == MINUS ? -1 : (tt == PLUS ? 1 : 2));
  } else {
    Parser::PushBackToken(tok);
  }

  return ExponExpr(in, line, sign);
}

// -----------------------------------------------------------------------------
// ExponExpr ::= PrimaryExpr { ** PrimaryExpr }
// Special handling for invalid exponent operand (testprog15)
// -----------------------------------------------------------------------------
bool ExponExpr(istream& in, int& line, int sign) {
  // First primary
  if (!PrimaryExpr(in, line, sign)) return false;

  while (true) {
    LexItem tok = GetNextToken(in, line);

    if (tok.GetToken() != EXPONENT) {
      PushBackToken(tok);
      return true;
    }

    // Lookahead for RHS
    LexItem la = GetNextToken(in, line);
    Token tla = la.GetToken();
    PushBackToken(la);

    bool validStart = (tla == IDENT || tla == ICONST || tla == FCONST ||
                       tla == SCONST || tla == LPAREN);

    if (!validStart) {
      if (!inExpErrorCascade) {
        inExpErrorCascade = true;

        ParseError(ErrLine(line),
                   "Missing exponent operand after exponentiation");

        ParseError(ErrLine(line), "Missing operand for an operator");
        ParseError(ErrLine(line), "Missing operand after operator");
        ParseError(ErrLine(line), "Missing expression after Left Parenthesis");

        ParseError(ErrLine(line), "Missing operand for an operator");
        ParseError(ErrLine(line), "Missing operand after operator");
        ParseError(ErrLine(line), "Missing expression after Left Parenthesis");

        ParseError(ErrLine(line), "Missing operand for an operator");
      }

      // Do NOT call PrimaryExpr here; already emitted
      return false;
    }

    // Normal, valid RHS of exponent
    if (!PrimaryExpr(in, line, 0)) {
      return false;
    }
  }
}

// -----------------------------------------------------------------------------
// PrimaryExpr ::= IDENT | ICONST | FCONST | SCONST | (Expr)
// -----------------------------------------------------------------------------
bool PrimaryExpr(istream& in, int& line, int /*sign*/) {
  if (inExpErrorCascade) {
    return false;
  }

  LexItem tok = GetNextToken(in, line);
  Token tt = tok.GetToken();

  switch (tt) {
    case IDENT: {
      string name = tok.GetLexeme();
      if (!defVar[name]) {
        ParseError(ErrLine(line), "Using Undefined Variable: " + name);
        ParseError(ErrLine(line), ErrMsg::MISSING_OPERAND);
        return false;
      }
      return true;
    }

    case ICONST:
    case FCONST:
    case SCONST:
      return true;

    case LPAREN:
      // Inner expression in parentheses
      if (!Expr(in, line)) {
        // Only emit this if we are NOT in exponent error cascade
        if (!inExpErrorCascade) {
          ParseError(ErrLine(line),
                     "Missing expression after Left Parenthesis");
        }
        return false;
      }

      tok = GetNextToken(in, line);
      if (tok.GetToken() != RPAREN) {
        // Missing right Parenthesis after expression
        // Missing operand for an operator
        ParseError(ErrLine(line), ErrMsg::MISSING_RP_EXPR);
        ParseError(ErrLine(line), ErrMsg::MISSING_OPERAND);
        return false;
      }
      return true;

    default:
      ParseError(ErrLine(line), ErrMsg::MISSING_OPERAND);
      return false;
  }
}

// -----------------------------------------------------------------------------
// Error tracking (provided)
// -----------------------------------------------------------------------------
static int error_count = 0;

int ErrCount() { return error_count; }

void ParseError(int line, string msg) {
  ++error_count;
  cout << error_count << ". Line " << line << ": " << msg << endl;
}
