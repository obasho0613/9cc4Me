#include "9cc.h"

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}