// gonna be real with u chief this is monolithic slop
// here be dragons, good luck

static Token peek_at(Parser *p, int n) {
    while (p->lookahead_count < n) {
        p->lookahead[p->lookahead_count++] = next_token(p->lx);
    }
    return p->lookahead[n - 1];
}

static void parser_advance(Parser *p) {
    p->previous = p->current;
    if (p->lookahead_count > 0) {
        p->current = p->lookahead[0];
        for (int i = 1; i < p->lookahead_count; i++) p->lookahead[i - 1] = p->lookahead[i];
        p->lookahead_count--;
    } else {
        p->current = next_token(p->lx);
    }
}

static bool check(Parser *p, TokenType type) {
    return p->current.type == type;
}

static bool match(Parser *p, TokenType type) {
    if (!check(p, type)) return false;
    parser_advance(p);
    return true;
}

static void parser_error(Parser *p, const char *msg) {
    p->had_error = true;
    printf("Parse error at line %d, col %d: %s (got '%.*s')\n",
           p->current.line, p->current.col, msg, p->current.len, p->current.start);
}

static void expect(Parser *p, TokenType type, const char *msg) {
    if (check(p, type)) {
        parser_advance(p);
        return;
    }
    parser_error(p, msg);
}

static Precedence binary_precedence(TokenType type) {
    switch (type) {
        case TOK_OR:            return PREC_OR;
        case TOK_AND:           return PREC_AND;
        case TOK_EQ:
        case TOK_NEQ:
        case TOK_IN:
                                return PREC_EQUALITY;
        case TOK_LT:
        case TOK_LTE:
        case TOK_GT:
        case TOK_GTE:
                                return PREC_COMPARISON;
        case TOK_SLICE:         return PREC_SLICE;
        case TOK_PLUS:
        case TOK_MINUS:
                                return PREC_ADDITIVE;
        case TOK_STAR:
        case TOK_SLASH:
        case TOK_PERCENT:
                                return PREC_MULTIPLICATIVE;
        default:                return PREC_NONE;
    }
}

static bool is_binary_op(TokenType type) {
    return binary_precedence(type) != PREC_NONE;
}

static AstNode* parse_primary(Parser *p) {
    int line = p->current.line, col = p->current.col;

    if (match(p, TOK_INT_LIT)) {
        return make_int_lit(p->previous.int_val, line, col);
    }
    if (match(p, TOK_FLOAT_LIT)) {
        return make_float_lit(p->previous.float_val, line, col);
    }
    if (match(p, TOK_STR_LIT)) {
        return make_str_lit(p->previous.start, p->previous.len, line, col);
    }
    if (match(p, TOK_RUNE_LIT)) {
        return make_rune_lit(p->previous.start, p->previous.len, line, col);
    }
    if (match(p, TOK_TRUE)) {
        return make_bool_lit(true, line, col);
    }
    if (match(p, TOK_FALSE)) {
        return make_bool_lit(false, line, col);
    }
    if (match(p, TOK_NULL)) {
        return make_null_lit(line, col);
    }
    if (match(p, TOK_IDENT)) {
        return make_ident(p->previous.start, p->previous.len, line, col);
    }
    if (check(p, TOK_FN)) {
        int line2 = p->current.line, col2 = p->current.col;
        parser_advance(p);
        return parse_fn_lit(p, line2, col2);
    }
    if (is_type_keyword(p->current.type)) {
        parser_advance(p);
        return make_ident(p->previous.start, p->previous.len, line, col);
    }
    if (match(p, TOK_LPAREN)) {
        AstNode *expr = parse_expression(p);
        expect(p, TOK_RPAREN, "expected ')' after expression");
        return expr;
    }
    if (check(p, TOK_LBRACKET)) {
        return parse_bracket_literal(p);
    }
    if (check(p, TOK_LBRACE)) {
        return parse_brace_literal(p);
    }
    if (check(p, TOK_IF)) {
        return parse_if_stmt(p);
    }

    parser_error(p, "expected expression");
    parser_advance(p);
    return make_null_lit(line, col);
}

