#pragma once


// (x + (y * z))
#define SYNTAX_LPAREN			'('
#define SYNTAX_RPAREN			')'

// let (x : y, z) = b
#define SYNTAX_FUNC_L			'('
#define SYNTAX_FUNC_PCOND		':'
#define SYNTAX_FUNC_PSEP		','
#define SYNTAX_FUNC_R			')'
#define SYNTAX_FUNC_PROLOGUE	'='

// [ x, y, z ]
#define SYNTAX_LIST_L			'['
#define SYNTAX_LIST_SEP			','
#define SYNTAX_LIST_R			']'

// { a = x, b = y, c = z }
#define SYNTAX_MAP_L			'{'
#define SYNTAX_MAP_SET			'='
#define SYNTAX_MAP_SEP			','
#define SYNTAX_MAP_R			'}'

// @(x) = y
// @{ let (x) = a
//    let (y) = b }
#define SYNTAX_LAMBDA			'@'
#define SYNTAX_LAMBDA_L			'{'
#define SYNTAX_LAMBDA_R			'}'

// a $ x : f(x) = g(x)
#define SYNTAX_LCOMP_SEP		'$'
#define SYNTAX_LCOMP_FILTER		':'
#define SYNTAX_LCOMP_MAP		'='

// with (x = a, [z, y] = b)
#define SYNTAX_WITH_L			'('
#define SYNTAX_WITH_ASSIGN		'='
#define SYNTAX_WITH_SEP			','
#define SYNTAX_WITH_R			')'

// `+ x
// &==
#define SYNTAX_MINI_LAMBDA_LEFT	'`'
#define SYNTAX_MINI_LAMBDA_BIN	'&'