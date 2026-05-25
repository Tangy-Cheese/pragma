#pragma once

enum class TokenType{
    
    IDENTIFIER,

    //keywords
    FUNC, FOR, PARALLEL, ALLOC, RETURN, IF, ELIF, ELSE, PRINT,

    //numerical operations
    PLUS, MINUS, ASTERISK, SLASH, PERCENT, EQUALS, GREATER, LESS, GREATEREQ, LESSEQ, NOTEQ, AND, OR,  
    
    //literals
    INT_LIT, CHAR_LIT, FLOAT_LIT, STRING_LIT,

    //synatxical symbols
    R_PAREN, L_PAREN, R_CURLY, L_CURLY, COMMA,
};
