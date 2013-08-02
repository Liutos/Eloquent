/*
 * poster.c
 *
 *  Created on: 2013年7月30日
 *      Author: liutos
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "type.h"
#include "utilities.h"

typedef struct token_t token_t;
typedef struct token_vector_t token_vector_t;
typedef struct lexer_t lexer_t;
typedef struct ast_node_t ast_node_t;
typedef struct parser_t parser_t;

enum TOKEN_TYPE {
  NUMBER,
  OPERATOR,
  LPAREN,
  RPAREN,
  ID,
  EOF_TOKEN,
};

enum {
  LEFT,
  RIGHT,
};

enum NODE_TYPE {
  ADD_OP,
  SUB_OP,
  MUL_OP,
  DIV_OP,
  NUM,
};

struct token_t {
  enum TOKEN_TYPE type;
  union {
    int number;
    char operator;
    char *id;
  } u;
};

struct lexer_t {
  char *source;
  int pos;
  char c;
};

struct token_vector_t {
  int length, index;
  token_t **data;
};

struct ast_node_t {
  enum NODE_TYPE type;
  union {
    struct {
      ast_node_t *left, *right;
    } add;
    struct {
      ast_node_t *left, *right;
    } sub;
    struct {
      ast_node_t *left, *right;
    } mul;
    struct {
      ast_node_t *left, *right;
    } div;
    int num_value;
  } u;
};

struct parser_t {
  lexer_t *lexer;
  token_t *token;
};

token_vector_t *infix2postfix(char *);
void write_tokens(token_vector_t *);
void convert_write(char *);
token_vector_t *get_tokens(char *);

int main(int argc, char *argv[]) {
  convert_write("1 + (1)");
  convert_write("(2 * 3)");
  convert_write("9 - (5 + 2)");
  convert_write("(1 + 2) * 3");
  convert_write("(1 + 2) ^ 3!");
  convert_write("var = 1 + 2");
  return 0;
}

token_t *make_number(int number) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = NUMBER;
  tk->u.number = number;
  return tk;
}

token_t *make_operator(char op) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = OPERATOR;
  tk->u.operator = op;
  return tk;
}

token_t *make_lparen(void) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = LPAREN;
  return tk;
}

token_t *make_rparen(void) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = RPAREN;
  return tk;
}

token_t *make_id(char *id) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = ID;
  tk->u.id = id;
  return tk;
}

token_t *make_eof_token(void) {
  token_t *tk = malloc(sizeof(*tk));
  tk->type = EOF_TOKEN;
  return tk;
}

token_vector_t *make_token_vector(int length) {
  token_vector_t *tv = malloc(sizeof(*tv));
  tv->length = length;
  tv->index = 0;
  tv->data = calloc(length, sizeof(token_t *));
  return tv;
}

parser_t *make_parser(lexer_t *lexer) {
  parser_t *parser = malloc(sizeof(*parser));
  parser->lexer = lexer;
  parser->token = NULL;
  return parser;
}

ast_node_t *make_node(enum NODE_TYPE type) {
  ast_node_t *node = malloc(sizeof(*node));
  node->type = type;
  return node;
}

ast_node_t *make_num_node(int value) {
  ast_node_t *node = make_node(NUM);
  node->u.num_value = value;
  return node;
}

ast_node_t *make_bin_op(enum NODE_TYPE type, ast_node_t *left, ast_node_t *right) {
  ast_node_t *node = make_node(type);
  switch (type) {
    case ADD_OP: case SUB_OP: case MUL_OP: case DIV_OP:
      node->u.add.left = left;
      node->u.add.right = right;
      return node;
    default :
      fprintf(stderr, "Unsupported node type %d\n", type);
      exit(1);
  }
}

lexer_t *make_lexer(char *source) {
  lexer_t *lexer = malloc(sizeof(*lexer));
  lexer->source = source;
  lexer->pos = 0;
  lexer->c = source[0];
  return lexer;
}

int is_vector_empty(token_vector_t *vector) {
  return vector->index == 0;
}

void push(token_t *tk, token_vector_t *vector) {
  assert(vector->index <= vector->length - 1);
  vector->data[vector->index] = tk;
  (vector->index)++;
}

token_t *pop(token_vector_t *vector) {
  assert(vector->index > 0);
  (vector->index)--;
  return vector->data[vector->index];
}

token_t *top(token_vector_t *vector) {
  assert(vector->index > 0);
  return vector->data[vector->index - 1];
}

int isparen(token_t *tk) {
  return tk->type == LPAREN || tk->type == RPAREN;
}

int op_precedence(token_t *op) {
  assert(op->type == OPERATOR);
  switch (op->u.operator) {
    case '+': case '-': return 0;
    case '*': case '/': return 1;
    case '^': case '!': return 2;
    case '=': return -1;
    default :
      fprintf(stderr, "Unknown operator %c\n", op->u.operator);
      exit(1);
  }
}

int op_associate(token_t *op) {
  assert(op->type == OPERATOR);
  switch (op->u.operator) {
    case '+': case '-': case '*': case '/':
      return LEFT;
    case '^': case '!': case '=':
      return RIGHT;
    default :
      fprintf(stderr, "Unknown operator %c\n", op->u.operator);
      exit(1);
  }
}

int isoperator(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '!' || c == '=';
}

int is_left_assoc(token_t *op) {
  return op_associate(op) == LEFT;
}

int is_right_assoc(token_t *op) {
  return op_associate(op) == RIGHT;
}

void write_node(ast_node_t *node) {
  switch (node->type) {
    case NUM:
      printf("%d", node->u.num_value);
      break;
    case ADD_OP: case SUB_OP: case MUL_OP: case DIV_OP:
      write_node(node->u.add.left);
      printf("%d", node->type);
      write_node(node->u.add.right);
  }
}

void write_tokens(token_vector_t *vector) {
  for (int i = 0; i < vector->index; i++) {
    token_t *tk = vector->data[i];
    switch (tk->type) {
      case NUMBER:
        printf("%d", tk->u.number);
        break;
      case OPERATOR:
        printf("%c", tk->u.operator);
        break;
      case LPAREN:
        printf("(");
        break;
      case RPAREN:
        printf(")");
        break;
      case ID:
        printf("%s", tk->u.id);
        break;
      case EOF_TOKEN:
        fprintf(stderr, "Hey! It's a BUG!!!\n");
        exit(1);
    }
  }
}

void convert_write(char *str) {
  printf("%s => ", str);
  write_tokens(infix2postfix(str));
  printf("\n");
}

void move(lexer_t *lexer) {
  lexer->pos++;
  lexer->c = lexer->source[lexer->pos];
}

token_t *get_num_token(lexer_t *lexer) {
  int n = 0;
  while (isdigit(lexer->c)) {
    n = n * 10 + lexer->c - '0';
    move(lexer);
  }
  return make_number(n);
}

token_t *get_id_token(lexer_t *lexer) {
  string_builder_t *sb = make_str_builder();
  while (isalpha(lexer->c) || isdigit(lexer->c)) {
    sb_add_char(sb, lexer->c);
    move(lexer);
  }
  return make_id(sb2string(sb));
}

token_t *scan(lexer_t *lexer) {
  char c = lexer->c;
  switch (c) {
    case '\0':
      return make_eof_token();
    case ' ': case '\t': case '\n':
      move(lexer);
      return scan(lexer);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return get_num_token(lexer);
    case '+': case '-': case '*': case '/': case '=':
    case '^': case '!':
      move(lexer);
      return make_operator(c);
    case '(':
      move(lexer);
      return make_lparen();
    case ')':
      move(lexer);
      return make_rparen();
    default :
      return get_id_token(lexer);
  }
}
//
//token_vector_t *infix2postfix(char *str) {
//  token_vector_t *stack = make_token_vector(20);
//  token_vector_t *queue = make_token_vector(20);
//  for (int i = 0; str[i] != '\0';) {
//    char c = str[i];
//    if (c == ' ' || c == '\t' || c == '\n') {
//      i++;
//      continue;
//    }
//    if (c == '(') {
//      push(make_lparen(), stack);
//      i++;
//    } else if (c == ')') {
//      while (!is_vector_empty(stack)) {
//        token_t *tk = pop(stack);
//        if (tk->type == LPAREN)
//          break;
//        push(tk, queue);
//      }
//      i++;
//    } else if (isdigit(c)) {
//      int n = 0;
//      do {
//        n = n * 10 + c - '0';
//        i++;
//        c = str[i];
//      } while (isdigit(c));
//      push(make_number(n), queue);
//    } else if (isalpha(c)) {
//      string_builder_t *sb = make_str_builder();
//      while (isalpha(c)) {
//        sb_add_char(sb, c);
//        i++;
//        c = str[i];
//      }
//      push(make_id(sb2string(sb)), queue);
//    } else if (isoperator(c)) {
//      i++;
//      token_t *o1 = make_operator(c);
//      if (!is_vector_empty(stack)) {
//        token_t *o2 = top(stack);
//        if (!isparen(o2)) {
//          int p1 = op_precedence(o1);
//          int p2 = op_precedence(o2);
//          while ((is_left_assoc(o1) && p1 <= p2)
//              || (is_right_assoc(o1) && p1 < p2)) {
//            pop(stack);
//            push(o2, queue);
//            if (is_vector_empty(stack))
//              break;
//            o2 = top(stack);
//          }
//        }
//      }
//      push(o1, stack);
//    } else {
//      fprintf(stderr, "Invalid character %c\n", c);
//      exit(1);
//    }
//  }
//  while (!is_vector_empty(stack))
//    push(pop(stack), queue);
//  return queue;
//}

int is_to_pop(token_t *op, token_vector_t *stack) {
  token_t *tk = top(stack);
  return (is_left_assoc(op) && op_precedence(op) <= op_precedence(tk)) ||
      (is_right_assoc(op) && op_precedence(op) < op_precedence(tk));
}

token_vector_t *infix2postfix(char *str) {
  lexer_t *lexer = make_lexer(str);
  token_t *la = scan(lexer);
  token_vector_t *stack = make_token_vector(100);
  token_vector_t *queue = make_token_vector(100);
  while (la->type != EOF_TOKEN) {
    switch (la->type) {
      case NUMBER:
        push(la, queue);
        break;
      case OPERATOR: {
        if (!is_vector_empty(stack) && !isparen(top(stack))) {
          while (is_to_pop(la, stack)) {
            token_t *tk = pop(stack);
            push(tk, queue);
            if (is_vector_empty(stack))
              break;
          }
        }
        push(la, stack);
      }
        break;
      case LPAREN:
        push(la, stack);
        break;
      case RPAREN: {
        token_t *tmp = pop(stack);
        while (tmp->type != LPAREN) {
          push(tmp, queue);
          tmp = pop(stack);
        }
      }
        break;
      case ID:
        push(la, queue);
        break;
      default :
        fprintf(stderr, "Unknown token type %d\n", la->type);
        exit(1);
    }
    la = scan(lexer);
  }
  while (!is_vector_empty(stack))
    push(pop(stack), queue);
  return queue;
}

token_vector_t *get_tokens(char *str) {
  token_vector_t *v = make_token_vector(10);
  lexer_t *lexer = make_lexer(str);
  token_t *tk = scan(lexer);
  while (tk->type != EOF_TOKEN) {
    push(tk, v);
    tk = scan(lexer);
  }
  return v;
}

token_t *next_token(parser_t *parser) {
  parser->token = scan(parser->lexer);
  return parser->token;
}

ast_node_t *parse_token(parser_t *parser) {
  token_t *token = next_token(parser);
  switch (token->type) {
    case NUMBER:
      return make_num_node(token->u.number);
    default :
      fprintf(stderr, "Syntax error\n");
      exit(1);
  }
}

ast_node_t *parse_factor(parser_t *parser) {
  ast_node_t *node = parse_token(parser);
  token_t *token = next_token(parser);
  while (token->type == OPERATOR && (token->u.operator == '*' || token->u.operator == '/')) {
    switch (token->u.operator) {
      case '*':
        node = make_bin_op(MUL_OP, node, parse_token(parser));
        break;
      case '/':
        node = make_bin_op(DIV_OP, node, parse_token(parser));
        break;
    }
    token = next_token(parser);
  }
  return node;
}

ast_node_t *parse_term(parser_t *parser) {
  ast_node_t *node = parse_factor(parser);
  printf("In parse_term --- node is ");
  write_node(node);
  token_t *token = next_token(parser);
  while (token->type == OPERATOR && (token->u.operator == '+' || token->u.operator == '-')) {
    switch (token->u.operator) {
      case '+':
        node = make_bin_op(ADD_OP, node, parse_factor(parser));
        break;
      case '-':
        node = make_bin_op(SUB_OP, node, parse_factor(parser));
        break;
    }
    token = next_token(parser);
  }
  return node;
}

ast_node_t *parse_expr(parser_t *parser) {
  printf("Using parse_expr\n");
  return parse_term(parser);
}
