enum AstType {
    // Lits
    INT_LIT, FLOAT_LIT, STR_LIT, RUNE_LIT, BOOL_LIT, NULL_LIT, IDENT,

    // Collection lits
    ARRAY_LIT, ARRAY_ENTRY, STRUCT_LIT,

    // Expr
    BINARY, UNARY, POSTFIX, TERNARY, CALL, INDEX,
    FIELD_ACCESS, CAST, FN_LIT,

    // Decl
    IMMUT_DECL, MUT_DECL, FN_DECL, STRUCT_DECL, ENUM_DECL,
    PARAM, FIELD,

    // Statements
    EXPR_STMT, ASSIGN, BLOCK, IF_STMT, WHILE_STMT,
    FOR_RANGE, FOR_CSTYLE, FOR_IN,
    SWITCH_STMT, SWITCH_CASE, DEFAULT_STMT,
    BREAK_STMT, CONTINUE_STMT, RETURN_STMT, DEFER_STMT, EXIT_STMT,
    TRY_STMT, CATCH_STMT,

    // Type block
    TYPE_BLOCK,

    // Proctime
    PROCTIME_BLOCK,

    // Modules
    IMPORT,

    // Root
    PROGRAM
};

enum class ValueType {
    Null, Bool, Int, Float, Str,
    Array, Struct, Fn
};

struct AstNode {
    AstType type;
    int line, col;
    bool is_proctime;

    union {
        struct { long value; } int_lit;
        struct { double value; } float_lit;
        struct { const char *start; int len; } str_lit;
        struct { const char *start; int len; } rune_lit;
        struct { bool value; } bool_lit;
        struct { const char *name; int len; } ident;

        struct { TokenType op; struct AstNode *left, *right; } binary;
        struct { TokenType op; struct AstNode *operand; } unary;
        struct { struct AstNode *cond, *then_expr, *else_expr; } ternary;
        struct { struct AstNode *callee; struct AstNode **args; int arg_count; } call;
        struct { struct AstNode *object, *index; } index;
        struct { struct AstNode *object; const char *field; int len; } field_access;

        struct { const char *name; int len; struct AstNode **params; int param_count; AstNode *return_type; struct AstNode *body; } fn_decl;
        struct { const char *name; int len; struct AstNode **fields; int field_count; } struct_decl;

        struct { const char *name; int len; struct AstNode **variants; int variant_count; } enum_decl;

        struct { struct AstNode *target; TokenType op; struct AstNode *value; } assign;
        struct { struct AstNode **stmts; int stmt_count; } block;

        struct { struct AstNode *cond; struct AstNode *then_branch; struct AstNode *else_branch; } if_stmt;
        struct { struct AstNode *cond; struct AstNode *body; } while_stmt;
        struct { struct AstNode *start, *end; struct AstNode *body; } for_range;
        struct { struct AstNode *init, *cond, *incr; struct AstNode *body; } for_cstyle;
        struct { const char *key_name; int key_len; const char *val_name; int val_len; struct AstNode *iterable; struct AstNode *body; } for_in;

        struct { AstNode *try_block; const char *catch_name; int catch_len; AstNode *catch_block; } try_stmt;

        struct { struct AstNode *scrutinee; struct AstNode **cases; int case_count; } switch_stmt;
        struct { struct AstNode *label; struct AstNode *body; } switch_case;

        struct { struct AstNode *value; } return_stmt;
        struct { struct AstNode *expr; } expr_stmt;
        struct { struct AstNode *value; } defer_stmt;
        struct { struct AstNode *body; struct AstNode *catch_param; struct AstNode *catch_body; } try_catch;

        struct { struct AstNode **stmts; int stmt_count; } proctime_block;
        struct { const char *path; int path_len; const char *alias; int alias_len; } import;

        struct { struct AstNode **stmts; int stmt_count; } program;
        
        struct { const char *name; int len; struct AstNode *type_expr; struct AstNode *value; } field;

        struct { struct AstNode **items; int count; } array_lit;
        struct { struct AstNode *key; struct AstNode *value; } array_entry;
        struct { struct AstNode **fields; int count; } struct_lit;

        struct { AstNode *type_expr; AstNode *expr; } cast;

        struct { TokenType type_annotation; AstNode **decls; int decl_count; } type_block;

        struct { const char *name; int len; bool has_type_annotation; struct AstNode *type_annotation; struct AstNode *value; } decl;
    } as;
};

struct Obj {
    ValueType type;
    int refcount;
};

struct Value {
    ValueType type;
    union {
        bool b;
        long i;
        double f;
        Obj *obj;
    } as;
};

struct EnvEntry {
    const char *name;
    int len;
    Value value;
};

struct Env {
    EnvEntry *entries;
    int count, capacity;
    Env *parent;
    Value self_binding;
    bool has_self;
};

struct StructField {
    const char *name;
    int len;
    Value value;
};

struct ObjArray {
    Obj obj;
    Value *items;
    int count, capacity;
};

struct ObjStruct {
    Obj obj;
    const char *type_name;
    int type_name_len;
    StructField *fields;
    int count, capacity;
};

struct ObjFn {
    Obj obj;
    AstNode *decl;
    Env *closure;
};