static AstNode* parse_postfix(Parser *p) {
    AstNode *expr = parse_primary(p);

    for (;;) {
        int line = p->current.line, col = p->current.col;

        if (match(p, TOK_LPAREN)) {
            AstList args;
            ast_list_init(&args);
            if (!check(p, TOK_RPAREN)) {
                do {
                    ast_list_push(&args, parse_union(p));
                } while (match(p, TOK_COMMA));
            }
            expect(p, TOK_RPAREN, "expected ')' after arguments");

            bool is_type_ident = false;
            if (expr && expr->type == IDENT) {
                const char *nm = expr->as.ident.name;
                int len = expr->as.ident.len;
                if ((len == 4 && strncmp(nm, "rune", 4) == 0) ||
                    (len == 6 && strncmp(nm, "string", 6) == 0) ||
                    (len == 4 && strncmp(nm, "bool", 4) == 0) ||
                    (len == 2 && (strncmp(nm, "i8", 2) == 0 || strncmp(nm, "u8", 2) == 0)) ||
                    (len == 3 && (strncmp(nm, "i16", 3) == 0 || strncmp(nm, "u16", 3) == 0 || strncmp(nm, "f16", 3) == 0)) ||
                    (len == 3 && (strncmp(nm, "i32", 3) == 0 || strncmp(nm, "u32", 3) == 0 || strncmp(nm, "f32", 3) == 0)) ||
                    (len == 3 && (strncmp(nm, "i64", 3) == 0 || strncmp(nm, "u64", 3) == 0 || strncmp(nm, "f64", 3) == 0)))
                {
                    is_type_ident = true;
                }
            }

            if (is_type_ident) {
                int arg_count = args.count;
                if (arg_count == 1) {
                    AstNode **items = ast_list_finish(&args, &arg_count);
                    AstNode *arg_expr = items[0];
                    expr = make_cast(expr, arg_expr, line, col);
                } else {
                    expr = make_call(expr, &args, line, col);
                }
            } else {
                expr = make_call(expr, &args, line, col);
            }
        } else if (match(p, TOK_LBRACKET)) {
            AstNode *index = parse_expression(p);
            expect(p, TOK_RBRACKET, "expected ']' after index");
            expr = make_index(expr, index, line, col);
        } else if (match(p, TOK_DOT)) {
            expect(p, TOK_IDENT, "expected field name after '.'");
            expr = make_field_access(expr, p->previous.start, p->previous.len, line, col);
        } else if (match(p, TOK_PLUSPLUS)) {
            expr = make_postfix(TOK_PLUSPLUS, expr, line, col);
        } else if (match(p, TOK_MINUSMINUS)) {
            expr = make_postfix(TOK_MINUSMINUS, expr, line, col);
        } else {
            break;
        }
    }

    return expr;
}

static AstNode* parse_unary(Parser *p) {
    int line = p->current.line, col = p->current.col;

    if (check(p, TOK_NOT) || check(p, TOK_MINUS)) {
        TokenType op = p->current.type;
        parser_advance(p);
        AstNode *operand = parse_unary(p);
        return make_unary(op, operand, line, col);
    }

    return parse_postfix(p);
}

static AstNode* parse_binary(Parser *p, Precedence min_prec) {
    AstNode *left = parse_unary(p);

    for (;;) {
        TokenType op = p->current.type;

        if (!is_binary_op(op)) break;

        Precedence prec = binary_precedence(op);
        if (prec < min_prec) break;

        int line = p->current.line, col = p->current.col;
        parser_advance(p);

        AstNode *right = parse_binary(p, (Precedence)(prec + 1));
        left = make_binary(op, left, right, line, col);
    }

    return left;
}

static AstNode* parse_ternary(Parser *p) {
    AstNode *cond = parse_binary(p, PREC_OR);

    if (match(p, TOK_QUESTION)) {
        int line = p->previous.line, col = p->previous.col;
        AstNode *then_expr = parse_expression_no_union(p);
        expect(p, TOK_COLON, "expected ':' in ternary expression");
        AstNode *else_expr = parse_ternary(p);
        return make_ternary(cond, then_expr, else_expr, line, col);
    }

    return cond;
}

