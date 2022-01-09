#include <stdio.h>
#include <string.h>
#include "common.h"
#include "scanner.h"
typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

static char advance(){
    scanner.current++;
    return scanner.current[-1];
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    //printf("line: %d start: %s length: %d\n", token.line, token.start, token.length);
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static bool isAlpha(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='z')||c=='_';}
static char peek(){return*scanner.current;}
static bool isAtEnd(){return*scanner.current=='\0';}
static char peekNext(){return isAtEnd()?'\0':scanner.current[1];}
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type){return scanner.current-scanner.start==start+length&&memcmp(scanner.start + start, rest, length)==0?type:TOKEN_IDENTIFIER;}
static bool isDigit(char c){return c>='0'&&c<='9';}
static bool isHexy(char c){return c>='a'&&c<='f';}

static void skipWhiteToken(){
    for (;;) {
        char c = peek();
        switch (c){
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/'){
                    // A comment goes until the end of the line
                    while (peek() != '\n'&& !isAtEnd()) {
                        advance();
                    }
                    advance();
                } else {return;}
            default:return;
        }
    }
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static Token string(bool interpolate, char quoteType) {
    while (peek()!=quoteType&&!isAtEnd()){
        if (peek()=='\n')scanner.line++;
        advance();
        if (interpolate&&peek()=='{'){advance();return makeToken(TOKEN_STRING);}
    }

    if (isAtEnd())return errorToken("Unterminated string");
    
    // the closing quote
    advance();
    //printf("start: %scurr: %s\n", scanner.start, scanner.current);
    if (!interpolate)return makeToken(TOKEN_STRING);
    //return makeInterToken();
}

static TokenType Interpolation(){
    // f"hello {1} hi"
    // u are rn at '"'

    // Start of the string
    char quote = peek();
    advance();

    Token token = string(true, quote);
    if (&token!=NULL&&token.type==TOKEN_ERROR){
        scanner.start = token.start;
        return TOKEN_ERROR;
    };
    return TOKEN_INTERSTRING;
}

static Token number(bool val){
    //printf("number: %s", scanner.start);
    if (val){
        advance(); // Skip the 'x' in '0x'
        while (isDigit(peek())||isHexy(peek()))advance();
        if (peek()=='.'&&(isDigit(peekNext())||isHexy(peekNext()))){
            advance();
            while (isDigit(peek())||isHexy(peek()))advance(); 
        }
    }else {
        while (isDigit(peek())) advance();
        // look for decimals or fractions, etc
        if (peek()=='.'&&isDigit(peekNext())){
            //rm '.'
            advance();
            while(isDigit(peek()))advance();
        }
    }
    return makeToken(TOKEN_NUMBER);
}

static TokenType identifierType(){
    switch (scanner.start[0]) {
        // Straight keywords
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'b': return checkKeyword(1, 4, "reak", TOKEN_BREAK);
        case 'd': return checkKeyword(1, 2, "ef", TOKEN_FUNC);
        case 'g': return checkKeyword(1, 5, "lobal", TOKEN_GVAR);
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        /*case 'p': return checkKeyword(1, 2, "op", TOKEN_POP);*/
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "tack", TOKEN_SHOW_STACK);
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);

        // Multiple keywords
        case 'c':
            if (scanner.current - scanner.start>1){
                switch (scanner.start[1]){
                    case 'o':return checkKeyword(2,6,"ntinue", TOKEN_CONTINUE);
                    case 'l':return checkKeyword(2,3,"ass", TOKEN_CLASS);
                }
            }
        case 'e':
            if (scanner.current - scanner.start>1){
                switch (scanner.start[1]){
                    case 'n':return checkKeyword(2,1,"d", TOKEN_END);
                    case 'l':return checkKeyword(2,2,"se", TOKEN_ELSE);
                }
            }
        case 'f':
            if (scanner.current - scanner.start>1){
                switch (scanner.start[1]){
                    case 'a':return checkKeyword(2,3,"lse", TOKEN_FALSE);
                    case 'o':return checkKeyword(2,1,"r", TOKEN_FOR);
                }
            }
            //if(scanner.start[1] == '"'){return Interpolation();}
            break;

        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                case 'h': 
                    if (scanner.current - scanner.start > 1) {
                        switch (scanner.start[2])
                        {
                        case 'i': return checkKeyword(3,1,"s", TOKEN_THIS);
                        case 'e': return checkKeyword(3,1,"n", TOKEN_THEN);

                        }
                    }
                case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(){
    while(isAlpha(peek())||isDigit(peek()))advance();
    TokenType type = identifierType();
    return type==TOKEN_ERROR?errorToken(scanner.start):makeToken(type);
}

/*

String Interpolation

Sample -> f"1 + 2 = {1+2}"

if we see an 'f' before '"' then we will just continue
making a normal string token. We will then probably come
across a '{'. This is when we stop the current string, 
we then continue making normal tokens to add the command.
When we see '}' then we stop making commands, and continue
making a normal string.

During parsing, the entire command recorded will be
treated as a ||-> FUNCTION <-||

this will return a result which will be added
as a string into the previous strings

*/

Token scanToken(){
    skipWhiteToken();
    scanner.start = scanner.current;
    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (c == '0' && peek()=='x')return number(true);
    if (isAlpha(c))return identifier();
    if (isDigit(c))return number(false);

    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case ':': return makeToken(match('>') ? TOKEN_BSTART : TOKEN_COLON);
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string(false, '"');
        case '\'': return string(false, '\'');
    }

    return errorToken("Unexpected character");
}

void initScanner(const char* source){
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}
