/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//----------------
// Hash Functions
//----------------
#define SHIFT 4
static int hash(char *key)
{
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

//--------------------------------------------------------
// Scope Tables (each entries containes its symbol table)
//--------------------------------------------------------
static ScopeList scopeList = NULL;

//--------------------------------------------------------
// Symbol & Scope Table Functions
//--------------------------------------------------------
// Insert New Scope
ScopeRec *insertScope(char *name, ScopeRec *parent, TreeNode *func)
{
	// Error Check: Parameters
	ERROR_CHECK( name != NULL || parent != NULL );

	// Get Scope Name: Given or Derived
	char *scopeName = NULL;
	if (name == NULL)
	{
		size_t length = strlen(parent->name);
		scopeName = (char *)malloc(sizeof(char) * (length + 6));
		snprintf(scopeName, length + 6, "%s.%d", parent->name, parent->numScopes++);
		scopeName[length + 5] = '\0';
	}
	else
	{
		size_t length = strlen(name);
		scopeName = (char *)malloc(sizeof(char) * (length + 1));
		memcpy(scopeName, name, length);
		scopeName[length] = '\0';
	}

	// Get Last Index of Scope
	int redefined = (parent != NULL && parent->state == STATE_REDEFINED) ? TRUE : FALSE;
	ScopeRec *lastScope = scopeList;
	while (lastScope != NULL)
	{
		// Error Check: Already Defined Scope is Exist with Redefinition Flags
		if (strcmp(scopeName, lastScope->name) == 0)
		{
			ERROR_CHECK(lastScope->state == STATE_REDEFINED );
			redefined = TRUE;
		}
		// Iterate Scope List
		if (lastScope->next == NULL) break;
		lastScope = lastScope->next;
	}

	// Add New Scope to Scope HashTable
	ScopeRec *scope = (ScopeRec *)malloc(sizeof(ScopeRec));
	scope->name = scopeName;
	scope->state = redefined == TRUE ? STATE_REDEFINED : STATE_NORMAL;
	scope->func = func;
	for (int i = 0; i < SIZE; ++i) scope->symbolList[i] = NULL;
	scope->numSymbols = 0;
	scope->numScopes = 0;
	scope->parent = parent;
	if (lastScope == NULL) scopeList = scope;
	else
		lastScope->next = scope;
	scope->next = NULL;

	// Return
	return scope;
}


// Insert New Symbol
SymbolRec *insertSymbol(ScopeRec *currentScope, char *name, NodeType type, SymbolKind kind, int lineno, TreeNode *node)
{
	// Error Check: Parameters
	ERROR_CHECK( currentScope != NULL && name != NULL );

	// Find Symbol Table Entry: Just Find in Current Scope Only
	int hashIdx = hash(name);
	SymbolRec *lastSymbol = currentScope->symbolList[hashIdx];
	SemanticErrorState state = STATE_NORMAL;
	while (lastSymbol != NULL)
	{
		// If Duplicated Symbol Exist
		if( strcmp(name, lastSymbol->name) == 0 )
		{
			if (lastSymbol->state == STATE_REDEFINED) state = STATE_REDEFINED;
			else if( lastSymbol->state == STATE_UNDECLARED)
			{
				lastSymbol->type = type;
				lastSymbol->state = node == NULL ? STATE_UNDECLARED : STATE_NORMAL;

				return lastSymbol;
			}
			else ERROR_CHECK( "Normal, Duplicated Symbol Exist");
		}
		// Iterate Symbol List
		if (lastSymbol->next == NULL) break;
		lastSymbol = lastSymbol->next;
	}

	// Add New Symbol to Current Scope
	SymbolRec *symbol = (SymbolRec *)malloc(sizeof(SymbolRec));
	symbol->name = name;
	symbol->state = state;
	symbol->type = type;
	symbol->kind = kind;
	symbol->lineList = (LineList)malloc(sizeof(LineListRec));
	symbol->lineList->lineno = lineno;
	symbol->lineList->next = NULL;
	symbol->memloc = currentScope->numSymbols++;
	if (lastSymbol == NULL) currentScope->symbolList[hashIdx] = symbol;
	else
		lastSymbol->next = symbol;
	symbol->next = NULL;
	symbol->node = node;
	if( node == NULL ) symbol->state = STATE_UNDECLARED;

	// Return
	return symbol;
}

// Add Use to Exist Symbol
SymbolRec *appendSymbol(ScopeRec *currentScope, char *name, int lineno)
{
	// Error Check: Parameters
	ERROR_CHECK( currentScope != NULL && name != NULL );

	// Find Symbol Table Entry: Just Find in Current Scope Only
	int hashIdx = hash(name);
	ScopeRec *scope = currentScope;
	SymbolRec *symbol = NULL;
	while (scope != NULL)
	{
		symbol = scope->symbolList[hashIdx];
		while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;

		// If Find, Break, Else, Goto Parent Scope
		if (symbol == NULL) scope = scope->parent;
		else
			break;
	}

	// Error Check: Undefined symbolList
	ERROR_CHECK( scope != NULL );

	// Add Line In Symbol Table symbol
	LineListRec *line = symbol->lineList;
	while (line->next != NULL) line = line->next;
	line->next = (LineListRec *)malloc(sizeof(LineListRec));
	line->next->lineno = lineno;
	line->next->next = NULL;

	// Return
	return symbol;
}

// Search symbolList with Name
SymbolRec *lookupSymbol(ScopeRec *currentScope, char *name)
{
	// Error Check: Parameters
	ERROR_CHECK( currentScope != NULL && name != NULL );

	// Find Symbol Table Record
	int hashIdx = hash(name);
	ScopeRec *scope = currentScope;
	SymbolRec *symbol = NULL;

	while (scope != NULL)
	{
		symbol = scope->symbolList[hashIdx];
		while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;

		// If Find, Return, Else, Goto Parent Scope
		if (symbol == NULL) scope = scope->parent;
		else
			return symbol;
	}

	// Cannot Find
	return NULL;
}

SymbolRec *lookupSymbolInCurrentScope(ScopeRec *currentScope, char *name)
{
	// Error Check: Parameters
	ERROR_CHECK( currentScope != NULL && name != NULL );

	// Find Symbol Table Record
	int hashIdx = hash(name);
	ScopeRec *scope = currentScope;
	SymbolRec *symbol = scope->symbolList[hashIdx];
	while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;

	return symbol;
}

SymbolRec *lookupSymbolWithKind(ScopeRec *currentScope, char *name, SymbolKind kind)
{
	// Error Check: Parameters
	ERROR_CHECK( currentScope != NULL && name != NULL );

	// Find Symbol Record
	int hashIdx = hash(name);
	ScopeRec *scope = currentScope;
	SymbolRec *symbol = NULL;

	while (scope != NULL)
	{
		symbol = scope->symbolList[hashIdx];
		while ((symbol != NULL) && ((strcmp(name, symbol->name) != 0) || (symbol->kind != kind))) symbol = symbol->next;

		// If Find, Return, Else, Goto Parent Scope
		if (symbol == NULL) scope = scope->parent;
		else
			return symbol;
	}

	return NULL;
}

// Print Symbol & Scope Tables
void printSymbolTable(FILE *listing)
{
	fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
	fprintf(listing, "-------------  -----------  -------------  ------------  --------  ------------\n");
	ScopeRec *scope = scopeList;
	while (scope != NULL)
	{
		for (int i = 0; i < SIZE; ++i)
		{
			SymbolRec *symbol = scope->symbolList[i];
			while (symbol != NULL)
			{
				// Symbol Name, Symbol Kind, Symbol Type, Scope Name, Location
				fprintf(listing,
						"%-13s  %-11s  %-13s  %-12s  %-8d ",
						symbol->name,
						KIND2STR(symbol->kind),
						TYPE2STR(symbol->type),
						scope->name,
						symbol->memloc);
				// Line Numbers
				LineListRec *line = symbol->lineList;
				while (line != NULL)
				{
					fprintf(listing, "%4d ", line->lineno);
					line = line->next;
				}
				fprintf(listing, "\n");

				// Iterate
				symbol = symbol->next;
			}
		}
		// Iterate
		scope = scope->next;
	}
}

void printFunction(FILE *listing)
{
	fprintf(listing, "Function Name   Return Type   Parameter Name  Parameter Type\n");
	fprintf(listing, "-------------  -------------  --------------  --------------\n");
	ScopeRec *scope = scopeList;
	while (scope != NULL)
	{
		for (int i = 0; i < SIZE; ++i)
		{
			SymbolRec *symbol = scope->symbolList[i];
			while (symbol != NULL)
			{
				if (symbol->kind == FunctionSym)
				{
					// Function Name, Return Type
					fprintf(listing, "%-13s  %-13s ", symbol->name, TYPE2STR(symbol->type));
					if (symbol->type == Undetermined) fprintf(listing, " %-14s  %-12s\n", "", TYPE2STR(Undetermined));
					else
					{
						// Parameter Name & Type
						TreeNode *param = symbol->node->child[0];
						if (param->type == Void) fprintf(listing, " %-14s  %-12s\n", "", TYPE2STR(Void));
						else
						{
							fprintf(listing, "\n");
							while (param != NULL)
							{
								// Parameter Name, Parameter Type
								fprintf(listing, "%-13s  %-13s  %-14s  %-12s\n", "-", "-", param->name, TYPE2STR(param->type));
								// Iterate
								param = param->sibling;
							}
						}
					}
				}
				symbol = symbol->next;
			}
		}
		scope = scope->next;
	}
}

void printGlobal(FILE *listing, ScopeRec *globalScope)
{
	fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type\n");
	fprintf(listing, "-------------  -----------  -------------\n");
	for (int i = 0; i < SIZE; ++i)
	{
		SymbolRec *symbol = globalScope->symbolList[i];
		while (symbol != NULL)
		{
			fprintf(listing, "%-13s  %-11s  %-13s\n", symbol->name, KIND2STR(symbol->kind), TYPE2STR(symbol->type));
			symbol = symbol->next;
		}
	}
}

void printScope(FILE *listing, ScopeRec *globalScope)
{
	fprintf(listing, " Scope Name   Nested Level   Symbol Name   Symbol Type\n");
	fprintf(listing, "------------  ------------  -------------  -----------\n");
	ScopeRec *scope = scopeList;
	while (scope != NULL)
	{
		// Skip Global Scope
		if (scope == globalScope)
		{
			scope = scope->next;
			continue;
		}

		int PrintSymbol = FALSE;
		for (int i = 0; i < SIZE; ++i)
		{
			SymbolRec *symbol = scope->symbolList[i];
			while (symbol != NULL)
			{
				int nested_level = 0;
				ScopeRec *currentScope = scope;
				while (scope != globalScope)
				{
					scope = scope->parent;
					nested_level++;
				}
				scope = currentScope;

				// Scope Name, Nested Level, Symbol Name, Symbol Type
				fprintf(listing, "%-12s  %-12d  %-13s  %-11s\n", scope->name, nested_level, symbol->name, TYPE2STR(symbol->type));
				PrintSymbol = TRUE;

				// Iterate
				symbol = symbol->next;
			}
		}
		if (PrintSymbol) fprintf(listing, "\n");

		// Iterate
		scope = scope->next;
	}
}