static AstNode* parse_expression_no_union(Parser *p) {
    return parse_ternary(p);
}

static AstNode* parse_union(Parser *p) {
    AstNode *left = parse_ternary(p);

    while (match(p, TOK_PIPE)) {
        int line = p->previous.line, col = p->previous.col;
        AstNode *right = parse_ternary(p);
        left = make_binary(TOK_PIPE, left, right, line, col);
    }

    return left;
}

static AstNode* parse_concat(Parser *p) {
    AstNode *left = parse_union(p);

    while (match(p, TOK_COMMA)) {
        int line = p->previous.line, col = p->previous.col;
        AstNode *right = parse_union(p);
        left = make_binary(TOK_COMMA, left, right, line, col);
    }

    return left;
}

static AstNode* parse_expression(Parser *p) {
    return parse_concat(p);
}

static AstNode* parse_block(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_LBRACE, "expected '{'");

    AstList stmts;
    ast_list_init(&stmts);

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        ast_list_push(&stmts, parse_statement(p));
    }

    expect(p, TOK_RBRACE, "expected '}' after block");
    return make_block(&stmts, line, col);
}

static bool is_assign_op(TokenType type) {
    switch (type) {
        case TOK_ASSIGN:
        case TOK_PLUSEQ: case TOK_MINUSEQ:
        case TOK_STAREQ: case TOK_SLASHEQ:
            return true;
        default:
            return false;
    }
}

static AstNode* parse_expr_statement(Parser *p) {
    int line = p->current.line, col = p->current.col;
    AstNode *expr = parse_expression(p);

    if (expr->type == IDENT && (check(p, TOK_MUT) || check(p, TOK_IMMUT))) {
        bool is_immutable = check(p, TOK_IMMUT);
        parser_advance(p);
        AstNode *value = parse_expression(p);

        if (match(p, TOK_SEMICOLON)) {
        } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
        } else {
            parser_error(p, "expected newline or ';' after declaration");
        }

        return is_immutable
            ? make_immut_decl(expr->as.ident.name, expr->as.ident.len, false, nullptr, value, line, col)
            : make_mut_decl(expr->as.ident.name, expr->as.ident.len, false, nullptr, value, line, col);
    }

    if (is_assign_op(p->current.type)) {
        TokenType op = p->current.type;
        parser_advance(p);
        AstNode *value = parse_expression(p);

        if (match(p, TOK_SEMICOLON)) {
        } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
        } else {
            parser_error(p, "expected newline or ';' after assignment");
        }

        return make_assign(expr, op, value, line, col);
    }

    if (match(p, TOK_SEMICOLON)) {
    } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
    } else {
        parser_error(p, "expected newline or ';' after expression statement");
    }

    return make_expr_stmt(expr, line, col);
}

static AstNode* parse_bracket_literal(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_LBRACKET, "expected '['");

    if (match(p, TOK_RBRACKET)) {
        return make_array_lit(nullptr, 0, line, col);
    }

    AstList elems;
    ast_list_init(&elems);
    do {
        int elem_line = p->current.line, elem_col = p->current.col;
        AstNode *first = parse_union(p);

        if (match(p, TOK_COLON)) {
            AstNode *value = parse_union(p);
            ast_list_push(&elems, make_array_entry(first, value, elem_line, elem_col));
        } else {
            ast_list_push(&elems, first);
        }
    } while (match(p, TOK_COMMA));
    expect(p, TOK_RBRACKET, "expected ']' after array elements");

    int count;
    AstNode **items = ast_list_finish(&elems, &count);
    return make_array_lit(items, count, line, col);
}

