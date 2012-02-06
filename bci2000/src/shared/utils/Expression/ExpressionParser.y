%output="ExpressionParser.cpp"
%defines
%{
////////////////////////////////////////////////////////////////////////
// $Id: ExpressionParser.y 1451 2007-07-24 15:27:44Z mellinger $
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Bison grammar file for a simple expression parser.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <string>
#include <cmath>
#include "Expression.h"

using namespace std;

%}

%pure-parser
%parse-param { ::Expression* pInstance }
%lex-param   { ::Expression* pInstance }
%union
{
  double      value;
  const char* name;
}

%{
namespace ExpressionParser
{
%}

/* Bison declarations.  */
%token <value>  NUMBER
%token <name>   NAME SIGNAL
%left '?' ':'
%left '&' '|'
%left '=' '~' '!' '>' '<'
%left '-' '+'
%left '*' '/'
%left NEG     /* negation--unary minus */
%right '^'    /* exponentiation */

%type <value> exp input

%% /* The grammar follows.  */
input: /* empty */                  { pInstance->mValue = 0; }
       | exp                        { pInstance->mValue = $1; }
;

exp:     NAME                       { $$ = pInstance->State( $1 ); }
       | NUMBER                     { $$ = $1;       }
       | exp '+' exp                { $$ = $1 + $3;  }
       | exp '-' exp                { $$ = $1 - $3;  }
       | exp '*' exp                { $$ = $1 * $3;  }
       | exp '/' exp                { $$ = $1 / $3;  }
       | '-' exp  %prec NEG         { $$ = -$2;      }
       | exp '^' exp                { $$ = ::pow( $1, $3 ); }
       | exp '&' '&' exp            { $$ = $1 && $4; }
       | exp '|' '|' exp            { $$ = $1 || $4; }
       | exp '=' '=' exp            { $$ = $1 == $4; }
       | exp '!' '=' exp            { $$ = $1 != $4; }
       | exp '~' '=' exp            { $$ = $1 != $4; }
       | exp '>' exp                { $$ = $1 > $3;  }
       | exp '<' exp                { $$ = $1 < $3;  }
       | exp '>' '=' exp            { $$ = $1 >= $4; }
       | exp '<' '=' exp            { $$ = $1 <= $4; }
       | '~' exp %prec NEG          { $$ = !$2;      }
       | '!' exp %prec NEG          { $$ = !$2;      }
       | '(' exp ')'                { $$ = $2;       }
       | exp '?' exp ':' exp        { $$ = $1 ? $3 : $5 }
       | SIGNAL '(' exp ',' exp ')' { $$ = pInstance->Signal( $3, $5 ); }
;
%%

  int
  yylex( YYSTYPE* pLval, Expression* pInstance )
  {
    int token = -1;

    pInstance->mInput >> ws;
    int c = pInstance->mInput.peek();
    if( c == EOF )
      token = 0;
    else if( ::isdigit( c ) )
    {
      if( pInstance->mInput >> pLval->value )
        token = NUMBER;
    }
    else if( ::isalnum( c ) )
    {
      static string name;
      name = "";
      while( ::isalnum( c ) )
      {
        name += c;
        pInstance->mInput.ignore();
        c = pInstance->mInput.peek();
      }
      pLval->name = name.c_str();
      if( ::stricmp( pLval->name, "signal" ) == 0 )
        token = SIGNAL;
      else
        token = NAME;
    }
    else
    {
      pInstance->mInput.ignore();
      token = c;
    }
    return token;
  }

  void
  yyerror( Expression* pInstance, const char* pError )
  {
    bcierr__ << "Expression '" << pInstance->mExpression << "': "
             << pError << endl;
  }

} // namespace ExpressionParser

