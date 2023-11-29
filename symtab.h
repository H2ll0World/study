/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

//==================================================================
// Data Structures for Scope & Symbol Table (and Line Number Lists)
//==================================================================

// Struct: Error State
typedef enum SemanticErrorState
{
	STATE_NORMAL = 0x00,
	STATE_REDEFINED = 0xf0,
	STATE_UNDECLARED= 0xf1,
} SemanticErrorState;

// Struct: Line Number Lists
typedef struct LineListRec
{
	// Attributes: Line Number
	int lineno;
	// List Structures
	struct LineListRec *next;
} LineListRec, *LineList;

// Struct: Symbol Table
typedef struct SymbolRec
{
	// Attributes: Name, Error, Type, Kind, Line, Memory Location, Node
	char *name;
	SemanticErrorState state;
	NodeType type;
	SymbolKind kind;
	LineList lineList;
	int memloc;
	TreeNode *node;
	// List Structures
	struct SymbolRec *next;
} SymbolRec, *SymbolList;

// Struct: Scope
typedef struct ScopeRec // Scope Record
{
	// Attributes: Name, Function Node
	char *name;
	SemanticErrorState state;
	TreeNode *func;
	// Symbol Tables in This Scope
	SymbolList symbolList[SIZE];
	int numSymbols;
	// Tree & List Structures
	int numScopes;
	struct ScopeRec *parent;
	struct ScopeRec *next;
} ScopeRec, *ScopeList;

//==================================================================
// Symbol & Scope Table Functions
//==================================================================

// Insert New Scope
ScopeRec *insertScope(char *name, ScopeRec *parent, TreeNode *func);
// Search Scope with Name
// ScopeRec *lookupScope(char *name, ScopeRec *parent);

// Insert New Symbol
SymbolRec *insertSymbol(ScopeRec *currentScope, char *name, NodeType type, SymbolKind kind, int lineno, TreeNode *node);
// Add Use to Exist Symbol
SymbolRec *appendSymbol(ScopeRec *currentScope, char *name, int lineno);
// Search symbolList with Name (and Scope, Kind)
SymbolRec *lookupSymbol(ScopeRec *currentScope, char *name);
SymbolRec *lookupSymbolInCurrentScope(ScopeRec *currentScope, char *name);
SymbolRec *lookupSymbolWithKind(ScopeRec *currentScope, char *name, SymbolKind kind);

// Print Symbol & Scope Tables
void printSymbolTable(FILE *listing);
void printFunction(FILE *listing);
void printGlobal(FILE *listing, ScopeRec *globalScope);
void printScope(FILE *listing, ScopeRec *globalScope);

#endif