static AstNode* parse_brace_literal(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_LBRACE, "expected '{'");

    AstList fields;
    ast_list_init(&fields);

    if (!check(p, TOK_RBRACE)) {
        do {
            int field_line = p->current.line, field_col = p->current.col;
            expect(p, TOK_IDENT, "expected field name in struct literal");
            const char *name = p->previous.start;
            int name_len = p->previous.len;

            expect(p, TOK_COLON, "expected ':' after field name");
            AstNode *value = parse_union(p);

            ast_list_push(&fields, make_field(name, name_len, nullptr, value, field_line, field_col));
        } while (match(p, TOK_COMMA));
    }

    expect(p, TOK_RBRACE, "expected '}' after struct literal");

    int count;
    AstNode **items = ast_list_finish(&fields, &count);
    return make_struct_lit(items, count, line, col);
}

static bool is_type_keyword(TokenType type) {
    switch (type) {
        case TOK_TYPE_RUNE: case TOK_TYPE_STR: case TOK_TYPE_BOOL:
        case TOK_TYPE_I8: case TOK_TYPE_I16: case TOK_TYPE_I32: case TOK_TYPE_I64:
        case TOK_TYPE_U8: case TOK_TYPE_U16: case TOK_TYPE_U32: case TOK_TYPE_U64:
        case TOK_TYPE_F16: case TOK_TYPE_F32: case TOK_TYPE_F64: case TOK_TYPE_EXT:
            return true;
        default:
            return false;
    }
}

static bool at_type_annotation(Parser *p) {
    if (is_type_keyword(p->current.type)) return true;
    if (check(p, TOK_IDENT) && peek_at(p, 1).type == TOK_IDENT) return true;
    return false;
}

static AstNode* parse_decl(Parser *p) {
    int line = p->current.line, col = p->current.col;

    AstNode *type_annotation = nullptr;
    if (at_type_annotation(p)) {
        type_annotation = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
        parser_advance(p);
    }

    expect(p, TOK_IDENT, "expected identifier in declaration");
    bool has_type_annotation = (type_annotation != nullptr);
    const char *name = p->previous.start;
    int name_len = p->previous.len;

    bool is_immutable = false;
    if (match(p, TOK_MUT)) {
        is_immutable = false;
    } else if (match(p, TOK_IMMUT)) {
        is_immutable = true;
    } else {
        parser_error(p, "expected ':=' or '::' in declaration");
    }

    AstNode *value = parse_expression(p);

    if (match(p, TOK_SEMICOLON)) {
    } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
    } else {
        parser_error(p, "expected newline or ';' after declaration");
    }

    return is_immutable
        ? make_immut_decl(name, name_len, has_type_annotation, type_annotation, value, line, col)
        : make_mut_decl(name, name_len, has_type_annotation, type_annotation, value, line, col);
}

static AstNode* parse_statement(Parser *p) {
    if (check(p, TOK_IF))       return parse_if_stmt(p);
    if (check(p, TOK_WHILE))    return parse_while_stmt(p);
    if (check(p, TOK_SWITCH))   return parse_switch_stmt(p);
    if (check(p, TOK_FOR))      return parse_for_dispatch(p);
    if (check(p, TOK_FN))       return parse_fn_decl(p);
    if (check(p, TOK_IMPORT))   return parse_import(p);
    if (check(p, TOK_PROCTIME)) return parse_proctime_stmt(p);
    if (check(p, TOK_STRUCT))   return parse_struct_decl(p);
    if (check(p, TOK_ENUM))     return parse_enum_decl(p);
    if (check(p, TOK_TRY))      return parse_try_stmt(p);
    
    if (check(p, TOK_RETURN)) {
        return parse_return_stmt(p, p->current.line, p->current.col);
    }
    
    if (check(p, TOK_DEFER)) {
        return parse_defer_stmt(p, p->current.line, p->current.col);
    }

    if (check(p, TOK_BREAK)) {
        return parse_break_stmt(p, p->current.line, p->current.col);
    }

    if (check(p, TOK_CONTINUE)) {
        return parse_continue_stmt(p, p->current.line, p->current.col);
    }

    if (check(p, TOK_EXIT)) {
        return parse_exit_stmt(p, p->current.line, p->current.col);
    }

    if (is_type_keyword(p->current.type)) {
        Token next = peek_at(p, 1);
        if (next.type == TOK_IDENT) {
            return parse_decl(p);
        }
        if (next.type == TOK_LBRACE) {
            return parse_type_block(p);
        }
    }

    if (check(p, TOK_IDENT) && peek_at(p, 1).type == TOK_IDENT) {
        return parse_decl(p);
    }

    return parse_expr_statement(p);
}

