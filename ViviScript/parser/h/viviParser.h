struct Parser {
    Lexer *lx;
    Token current;
    Token previous;
    bool had_error;

    Token lookahead[3];
    int lookahead_count;
};

enum Precedence {
    PREC_NONE,
    PREC_UNION,             // |
    PREC_TERNARY,           // ?:
    PREC_OR,                // or, ||
    PREC_AND,               // and, &&
    PREC_EQUALITY,          // == != not in
    PREC_COMPARISON,        // < <= > >=
    PREC_SLICE,             // ..
    PREC_ADDITIVE,          // + -
    PREC_MULTIPLICATIVE,    // * / %
    PREC_UNARY,             // !
    PREC_POSTFIX,           // () [] . ++ --
};

// Helpers
static void parser_advance(Parser *p);
static bool check(Parser *p, TokenType type);
static bool match(Parser *p, TokenType type);
static void expect(Parser *p, TokenType type, const char *msg);
static void parser_error(Parser *p, const char *msg);
static bool is_type_keyword(TokenType type);

// Expr
static AstNode* parse_expression(Parser *p);
static AstNode* parse_expression_no_union(Parser *p);
static AstNode* parse_bracket_literal(Parser *p);
static AstNode* parse_brace_literal(Parser *p);
static AstNode* parse_fn_lit(Parser *p, int line, int col);
static AstNode** parse_param_list(Parser *p, int *out_count);
static AstNode* parse_optional_return_type(Parser *p);
static AstNode* parse_import(Parser *p);
static AstNode* parse_proctime_stmt(Parser *p);
static AstNode* parse_union(Parser *p);

// Decl
static AstNode* parse_decl(Parser *p);

// Stmt
static AstNode* parse_statement(Parser *p);
static AstNode* parse_block(Parser *p);
static AstNode* parse_program(Parser *p);
static AstNode* parse_if_stmt(Parser *p);
static AstNode* parse_while_stmt(Parser *p);
static AstNode* parse_switch_stmt(Parser *p);
static AstNode* parse_switch_case(Parser *p);
static AstNode* parse_return_stmt(Parser *p, int line, int col);
static AstNode* parse_defer_stmt(Parser *p, int line, int col);
static AstNode* parse_break_stmt(Parser *p, int line, int col);
static AstNode* parse_continue_stmt(Parser *p, int line, int col);
static AstNode* parse_exit_stmt(Parser *p, int line, int col);
static AstNode* parse_fn_decl(Parser *p);
static AstNode* parse_struct_decl(Parser *p);
static AstNode* parse_enum_decl(Parser *p);
static AstNode* parse_type_block(Parser *p);
static AstNode* parse_try_stmt(Parser *p);

// For loops
static AstNode* parse_for_dispatch(Parser *p);
static AstNode* parse_for_cstyle(Parser *p, int line, int col, bool has_parens);
static AstNode* parse_for_in(Parser *p, int line, int col, bool has_parens);
static AstNode* parse_for_range(Parser *p, int line, int col, bool has_parens);