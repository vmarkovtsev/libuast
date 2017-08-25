#ifndef LIBUAST_FUNCTION_H
#define LIBUAST_FUNCTION_H

typedef struct {
  // Line number where the function begins
  int lineno_begin;
  // Line number where the function ends; the function length is (lineno_end - lineno_begin)
  int lineno_end;
  // Function body
  void *subtree;
  // Function scope - node path to subtree
  void **scope;
  // Number of items in scope
  int scope_size;
} uast_function;

#endif //LIBUAST_FUNCTION_H