static AstNode* parse_program(Parser *p) {
    int line = p->current.line, col = p->current.col;

    AstList stmts;
    ast_list_init(&stmts);

    while (!check(p, TOK_EOF)) {
        ast_list_push(&stmts, parse_statement(p));
    }

    return make_program(&stmts, line, col);
}

static AstNode* parse_if_stmt(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_IF, "expected 'if'");

    AstNode *cond = parse_expression(p);
    AstNode *then_branch = parse_block(p);

    AstNode *else_branch = nullptr;
    if (match(p, TOK_ELSE)) {
        if (check(p, TOK_IF)) {
            else_branch = parse_if_stmt(p);
        } else {
            else_branch = parse_block(p);
        }
    }

    return make_if_stmt(cond, then_branch, else_branch, line, col);
}

static AstNode* parse_while_stmt(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_WHILE, "expected 'while'");

    AstNode *cond = parse_expression(p);
    AstNode *body = parse_block(p);

    return make_while_stmt(cond, body, line, col);
}

static AstNode* parse_switch_stmt(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_SWITCH, "expected 'switch'");

    AstNode *scrutinee = parse_expression(p);
    AstList cases;
    ast_list_init(&cases);

    expect(p, TOK_LBRACE, "expected '{'");

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        ast_list_push(&cases, parse_switch_case(p));
    }

    expect(p, TOK_RBRACE, "expected '}'");

    return make_switch_stmt(scrutinee, &cases, line, col);
}

static AstNode* parse_switch_case(Parser *p) {
    int line = p->current.line, col = p->current.col;

    AstNode *label;

    if (check(p, TOK_DEFAULT)) {
        parser_advance(p);
        label = make_default_stmt(line, col);
    } else {
        label = parse_expression(p);
    }

    expect(p, TOK_COLON, "expected ':' after switch label");

    AstList stmts;
    ast_list_init(&stmts);

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        if (p->current.preceded_nl &&
            (check(p, TOK_INT_LIT) ||
            check(p, TOK_FLOAT_LIT) ||
            check(p, TOK_STR_LIT) ||
            check(p, TOK_RUNE_LIT) ||
            check(p, TOK_IDENT) ||
            check(p, TOK_TRUE) ||
            check(p, TOK_FALSE) ||
            check(p, TOK_NULL) ||
            check(p, TOK_TYPE_RUNE) ||
            check(p, TOK_TYPE_STR) ||
            check(p, TOK_TYPE_BOOL) ||
            check(p, TOK_TYPE_I8) ||
            check(p, TOK_TYPE_I16) ||
            check(p, TOK_TYPE_I32) ||
            check(p, TOK_TYPE_I64) ||
            check(p, TOK_TYPE_U8) ||
            check(p, TOK_TYPE_U16) ||
            check(p, TOK_TYPE_U32) ||
            check(p, TOK_TYPE_U64) ||
            check(p, TOK_TYPE_F16) ||
            check(p, TOK_TYPE_F32) ||
            check(p, TOK_TYPE_F64) ||
            check(p, TOK_DEFAULT)))
        {
            Token next = peek_at(p, 1);
            if (next.type == TOK_COLON) {
                break;
            }
            if (next.type == TOK_SLICE) {
                Token after_slice_end = peek_at(p, 3);
                if (after_slice_end.type == TOK_COLON) {
                    break;
                }
            }
        }

        ast_list_push(&stmts, parse_statement(p));
    }

    AstNode *body = make_block(&stmts, line, col);

    return make_switch_case(label, body, line, col);
}

