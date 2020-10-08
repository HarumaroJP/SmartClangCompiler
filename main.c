#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED, //記号
    TK_NUM, //整数
    TK_EOF, //入力の終わり
} TokenType;

typedef struct Token Token;

struct Token {
    TokenType type;
    struct Token *next;
    int val;
    char *str;
};

void error(char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenType type, Token *cur, char *str);
Token *tokenize(char *p);

Token *token;


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません！\n");
        return 1;
    }

    token = tokenize(argv[1]);


    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");


    printf("  mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf(" ret\n");
    return 0;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if (token->type != TK_RESERVED || token->str[0] != op) //token->type : (*token).type  same meaning.
        return false;
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->type != TK_RESERVED || token->str[0] != op)
        error("'%c'ではありません。", op);

    token = token->next;
}

int expect_number() {
    if (token->type != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->type == TK_EOF;
}

Token *new_token(TokenType type, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->type = type;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}
