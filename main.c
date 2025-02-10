#include "9cc.h"

// 現在着目しているトークン
extern Token *token;

int main(int argc, char **argv) {
	if (argc != 2)
		fprintf(stderr, "引数の個数が正しくありません\n");

	// トークナイズしてパースする
	token = tokenize(argv[1]);
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
