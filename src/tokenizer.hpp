#pragma once

enum class TokenType{

    //keywords
    FUNC, FOR, PARALLEL, ALLOC, RETURN, IF, ELIF, ELSE, PRINT,

    //numerical operations
    PLUS, MINUS, ASTERISK, SLASH, PERCENT, EQUALS, GREATER, LESS, GREATEREQ, LESSEQ, NOTEQ, AND, OR,  
    
    //literals
    INT_LIT, CHAR_LIT, FLOAT_LIT, STRING_LIT,
};