static AstNode* parse_for_in(Parser *p, int line, int col, bool has_parens) {
    expect(p, TOK_IDENT, "expected identifier in for-in loop");
    const char *key_name = p->previous.start;
    int key_len = p->previous.len;

    expect(p, TOK_COMMA, "expected ',' after first identifier in for-in loop");

    expect(p, TOK_IDENT, "expected second identifier in for-in loop");
    const char *val_name = p->previous.start;
    int val_len = p->previous.len;

    expect(p, TOK_IN, "expected 'in' in for-in loop");
    AstNode *iterable = parse_expression(p);

    if (has_parens) {
        expect(p, TOK_RPAREN, "expected ')' to close for-loop header");
    }

    AstNode *body = parse_block(p);
    return make_for_in(key_name, key_len, val_name, val_len, iterable, body, line, col);
}


static AstNode* parse_for_init_decl(Parser *p) {
    int line = p->current.line, col = p->current.col;

    AstNode *type_annotation = nullptr;
    if (is_type_keyword(p->current.type)) {
        type_annotation = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
        parser_advance(p);
    }

    expect(p, TOK_IDENT, "expected identifier in for-loop init");
    const char *name = p->previous.start;
    int name_len = p->previous.len;

    if (check(p, TOK_IMMUT)) {
        parser_error(p, "for-loop init must use ':=', not '::' (loop counters can't be proctime-immutable)");
        parser_advance(p);
    } else {
        expect(p, TOK_MUT, "expected ':=' in for-loop init");
    }

    AstNode *value = parse_expression(p);

    return make_mut_decl(name, name_len, type_annotation != nullptr, type_annotation, value, line, col);
}

static AstNode* parse_for_cstyle(Parser *p, int line, int col, bool has_parens) {
    AstNode *init = parse_for_init_decl(p);
    expect(p, TOK_SEMICOLON, "expected ';' after for-loop init");

    AstNode *cond = parse_expression(p);
    expect(p, TOK_SEMICOLON, "expected ';' after for-loop condition");

    AstNode *incr = parse_expression(p);

    if (has_parens) {
        expect(p, TOK_RPAREN, "expected ')' to close for-loop header");
    }

    AstNode *body = parse_block(p);
    return make_for_cstyle(init, cond, incr, body, line, col);
}

static AstNode* parse_for_range(Parser *p, int line, int col, bool has_parens) {
    AstNode *start = parse_binary(p, (Precedence)(PREC_SLICE + 1));

    AstNode *end = nullptr;
    if (match(p, TOK_SLICE)) {
        end = parse_binary(p, (Precedence)(PREC_SLICE + 1));
    }

    if (has_parens) {
        expect(p, TOK_RPAREN, "expected ')' to close for-loop header");
    }

    AstNode *body = parse_block(p);
    return make_for_range(start, end, body, line, col);
}

static AstNode* parse_for_dispatch(Parser *p) {
    int line = p->current.line, col = p->current.col;
    parser_advance(p);

    bool has_parens = match(p, TOK_LPAREN);

    if (check(p, TOK_IDENT)) {
        Token after = peek_at(p, 1);
        if (after.type == TOK_COMMA) return parse_for_in(p, line, col, has_parens);
        if (after.type == TOK_MUT || after.type == TOK_IMMUT) return parse_for_cstyle(p, line, col, has_parens);
    }
    return parse_for_range(p, line, col, has_parens);
}

static AstNode* parse_return_stmt(Parser *p, int line, int col) {
    expect(p, TOK_RETURN, "expected 'return'");

    AstNode *value = nullptr;
    if (!(p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF))) {
        value = parse_expression(p);
    }

    return make_return_stmt(value, line, col);
}

static AstNode* parse_defer_stmt(Parser *p, int line, int col) {
    expect(p, TOK_DEFER, "expected 'defer'");

    AstNode *value = nullptr;
    if (!(p->current.preceded_nl || check(p, TOK_EOF))) {
        value = parse_expression(p);
    }

    return make_defer_stmt(value, line, col);
}

static AstNode* parse_break_stmt(Parser *p, int line, int col) {
    expect(p, TOK_BREAK, "expected 'break'");
    return make_break_stmt(line, col);
}

static AstNode* parse_continue_stmt(Parser *p, int line, int col) {
    expect(p, TOK_CONTINUE, "expected 'continue'");
    return make_continue_stmt(line, col);
}

