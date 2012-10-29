%{
#include <stdio.h>
#include <stdlib.h>

#include "auxlib.h"
#include "astree.rep.h"
#include "lyutils.h"
#include "toklist.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(expr)	{}

extern toklist_ref token_list;

void tlist_scanner_include(void);
void tlist_scanner_badtoken(char*);
void tlist_scanner_badchar(char);
int tlist_yylval_token(int);
%}

%option 8bit
%option backup
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose

LETTER 		[a-zA-Z_]
DIGIT		[0-9]
OPERATOR	[\-\+\/\*\=\%\!\(\)\[\]\{\}\.\,\;]
IDENT   	({LETTER}({LETTER}|{DIGIT})*)
BADID       ({DIGIT}({LETTER}|{DIGIT})*)
NUMBER		{DIGIT}({DIGIT})*
STRING      (\"([^\\"\n]|\\[\\’"0nt])*\")
BADSTR      (\"([^\\"\n]|\\[\\’"0nt])*$)
CHAR        (\'[^\n\'\\']|\\[\\\'\"n0t]\')
BADCHAR     (\'(([^\n\'\\][^\n\'\\]+)|(\\[\\\'\"n0t][^\n\'\\]+))\')
COMMENT     (\/\/.*)

%%

"#".*			{ tlist_scanner_include(); 	}
[\040\t]+		{ IGNORE(witespace); 		}
\n 				{ scanner_newline(); 		}

"void"			{ return tlist_yylval_token(TOK_VOID); 	}
"bool"			{ return tlist_yylval_token(TOK_BOOL); 	}
"char"			{ return tlist_yylval_token(TOK_CHAR); 	}
"int"			{ return tlist_yylval_token(TOK_INT);		}
"string"		{ return tlist_yylval_token(TOK_STRING); 	}
"struct"        { return tlist_yylval_token(TOK_STRUCT);    }
"if"            { return tlist_yylval_token(TOK_IF);        }
"else"			{ return tlist_yylval_token(TOK_ELSE);		}
"while"			{ return tlist_yylval_token(TOK_WHILE); 	}
"return"		{ return tlist_yylval_token(TOK_RETURN); 	}
"new"			{ return tlist_yylval_token(TOK_NEW); 		}
"false"			{ return tlist_yylval_token(TOK_FALSE); 	}
"true"			{ return tlist_yylval_token(TOK_TRUE);	 	}
"null"          { return tlist_yylval_token(TOK_NULL);     }
"ord"			{ return tlist_yylval_token(TOK_ORD); 		}
"chr"			{ return tlist_yylval_token(TOK_CHR); 		}

{OPERATOR}		{ return tlist_yylval_token((int)*yytext); }
"=="			{ return tlist_yylval_token(TOK_EQ); 		}
"<="			{ return tlist_yylval_token(TOK_LE); 		}
">="			{ return tlist_yylval_token(TOK_GE); 		}
"<"				{ return tlist_yylval_token(TOK_LT); 		}
">"				{ return tlist_yylval_token(TOK_GT); 		}
"[]"			{ return tlist_yylval_token(TOK_ARRAY);	}

{NUMBER}		{ return tlist_yylval_token(TOK_INTCON);	}
{IDENT}	        { return tlist_yylval_token(TOK_IDENT);	}
{BADID}         { scanner_badtoken(yytext);
                  return tlist_yylval_token(TOK_IDENT);       }

{STRING}        { return tlist_yylval_token(TOK_STRINGCON);}
{CHAR}          { return tlist_yylval_token(TOK_CHARCON);  }

{BADSTR}        { tlist_scanner_badtoken(yytext);
                  return tlist_yylval_token(TOK_STRINGCON);}

{BADCHAR}       { tlist_scanner_badtoken(yytext);
                  return tlist_yylval_token(TOK_CHARCON);  }

.               { tlist_scanner_badchar(*yytext);         }





%%

void tlist_scanner_include() {
	add_toknode(token_list, NULL, yytext);
	scanner_include();
}

int tlist_yylval_token(int symbol) {
	int rcode = yylval_token(symbol);
	char buf[43+strlen(yytext)];
	sprintf(buf, "%4d %6d:%-4d %3d %16s (%s)\n", yylval->filenr, 
		yylval->linenr,	yylval->offset, yylval->symbol, 
		get_yytname(yylval->symbol), yytext);
	add_toknode(token_list, yylval, buf);
	return rcode;
}

void tlist_scanner_badtoken(char * lexeme) {
    set_exitstatus(1);
    scanner_badtoken(lexeme);
}

void tlist_scanner_badchar(char badchar) {
    set_exitstatus(1);
    scanner_badchar(badchar);
}

