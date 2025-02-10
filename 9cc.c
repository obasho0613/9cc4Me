#include "9cc.h"

// 現在着目しているトークン
extern Token *token;

// 入力プログラム
extern char *user_input;

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
