#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED,	// 記号
	TK_NUM,		// 整数トークン
	TK_EOF,		// 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
	int len;
};

/*
優先順位のカッコ()を四則演算に追加した文法の生成規則
expr       = equality
equality   = relational ("==" relational | "!=" relational )*
relational = add ("<" add | "<=" add | ">" add | "<" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | "(" expr ")"
*/

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ,  // ==
	ND_NE,  // !=
	ND_LT,  // <
	ND_LE,  // <=
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;
// 抽象構文木のノードの型
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs;     // 左辺(left-hand side)
	Node *rhs;     // 右辺(right-hand side)
	int val;       // kindがND_NUMの場合のみ扱う
};

// コンパイルエラー回避のためのプロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

Token *tokenize();

void gen(Node *node);

bool consume(char *op);
void expect(char *op);
int expect_number();

void error_at(char *loc, char *fmt, ...);