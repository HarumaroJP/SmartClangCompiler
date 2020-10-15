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

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM  // 整数
} NodeType;

typedef struct Node Node;
typedef struct Token Token;

struct Token {
    TokenType type; //トークンの型
    Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合、その数値
    char *str;      //トークン文字列
};

struct Node {
    NodeType type; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;       //typeがND_NUMの場合のみ使う
};

// function List
void error_at(const char *loc, char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
bool isreserved(char r);
Token *new_token(TokenType type, Token *cur, char *str);
Token *tokenize(char *p);
Node *new_node(NodeType type, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *primary();
void gen(Node *node);
void *unary();

Token *token;
char *user_input;
char reservedList[6] = {'+', '-', '*', '/', '(', ')'};


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません！\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    Node *node = expr();


    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

void error_at(const char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが記号か。記号だったらトークンを次に進める。
bool consume(char op) {
    if (token->type != TK_RESERVED || token->str[0] != op) //token->type : (*token).type  same meaning.
        return false;
    token = token->next;
    return true;
}

//記号じゃなかったらエラーを吐く。
void expect(char op) {
    if (token->type != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "'%c'ではありません。", op);

    token = token->next;
}

//整数じゃなかったらエラーを吐く。トークンを次に進めて、現在のトークンの数を返す。
int expect_number() {
    if (token->type != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

//入力の終わりかどうか
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

        if (strchr(reservedList, *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *new_node(NodeType type, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->type = ND_NUM;
    node->val = val;
    return node;
};

Node *expr() {
    Node *node = mul();

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    while (true) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node) {
    if (node->type == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->type) {
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

        default:
            break;
    }

    printf("  push rax\n");
}

void *unary() {
    if (consume('+'))
        return primary();
    if (consume('-')) {
//        fprintf(stderr, "%s", (const char *) token->type);
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}


