#include "9cc.h"

extern char *user_input;

bool startsWith(char *p, char *q) {
	return memcmp(p,q,strlen(q)) == 0;
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