static AstNode* parse_exit_stmt(Parser *p, int line, int col) {
    expect(p, TOK_EXIT, "expected 'exit'");
    return make_exit_stmt(line, col);
}

static AstNode** parse_param_list(Parser *p, int *out_count) {
    expect(p, TOK_LPAREN, "expected '(' to start parameter list");

    AstList params;
    ast_list_init(&params);

    if (!check(p, TOK_RPAREN)) {
        do {
            int line = p->current.line, col = p->current.col;

            AstNode *type_expr = nullptr;
            if (at_type_annotation(p)) {
                type_expr = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
                parser_advance(p);
            }

            expect(p, TOK_IDENT, "expected parameter name");
            const char *name = p->previous.start;
            int name_len = p->previous.len;

            AstNode *default_value = nullptr;
            if (match(p, TOK_MUT)) {
                default_value = parse_union(p);
            }

            ast_list_push(&params, make_param(name, name_len, type_expr, default_value, line, col));
        } while (match(p, TOK_COMMA));
    }

    expect(p, TOK_RPAREN, "expected ')' after parameter list");
    return ast_list_finish(&params, out_count);
}

static AstNode* parse_fn_lit(Parser *p, int line, int col) {
    int param_count;
    AstNode **params = parse_param_list(p, &param_count);
    AstNode *return_type = parse_optional_return_type(p);
    AstNode *body = parse_block(p);
    return make_fn_lit(params, param_count, return_type, body, line, col);
}

static AstNode* parse_fn_decl(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_FN, "expected 'fn'");

    expect(p, TOK_IDENT, "expected function name after 'fn'");
    const char *name = p->previous.start;
    int name_len = p->previous.len;

    expect(p, TOK_IMMUT, "expected '::' after function name");

    int param_count;
    AstNode **params = parse_param_list(p, &param_count);
    AstNode *return_type = parse_optional_return_type(p);
    AstNode *body = parse_block(p);

    return make_fn_decl(name, name_len, params, param_count, return_type, body, line, col);
}

static AstNode* parse_optional_return_type(Parser *p) {
    if (!match(p, TOK_ARROW)) return nullptr;
    if (!is_type_keyword(p->current.type) && !check(p, TOK_IDENT)) {
        parser_error(p, "expected return type after '->'");
        return nullptr;
    }
    AstNode *type_expr = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
    parser_advance(p);
    return type_expr;
}

static AstNode* parse_struct_decl(Parser *p) {
    int line = p->current.line, col = p->current.col;

    expect(p, TOK_STRUCT, "expected 'struct'");
    expect(p, TOK_IDENT, "expected struct name");

    const char *name = p->previous.start;
    int name_len = p->previous.len;
    expect(p, TOK_LBRACE, "expected '{'");

    AstList fields;
    ast_list_init(&fields);
    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        int field_line = p->current.line, field_col = p->current.col;

        AstNode *type_expr = nullptr;
        if (at_type_annotation(p)) {
            type_expr = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
            parser_advance(p);
        }

        expect(p, TOK_IDENT, "expected field name");

        const char *fname = p->previous.start;
        int fname_len = p->previous.len;
        expect(p, TOK_COLON, "expected ':' after field name");
        AstNode *value = parse_union(p);

        ast_list_push(&fields, make_field(fname, fname_len, type_expr, value, field_line, field_col));
        if (!match(p, TOK_COMMA)) break;
    }

    expect(p, TOK_RBRACE, "expected '}'");

    int count;
    AstNode **items = ast_list_finish(&fields, &count);
    return make_struct_decl(name, name_len, items, count, line, col);
}

static AstNode* parse_enum_decl(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_ENUM, "expected 'enum'");
    expect(p, TOK_IDENT, "expected enum name");
    const char *name = p->previous.start;
    int name_len = p->previous.len;
    expect(p, TOK_LBRACE, "expected '{'");

    AstList variants;
    ast_list_init(&variants);
    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        int vline = p->current.line, vcol = p->current.col;
        expect(p, TOK_IDENT, "expected enum variant name");
        const char *vname = p->previous.start;
        int vlen = p->previous.len;
        AstNode *value = nullptr;
        if (match(p, TOK_COLON)) {
            value = parse_union(p);
        }
        ast_list_push(&variants, make_field(vname, vlen, nullptr, value, vline, vcol));
        if (!match(p, TOK_COMMA)) break;
    }
    expect(p, TOK_RBRACE, "expected '}'");
    int count;
    AstNode **items = ast_list_finish(&variants, &count);
    return make_enum_decl(name, name_len, items, count, line, col);
}

