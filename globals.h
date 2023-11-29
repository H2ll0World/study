/****************************************************/
/* File: globals.h                                  */
/* Yacc/Bison Version                               */
/* Global types and vars for TINY compiler          */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Yacc/Bison generates internally its own values
 * for the tokens. Other files can access these values
 * by including the tab.h file generated using the
 * Yacc/Bison option -d ("generate header")
 *
 * The YYPARSER flag prevents inclusion of the tab.h
 * into the Yacc/Bison output itself
 */

#ifndef YYPARSER

	/* the name of the following file may change */
	#include "y.tab.h"

	/* ENDFILE is implicitly defined by Yacc/Bison,
	 * and not included in the tab.h file
	 */
	#define ENDFILE 0

#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef TRUE
	#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

/* Yacc/Bison generates its own integer values
 * for tokens
 */
typedef int TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code;	  /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

// Node Kind
typedef enum
{
	NoneStmt = 0xff,
	NoneExpr = 0xff,
	/* Declarations */
	VariableDecl = 0x30,
	FunctionDecl = 0x31,
	Params = 0x32,
	/* Statements */
	CompoundStmt = 0x40,
	IfStmt = 0x41,
	WhileStmt = 0x42,
	ReturnStmt = 0x43,
	/* Expressions */
	AssignExpr = 0x50,
	VarAccessExpr = 0x51,
	BinOpExpr = 0x52,
	ConstExpr = 0x53,
	CallExpr = 0x54,
	/* For Parser Implementations */
	Indentifier = 0xf0,
	TypeSpecifier = 0xf1,
	Opcode = 0xf2
} NodeKind;

// Type Specifier
typedef enum
{
	None = 0xff,
	Undetermined = 0xf0,
	Void = 0x10,
	Integer = 0x11,
	VoidArray = 0x20,
	IntegerArray = 0x21
} NodeType;

// Symbol Kind (Function or Variable)
typedef enum
{
	NoneSym = 0xff,
	FunctionSym = 0x10,
	VariableSym = 0x11
} SymbolKind;

// TreeNode Structure
#define MAXCHILDREN 3
struct ScopeRec;
typedef struct treeNode
{
	// For Tree Structures
	struct treeNode *child[MAXCHILDREN];
	struct treeNode *sibling;
	int lineno;
	// Experssion Type
	NodeKind kind;
	// Attributes
	NodeType type;
	char *name;
	int val;
	int flag;
	TokenType opcode;
	// Scope for Semantic Analysis
	struct ScopeRec *scope;
} TreeNode;

// Useful Macros

#define ERROR_CHECK(condition) \
	if (!(condition)) { fprintf(stderr, "[%s:%d] Error Occurs: \"%s\"\n", __FILE__, __LINE__, #condition); }

#define KIND2STR(kind) ((kind) == FunctionSym ? "Function" : (kind) == VariableSym ? "Variable" : "Unknown")

#define TYPE2STR(type)                      \
	((type) == Integer                      \
		 ? "int"                            \
		 : (type) == IntegerArray ? "int[]" \
								  : (type) == Void ? "void" : (type) == VoidArray ? "void[]" : (type) == Undetermined ? "undetermined" : "unknown")

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
