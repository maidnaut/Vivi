namespace fs = std::filesystem;

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ static_cast<unsigned int>(s[off]);
}

void Vivi::processFlags(int argc, char* argv[], std::string& outScriptPath) {

    printf("\n");

    outScriptPath.clear();
    bool hasScript = false;
    bool arg1IsPath = false;
    if (argc > 1 && argv[1][0] != '-') {
        outScriptPath = argv[1];
        hasScript = true;
        arg1IsPath = true;
    }

    if (argc == 1) {
        printf("Vivi Usage: vivi <path> --args\n\n");
    }

    if (hasScript) {
        fs::path p(outScriptPath);
        if (!fs::exists(p)) {
            printf("Error: path '%s' does not exist.\n\n", outScriptPath.c_str());
            hasScript = false;
            outScriptPath.clear();
        } else if (fs::is_directory(p)) {
            fs::path mainPath = p / "main.vivi";
            if (fs::exists(mainPath) && fs::is_regular_file(mainPath)) {
                outScriptPath = mainPath.string();
            } else {
                bool found = false;
                for (const auto& entry : fs::directory_iterator(p)) {
                    if (fs::is_regular_file(entry.status()) && entry.path().extension() == ".vivi") {
                        outScriptPath = entry.path().string();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    printf("Error: no .vivi file found in directory '%s'.\n\n", p.string().c_str());
                    hasScript = false;
                    outScriptPath.clear();
                }
            }
        } else if (fs::is_regular_file(p)) {
            if (p.extension() != ".vivi") {
                printf("Error: '%s' is not a .vivi file.\n\n", outScriptPath.c_str());
                hasScript = false;
                outScriptPath.clear();
            }
        } else {
            printf("Error: '%s' is not a valid file or directory.\n\n", outScriptPath.c_str());
            hasScript = false;
            outScriptPath.clear();
        }
    }

    std::string source;
    if (!outScriptPath.empty()) {
        std::ifstream file(outScriptPath);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            source = buffer.str();
        }
    }

    int startIdx = arg1IsPath ? 2 : 1;
    for (int i = startIdx; i < argc; ++i) {
        std::string arg(argv[i]);

        auto eqPos = arg.find('=');
        std::string key = (eqPos != std::string::npos) ? arg.substr(0, eqPos) : arg;
        std::string value = (eqPos != std::string::npos) ? arg.substr(eqPos + 1) : "";

        switch (hash(key.c_str())) {
            case hash("--version"):
                printf("Vivi is v%s\n\n", vivi.version);
                break;

            case hash("--lexer"): {
                if (!source.empty()) {
                    printf("Lexer output:\n\n");
                    print_lexer(source);
                } else {
                    printf("Error: no source to lex.\n\n");
                }
                break;
            }

            case hash("--ast"): {
                if (!source.empty()) {
                    Lexer parse_lx = { source.c_str(), source.c_str(), 1, 1 };
                    Parser p = {};
                    p.lx = &parse_lx;
                    p.had_error = false;
                    p.lookahead_count = 0;
                    parser_advance(&p);

                    AstNode *program = parse_program(&p);
                    if (p.had_error) {
                        printf("\nParsing finished with errors.\n\n");
                    } else {
                        printf("AST output:\n\n");
                        print_ast(program, 0);
                        printf("\n");
                    }
                } else {
                    printf("Error: no source to parse.\n\n");
                }
                break;
            }

            default:
                printf("Unknown flag %s, ignoring.\n\n", argv[i]);
                break;
        }
    }
}

void Vivi::sourceExec(const std::string& source) {
    
    Lexer parse_lx = { source.c_str(), source.c_str(), 1, 1 };
    Parser p = {};
    p.lx = &parse_lx;
    p.had_error = false;
    p.lookahead_count = 0;
    parser_advance(&p);

    AstNode *program = parse_program(&p);
    (void)program;

    if (p.had_error) {
        printf("Parsing finished with errors.\n\n");
    }
}

void Vivi::run(int argc, char* argv[]) {
    std::string scriptPath;
    vivi.processFlags(argc, argv, scriptPath);

    if (!scriptPath.empty()) {
        std::ifstream file(scriptPath);
        if (!file) {
            printf("Error: could not open '%s'", scriptPath.c_str());
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        sourceExec(buffer.str());
    }
}