static AstNode* parse_type_block(Parser *p) {
    int line = p->current.line, col = p->current.col;

    TokenType type_annotation = p->current.type;
    AstNode *field_type = make_ident(p->current.start, p->current.len, p->current.line, p->current.col);
    parser_advance(p);

    expect(p, TOK_LBRACE, "expected '{' after type in mass type declaration");

    AstList decls;
    ast_list_init(&decls);

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        expect(p, TOK_IDENT, "expected identifier in mass type declaration");
        const char *name = p->previous.start;
        int name_len = p->previous.len;

        bool is_immutable = false;
        if (match(p, TOK_MUT)) {
            is_immutable = false;
        } else if (match(p, TOK_IMMUT)) {
            is_immutable = true;
        } else {
            parser_error(p, "expected ':=' or '::' in mass type declaration");
        }

        AstNode *value = parse_expression(p);

        if (match(p, TOK_SEMICOLON)) {
        } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
        } else {
            parser_error(p, "expected newline or ';' after declaration in type block");
        }

        AstNode *decl = is_immutable
            ? make_immut_decl(name, name_len, true, field_type, value, line, col)
            : make_mut_decl(name, name_len, true, field_type, value, line, col);
            
        ast_list_push(&decls, decl);
    }

    expect(p, TOK_RBRACE, "expected '}' after mass type declaration");

    int count;
    AstNode **items = ast_list_finish(&decls, &count);
    return make_type_block(type_annotation, items, count, line, col);
}

static AstNode* parse_try_stmt(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_TRY, "expected 'try'");

    AstNode *try_block = parse_block(p);

    expect(p, TOK_CATCH, "expected 'catch' after try block");
    expect(p, TOK_LPAREN, "expected '(' after 'catch'");
    expect(p, TOK_IDENT, "expected error identifier in catch clause");
    const char *catch_name = p->previous.start;
    int catch_len = p->previous.len;
    expect(p, TOK_RPAREN, "expected ')' after catch identifier");

    AstNode *catch_block = parse_block(p);

    return make_try_stmt(try_block, catch_name, catch_len, catch_block, line, col);
}

static AstNode* parse_import(Parser *p) {
    int line = p->current.line, col = p->current.col;
    expect(p, TOK_IMPORT, "expected '#import'");
    expect(p, TOK_STR_LIT, "expected import path string");
    const char *path = p->previous.start;
    int path_len = p->previous.len;

    const char *alias = nullptr;
    int alias_len = 0;
    if (match(p, TOK_AS)) {
        expect(p, TOK_IDENT, "expected alias name after 'as'");
        alias = p->previous.start;
        alias_len = p->previous.len;
    }

    if (match(p, TOK_SEMICOLON)) {
    } else if (p->current.preceded_nl || check(p, TOK_RBRACE) || check(p, TOK_EOF)) {
    } else {
        parser_error(p, "expected newline or ';' after import");
    }

    return make_import(path, path_len, alias, alias_len, line, col);
}

static AstNode* parse_proctime_stmt(Parser *p) {
    expect(p, TOK_PROCTIME, "expected '#proctime'");

    int line = p->current.line, col = p->current.col;
    expect(p, TOK_LBRACE, "expected '{' after '#proctime'");

    AstList stmts;
    ast_list_init(&stmts);
    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        ast_list_push(&stmts, parse_statement(p));
    }
    expect(p, TOK_RBRACE, "expected '}' after #proctime block");

    AstNode *node = make_proctime_block(&stmts, line, col);
    node->is_proctime = true;
    return node;
}

// you have now escaped the 7th layer of data hell