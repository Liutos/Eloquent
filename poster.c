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

enum TOKEN_TYPE {
  NUMBER,
  OPERATOR,
  LPAREN,
  RPAREN,
  ID,
};

enum {
  LEFT,
  RIGHT,
};

struct token_t {
  enum TOKEN_TYPE type;
  union {
    int number;
    char operator;
    char *id;
  } u;
};

struct token_vector_t {
  int length, index;
  token_t **data;
};

token_vector_t *infix2postfix(char *);
void write_tokens(token_vector_t *);
void convert_write(char *);

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

token_vector_t *make_token_vector(int length) {
  token_vector_t *tv = malloc(sizeof(*tv));
  tv->length = length;
  tv->index = 0;
  tv->data = calloc(length, sizeof(token_t *));
  return tv;
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

token_vector_t *infix2postfix(char *str) {
  token_vector_t *stack = make_token_vector(20);
  token_vector_t *queue = make_token_vector(20);
  for (int i = 0; str[i] != '\0';) {
    char c = str[i];
    if (c == ' ' || c == '\t' || c == '\n') {
      i++;
      continue;
    }
    if (c == '(') {
      push(make_lparen(), stack);
      i++;
    } else if (c == ')') {
      while (!is_vector_empty(stack)) {
        token_t *tk = pop(stack);
        if (tk->type == LPAREN)
          break;
        push(tk, queue);
      }
      i++;
    } else if (isdigit(c)) {
      int n = 0;
      do {
        n = n * 10 + c - '0';
        i++;
        c = str[i];
      } while (isdigit(c));
      push(make_number(n), queue);
    } else if (isalpha(c)) {
      string_builder_t *sb = make_str_builder();
      while (isalpha(c)) {
        sb_add_char(sb, c);
        i++;
        c = str[i];
      }
      push(make_id(sb2string(sb)), queue);
    } else if (isoperator(c)) {
      i++;
      token_t *o1 = make_operator(c);
      if (!is_vector_empty(stack)) {
        token_t *o2 = top(stack);
        if (!isparen(o2)) {
          int p1 = op_precedence(o1);
          int p2 = op_precedence(o2);
          while ((is_left_assoc(o1) && p1 <= p2)
              || (is_right_assoc(o1) && p1 < p2)) {
            pop(stack);
            push(o2, queue);
            if (is_vector_empty(stack))
              break;
            o2 = top(stack);
          }
        }
      }
      push(o1, stack);
    } else {
      fprintf(stderr, "Invalid character %c\n", c);
      exit(1);
    }
  }
  while (!is_vector_empty(stack))
    push(pop(stack), queue);
  return queue;
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
    }
  }
}

void convert_write(char *str) {
  printf("%s => ", str);
  write_tokens(infix2postfix(str));
  printf("\n");
}
