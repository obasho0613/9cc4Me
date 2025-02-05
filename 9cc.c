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

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号の時には、トークンを一つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

// 次のトークンが期待している記号の時には、トークンを一つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
	if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
		error_at(token->str, "'%s'ではありません", op);
	token = token->next;
}

// 次のトークンが数字の場合、トークンを一つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startsWith(char *p, char *q) {
	return memcmp(p,q,strlen(q)) == 0;
}


// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
	char *p = user_input;
	// ダミーのhead要素を作る
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// 空白をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}
		// 記号(複数文字)
		if (startsWith(p, "==") || startsWith(p, "!=" ) ||
		    startsWith(p, "<=") || startsWith(p, ">=" )) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}
		// 記号(単一文字)
		if (strchr("+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}
		// 数字
		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	// 最後に返す
	return head.next;
}

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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs  = lhs;
	node->rhs  = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val  = val;
	return node;
}

// コンパイルエラー回避のためのプロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

// expr       = equality
Node *expr() {
	//printf("%c:%s\n", *(token->str), __func__);
	return equality();
}
// equality   = relational ("==" relational | "!=" relational )*
Node *equality() {
	//printf("%c:%s\n", *(token->str), __func__);
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}
//relational = add ("<" add | "<=" add | ">" add | "<" add)*
Node *relational() {
	//printf("%c:%s\n", *(token->str), __func__);
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}
// add = mul ("+" mul | "-" mul)*
Node *add() {
	//printf("%c:%s\n", *(token->str), __func__);
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}
// mul = primary ("*" primary | "/" primary)*
Node *mul() {
	//printf("%c:%s\n", *(token->str), __func__);
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}
Node *unary() {
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), unary());
	return primary();
}
// primary = "(" expr ")" | num
Node *primary() {
	//printf("%c:%s\n", *(token->str), __func__);
	// 次のトークンが"("なら、"(" expr ")"のはず
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	// そうでなければ数値のはず
	return new_node_num(expect_number());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		//printf("%d:%s\n", node->val, __func__);
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->kind) {
	case ND_ADD:
		printf("  add rax, rdi\n");
		break;
	case ND_SUB:
		printf("  sub rax, rdi\n");
		break;
	case ND_MUL:
		printf("  imul rax, rdi\n");
		break;
	case ND_DIV:
		printf("  cqo\n");
		printf("  idiv rdi\n");
		break;
	case ND_EQ:
		printf("  cmp rax, rdi\n");
		printf("  sete al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_NE:
		printf("  cmp rax, rdi\n");
		printf("  setne al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LT:
		printf("  cmp rax, rdi\n");
		printf("  setl al\n");
		printf("  movzb rax, al\n");
		break;
	case ND_LE:
		printf("  cmp rax, rdi\n");
		printf("  setle al\n");
		printf("  movzb rax, al\n");
		break;
	}

	printf("  push rax\n");
}

int main(int argc, char **argv) {
	if (argc != 2)
		fprintf(stderr, "引数の個数が正しくありません\n");

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// 抽象構文木(AST)をトラバースしてアセンブリを生成する
	gen(node);

	// 結果はスタックの一番上に格納されているので、
	// RAXにpopして、プログラムの終了コードにする。
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}

