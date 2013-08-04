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

typedef struct ast_node_t ast_node_t;
typedef struct lexer_t lexer_t;
typedef struct parser_t parser_t;
typedef struct token_t token_t;

enum TOKEN_TYPE {
  _TYPE_START_=255,
  ID,
  NUM,
};

enum NODE_TYPE {
  ARGS_NODE,
  ASSIGN_NODE,
  CALL_NODE,
  ID_NODE,
  NUM_NODE,
  OFFSET_NODE,
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

struct ast_node_t {
  enum NODE_TYPE type;
  union {
    struct {
      ast_node_t *left, *right;
    } arith;
    int num_value;
    char *id;
    struct {
      ast_node_t *lv, *rv;
    } assign;
    struct {
      ast_node_t *fn, *args;
    } call;
    struct {
      ast_node_t *arg, *rest;
    } args;
    struct {
      ast_node_t *array, *index;
    } offset;
  } u;
};

struct parser_t {
  lexer_t *lexer;
  token_t *look;
};

void convert_write(char *);
ast_node_t *parse_assign(parser_t *);

/* Token Constructors */
token_t *make_eof_token(void) {
  token_t *tk = malloc(sizeof(*tk));
  tk->type = EOF;
  return tk;
}

token_t *make_id(char *id) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = ID;
  tk->u.id = id;
  return tk;
}

token_t *make_number(int number) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = NUM;
  tk->u.number = number;
  return tk;
}

token_t *make_operator(char op) {
  token_t *tk = malloc(sizeof(struct token_t));
  tk->type = op;
  return tk;
}

lexer_t *make_lexer(char *source) {
  lexer_t *lexer = malloc(sizeof(*lexer));
  lexer->source = source;
  lexer->pos = 0;
  lexer->c = source[0];
  return lexer;
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
    case '^': case '!': case '[': case ']': case '(':
    case ')': case ',':
      move(lexer);
      return make_operator(c);
    default :
      return get_id_token(lexer);
  }
}

/* Node Constructors */
void parser_move(parser_t *parser) {
  parser->look = scan(parser->lexer);
}

parser_t *make_parser(lexer_t *lexer) {
  parser_t *parser = malloc(sizeof(*parser));
  parser->lexer = lexer;
  parser_move(parser);
  return parser;
}

void match(parser_t *parser, enum TOKEN_TYPE type) {
  if (parser->look->type == type)
    parser_move(parser);
  else {
    fprintf(stderr, "Syntax error. Expecting token of type %d\n", type);
    exit(1);
  }
}

ast_node_t *make_node(enum NODE_TYPE type) {
  ast_node_t *node = malloc(sizeof(*node));
  node->type = type;
  return node;
}

ast_node_t *make_args_node(ast_node_t *arg, ast_node_t *rest) {
  ast_node_t *node = make_node(ARGS_NODE);
  node->u.args.arg = arg;
  node->u.args.rest = rest;
  return node;
}

ast_node_t *make_assign_node(ast_node_t *lv, ast_node_t *rv) {
  ast_node_t *node = make_node(ASSIGN_NODE);
  node->u.assign.lv = lv;
  node->u.assign.rv = rv;
  return node;
}

ast_node_t *make_call_node(ast_node_t *fn, ast_node_t *args) {
  ast_node_t *node = make_node(CALL_NODE);
  node->u.call.fn = fn;
  node->u.call.args = args;
  return node;
}

ast_node_t *make_num_node(int value) {
  ast_node_t *node = make_node(NUM_NODE);
  node->u.num_value = value;
  return node;
}

ast_node_t *make_offset_node(ast_node_t *array, ast_node_t *index) {
  ast_node_t *node = make_node(OFFSET_NODE);
  node->u.offset.array = array;
  node->u.offset.index = index;
  return node;
}

ast_node_t *make_arith_node(enum NODE_TYPE op, ast_node_t *left, ast_node_t *right) {
  ast_node_t *node = make_node(op);
  node->u.arith.left = left;
  node->u.arith.right = right;
  return node;
}

ast_node_t *make_id_node(char *id) {
  ast_node_t *node = malloc(sizeof(*node));
  node->type = ID_NODE;
  node->u.id = id;
  return node;
}

