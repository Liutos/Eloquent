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
#include <string.h>

#include "object.h"
#include "prims.h"
#include "type.h"
#include "utilities.h"

typedef struct ast_node_t ast_node_t;
typedef struct lexer_t lexer_t;
typedef struct parser_t parser_t;
typedef struct token_t token_t;

enum TOKEN_TYPE {
  _TYPE_START_=255,
  ELSE,
  ID,
  IF,
  NUM,
  WHILE,
};

enum NODE_TYPE {
  ARGS_NODE,
  ASSIGN_NODE,
  CALL_NODE,
  ELSE_NODE,
  ID_NODE,
  IF_NODE,
  NUM_NODE,
  OFFSET_NODE,
  SEQ_NODE,
  WHILE_NODE,
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
    int num_value;
    char *id;
    struct {
      ast_node_t *arg, *rest;
    } args;
    struct {
      ast_node_t *left, *right;
    } arith;
    struct {
      ast_node_t *lv, *rv;
    } assign;
    struct {
      ast_node_t *fn, *args;
    } call;
    struct {
      ast_node_t *pred, *then_part, *else_part;
    } else_stmt;
    struct {
      ast_node_t *pred, *then_part;
    } if_stmt;
    struct {
      ast_node_t *array, *index;
    } offset;
    struct {
      ast_node_t *expr, *rest;
    } seq;
    struct {
      ast_node_t *test, *body;
    } while_stmt;
  } u;
};

struct parser_t {
  lexer_t *lexer;
  token_t *look;
};

void convert_write(char *);
ast_node_t *parse_assign(parser_t *);
ast_node_t *parse_stmt(parser_t *);

/* Token Constructors */
token_t *make_token(enum TOKEN_TYPE type) {
  token_t *tk = malloc(sizeof(*tk));
  tk->type = type;
  return tk;
}
//
//token_t *make_eof_token(void) {
//  token_t *tk = make_token(EOF);
//  return tk;
//}

token_t *make_id(char *id) {
  token_t *tk = make_token(ID);
  tk->u.id = id;
  return tk;
}

token_t *make_num(int number) {
  token_t *tk = make_token(NUM);
  tk->u.number = number;
  return tk;
}

token_t *make_operator(char op) {
  token_t *tk = make_token(op);
  return tk;
}

