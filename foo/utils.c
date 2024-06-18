


/// This function will be specific to the lexer. It will be globally avaliable,
/// However, it can only accept the correct lexer structure.
void tokenizer_log(const char* token, const char* line) {
    printf("[Lexer] Token: | %s | Line: %s", token, line);
}


/// This function will be specific to the parser. It only accepts an ASTNode
// data structure.
void parser_log(void) {
    // ...
}