void write_node(ast_node_t *node) {
  if (!node)
    return;
  switch (node->type) {
    case NUM_NODE:
      printf("%d", node->u.num_value);
      break;
    case '+': case '-': case '*': case '/':
      write_node(node->u.arith.left);
      write_node(node->u.arith.right);
      printf("%c", node->type);
      break;
    case ID_NODE:
      printf("%s", node->u.id);
      break;
    case ASSIGN_NODE:
      write_node(node->u.assign.lv);
      write_node(node->u.assign.rv);
      printf("<=>");
      break;
    case CALL_NODE:
      write_node(node->u.call.fn);
      write_node(node->u.call.args);
      printf("<call>");
      break;
    case ARGS_NODE:
      write_node(node->u.args.arg);
      write_node(node->u.args.rest);
      printf("<args>");
      break;
    case OFFSET_NODE:
      write_node(node->u.offset.array);
      write_node(node->u.offset.index);
      printf("<[]>");
      break;
    default :
      printf("It's a bug for printing node of type %d\n", node->type);
      exit(1);
  }
}

ast_node_t *parse_args(parser_t *parser) {
  match(parser, '(');
  ast_node_t *head = make_args_node(NULL, NULL);
  ast_node_t *pre = head;
  while (parser->look->type != ')') {
    ast_node_t *node = make_args_node(parse_assign(parser), NULL);
    pre->u.args.rest = node;
    pre = node;
    if (parser->look->type != ')')
      match(parser, ',');
  }
  match(parser, ')');
  return head->u.args.rest;
}

ast_node_t *parse_offset(parser_t *parser) {
  match(parser, '[');
  ast_node_t *index = parse_assign(parser);
  match(parser, ']');
  return index;
}

ast_node_t *parse_postfix(ast_node_t *pre, parser_t *parser) {
  ast_node_t *x = pre;
  token_t *tk = parser->look;
  while (tk->type == '(' || tk->type == '[') {
    if (tk->type == '(')
      x = make_call_node(x, parse_args(parser));
    else
      x = make_offset_node(x, parse_offset(parser));
    tk = parser->look;
  }
  return x;
}

ast_node_t *parse_factor(parser_t *parser) {
  token_t *token = parser->look;
  ast_node_t *x = NULL;
  switch (token->type) {
    case NUM:
      x = make_num_node(token->u.number);
      parser_move(parser);
      return x;
    case ID:
      x = make_id_node(token->u.id);
      parser_move(parser);
      if (parser->look->type == '(' || parser->look->type == '[') {
//        return make_call_node(x, parse_args(parser));
        return parse_postfix(x, parser);
      } else
        return x;
    case '(':
      parser_move(parser);
      x = parse_assign(parser);
      match(parser, ')');
      return x;
    default :
      fprintf(stderr, "Syntax error\n");
      exit(1);
  }
}

ast_node_t *parse_term(parser_t *parser) {
  ast_node_t *node = parse_factor(parser);
  token_t *token = parser->look;
  while (token->type == '*' || token->type == '/') {
    parser_move(parser);
    node = make_arith_node(token->type, node, parse_factor(parser));
    token = parser->look;
  }
  return node;
}

ast_node_t *parse_expr(parser_t *parser) {
  ast_node_t *node = parse_term(parser);
  token_t *token = parser->look;
  while (token->type == '+' || token->type == '-') {
    parser_move(parser);
    node = make_arith_node(token->type, node, parse_term(parser));
    token = parser->look;
  }
  return node;
}

ast_node_t *parse_assign(parser_t *parser) {
  ast_node_t *node = parse_expr(parser);
  if (node->type == ID_NODE) {
    if (parser->look->type == '=') {
      parser_move(parser);
      return make_assign_node(node, parse_assign(parser));
    } else
      return node;
  } else if (parser->look->type == '=') {
    fprintf(stderr, "Syntax error: Invalid left-value\n");
    exit(1);
  } else
    return node;
}

ast_node_t *parse_prog(parser_t *parser) {
  return parse_assign(parser);
}

void convert_write(char *str) {
  printf("%s => ", str);
  lexer_t *lexer = make_lexer(str);
  parser_t *parser = make_parser(lexer);
  ast_node_t *node = parse_prog(parser);
  write_node(node);
  putchar('\n');
}

int main(int argc, char *argv[]) {
  convert_write("11*2+3/1");
  convert_write("1 + a");
  convert_write("(1)");
  convert_write("(2 * 3)");
  convert_write("9 - (5 + 2)");
  convert_write("(1 + 2) * 3");
  convert_write("var = 1 + 2");
  convert_write("var = val = (1 + 1) * 2");
  convert_write("fn()");
  convert_write("fn(1)");
  convert_write("fn(1, 2)");
  convert_write("a[1 + 1][2]");
  return 0;
}