token_t *make_reserve(enum TOKEN_TYPE type) {
  token_t *tk = make_token(type);
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
  return make_num(n);
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
      return make_token(EOF);
    case ' ': case '\t': case '\n':
      move(lexer);
      return scan(lexer);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return get_num_token(lexer);
    case '+': case '-': case '*': case '/': case '=':
    case '^': case '!': case '[': case ']': case '(':
    case ')': case ',': case '{': case '}':
      move(lexer);
      return make_operator(c);
    default : {
      token_t *id = get_id_token(lexer);
      char *name = id->u.id;
      if (strcmp("if", name) == 0) {
        return make_reserve(IF);
      } else if (strcmp("else", name) == 0)
        return make_reserve(ELSE);
      else if (strcmp("while", name) == 0)
        return make_reserve(WHILE);
      else
        return id;
    }
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

ast_node_t *make_assign_node(ast_node_t *lv, ast_node_t *rv) {
  ast_node_t *node = make_node(ASSIGN_NODE);
  node->u.assign.lv = lv;
  node->u.assign.rv = rv;
  return node;
}

ast_node_t *make_args_node(ast_node_t *arg, ast_node_t *rest) {
  ast_node_t *node = make_node(ARGS_NODE);
  node->u.args.arg = arg;
  node->u.args.rest = rest;
  return node;
}

ast_node_t *make_arith_node(enum NODE_TYPE op, ast_node_t *left, ast_node_t *right) {
  ast_node_t *node = make_node(op);
  node->u.arith.left = left;
  node->u.arith.right = right;
  return node;
}

ast_node_t *make_call_node(ast_node_t *fn, ast_node_t *args) {
  ast_node_t *node = make_node(CALL_NODE);
  node->u.call.fn = fn;
  node->u.call.args = args;
  return node;
}

ast_node_t *make_else_node(ast_node_t *pred, ast_node_t *tp, ast_node_t *ep) {
  ast_node_t *node = make_node(ELSE_NODE);
  node->u.else_stmt.pred = pred;
  node->u.else_stmt.then_part = tp;
  node->u.else_stmt.else_part = ep;
  return node;
}

ast_node_t *make_id_node(char *id) {
  ast_node_t *node = malloc(sizeof(*node));
  node->type = ID_NODE;
  node->u.id = id;
  return node;
}

ast_node_t *make_if_node(ast_node_t *pred, ast_node_t *tp) {
  ast_node_t *node = make_node(IF);
  node->u.if_stmt.pred = pred;
  node->u.if_stmt.then_part = tp;
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

ast_node_t *make_seq_node(ast_node_t *expr, ast_node_t *rest) {
  ast_node_t *node = make_node(SEQ_NODE);
  node->u.seq.expr = expr;
  node->u.seq.rest = rest;
  return node;
}

ast_node_t *make_while_node(ast_node_t *test, ast_node_t *body) {
  ast_node_t *node = make_node(WHILE_NODE);
  node->u.while_stmt.test = test;
  node->u.while_stmt.body = body;
  return node;
}

void write_node(ast_node_t *node) {
  if (!node)
    return;
  switch (node->type) {
    case '+': case '-': case '*': case '/':
      write_node(node->u.arith.left);
      write_node(node->u.arith.right);
      printf("%c", node->type);
      break;
    case ASSIGN_NODE:
      write_node(node->u.assign.lv);
      write_node(node->u.assign.rv);
      printf("<=>");
      break;
    case ARGS_NODE:
      write_node(node->u.args.arg);
      write_node(node->u.args.rest);
      printf("<args>");
      break;
    case CALL_NODE:
      write_node(node->u.call.fn);
      write_node(node->u.call.args);
      printf("<call>");
      break;
    case ELSE_NODE:
      write_node(node->u.else_stmt.pred);
      write_node(node->u.else_stmt.then_part);
      write_node(node->u.else_stmt.else_part);
      printf("<else>");
      break;
    case ID_NODE:
      printf("%s", node->u.id);
      break;
    case IF_NODE:
      write_node(node->u.if_stmt.pred);
      write_node(node->u.if_stmt.then_part);
      printf("<if>");
      break;
    case NUM_NODE:
      printf("%d", node->u.num_value);
      break;
    case OFFSET_NODE:
      write_node(node->u.offset.array);
      write_node(node->u.offset.index);
      printf("<[]>");
      break;
    case SEQ_NODE:
      write_node(node->u.seq.expr);
      write_node(node->u.seq.rest);
      printf("<seq>");
      break;
    case WHILE_NODE:
      write_node(node->u.while_stmt.test);
      write_node(node->u.while_stmt.body);
      printf("<while>");
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

ast_node_t *parse_block(parser_t *parser) {
  match(parser, '{');
  ast_node_t *head = make_seq_node(NULL, NULL);
  ast_node_t *pre = head;
  while (parser->look->type != '}') {
    ast_node_t *node = make_seq_node(parse_stmt(parser), NULL);
    pre->u.seq.rest = node;
    pre = node;
  }
  match(parser, '}');
  return head->u.seq.rest;
}

ast_node_t *parse_stmt(parser_t *parser) {
  token_t *token = parser->look;
  ast_node_t *tp;
  switch (token->type) {
    case '{': {
      return parse_block(parser);
    }
      break;
    case IF: {
      match(parser, IF);
      match(parser, '(');
      ast_node_t *pred = parse_assign(parser);
      match(parser, ')');
      tp = parse_stmt(parser);
      token = parser->look;
      if (token->type != ELSE)
        return make_if_node(pred, tp);
      else {
        match(parser, ELSE);
        return make_else_node(pred, tp, parse_stmt(parser));
      }
    }
      break;
    case WHILE: {
      match(parser, WHILE);
      match(parser, '(');
      ast_node_t *test = parse_assign(parser);
      match(parser, ')');
      ast_node_t *body = parse_stmt(parser);
      return make_while_node(test, body);
    }
      break;
    default :
      return parse_assign(parser);
  }
}

ast_node_t *parse_prog(parser_t *parser) {
  return parse_stmt(parser);
}

void convert_write(char *str) {
  printf("%s => ", str);
  lexer_t *lexer = make_lexer(str);
  parser_t *parser = make_parser(lexer);
  ast_node_t *node = parse_prog(parser);
  write_node(node);
  putchar('\n');
}

lt *ast2lisp(ast_node_t *node) {
  assert(node != NULL);
  switch (node->type) {
    case NUM_NODE:
      return make_fixnum(node->u.num_value);
    default :
      fprintf(stderr, "I don't want to process type %d now.\n", node->type);
      exit(1);
  }
}

void write_ast_lisp(char *str) {
  printf("%s => ", str);
  lexer_t *l = make_lexer(str);
  parser_t *p = make_parser(l);
  ast_node_t *a = parse_prog(p);
  lt *obj = ast2lisp(a);
  write_object(obj, standard_out);
  putchar('\n');
}

int main(int argc, char *argv[]) {
  init_global_variable();
//  convert_write("11*2+3/1");
//  convert_write("1 + a");
//  convert_write("(1)");
//  convert_write("(2 * 3)");
//  convert_write("9 - (5 + 2)");
//  convert_write("(1 + 2) * 3");
//  convert_write("var = 1 + 2");
//  convert_write("var = val = (1 + 1) * 2");
//  convert_write("fn()");
//  convert_write("fn(1)");
//  convert_write("fn(1, 2)");
//  convert_write("a[1 + 1][2]");
//  convert_write("if ( x = 0 ) 0 - x else x");
//  convert_write("while (x = 0) {x + 1}");
//  convert_write("{a = 1}");
//  convert_write("{a = 1 b = 2}");
  write_ast_lisp("123");
  return 0;
}
