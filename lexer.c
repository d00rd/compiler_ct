#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>

typedef enum {
    // Keywords
    BREAK, 
    CHAR, 
    DOUBLE, 
    ELSE, 
    FOR, 
    IF, 
    INT, 
    RETURN, 
    STRUCT, 
    VOID, 
    WHILE,
    // Operators
    ASSIGN,      
    EQUAL,       
    PLUS,       
    MINUS,       
    MUL,         
    DIV,         
    MOD,         
    NOT,         
    NOTEQUAL,    
    LT,          
    LE,          
    GT,          
    GE,          
    AND,         
    OR,          
    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    // Literals
    CT_INT,
    CT_CHAR,
    CT_REAL,
    CT_STRING,
    ID,
    END
} TokenCode;

typedef struct _Token {
    TokenCode code;           
    union {
        char *text;     
        long int i;     
        double r;       
    } attr;
    int line;           
    struct _Token *next;
} Token;


Token *tokens = NULL;    
Token *lastToken = NULL; 
int line = 1;           
const char *pCrtCh;      

#define SAFEALLOC(var, Type) \
    if ((var = (Type*)malloc(sizeof(Type))) == NULL) \
        err("not enough memory")

void err(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

Token* addTk(TokenCode code) {
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    
    if (lastToken) {
        lastToken->next = tk;
    } else {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

char* createString(const char *begin, const char *end) {
    int len = end - begin;
    char *str = (char*)malloc(sizeof(char) * len);
    if(!str){
        err("not enough memory");
    }
    memcpy(str, begin, len);
    str[len] = '\0';
    return str;
}

void freeTokens() {
    Token *tk = tokens;
    while (tk) {
        Token *next = tk->next;
        if ((tk->code == ID || tk->code == CT_STRING) && tk->attr.text) {
            free(tk->attr.text);
        }
        free(tk);
        tk = next;
    }
    tokens = NULL;
    lastToken = NULL;
}


int isKeyword(const char *start, const char *end, int len) {
    if (len == 5 && !memcmp(start, "break", 5)) return BREAK;
    if (len == 4 && !memcmp(start, "char", 4)) return CHAR;
    if (len == 6 && !memcmp(start, "double", 6)) return DOUBLE;
    if (len == 4 && !memcmp(start, "else", 4)) return ELSE;
    if (len == 3 && !memcmp(start, "for", 3)) return FOR;
    if (len == 2 && !memcmp(start, "if", 2)) return IF;
    if (len == 3 && !memcmp(start, "int", 3)) return INT;
    if (len == 6 && !memcmp(start, "return", 6)) return RETURN;
    if (len == 6 && !memcmp(start, "struct", 6)) return STRUCT;
    if (len == 4 && !memcmp(start, "void", 4)) return VOID;
    if (len == 5 && !memcmp(start, "while", 5)) return WHILE;
    return ID;
}

int getNextToken() {
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;
    int charValue = 0;
    
    while (1) {  
        ch = *pCrtCh;
        
        switch (state) {
        case 0: // initial state
            if (isalpha(ch) || ch == '_') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 13;
            }
            else if (isdigit(ch)) {
                pStartCh = pCrtCh;
                state = 1;
            }
            else if (ch == '\'') {
                pStartCh = pCrtCh;
                charValue = 0;
                pCrtCh++;
                state = 17;
            }
            else if (ch == '"') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 19;
            }
            else if (ch == '=') {
                pCrtCh++;
                state = 39;
            }
            else if (ch == '!') {
                pCrtCh++;
                state = 38;
            }
            else if (ch == '<') {
                pCrtCh++;
                state = 42;
            }
            else if (ch == '>') {
                pCrtCh++;
                state = 43;
            }
            else if (ch == '&') {
                pCrtCh++;
                state = 34;
            }
            else if (ch == '|') {
                pCrtCh++;
                state = 37;
            }
            else if (ch == '+') {
                pCrtCh++;
                tk = addTk(PLUS);
                return PLUS;
            }
            else if (ch == '-') {
                pCrtCh++;
                tk = addTk(MINUS);
                return MINUS;
            }
            else if (ch == '*') {
                pCrtCh++;
                state = 30;
            }
            else if (ch == '/') {
                pCrtCh++;
                state = 31;
            }
            else if (ch == '%') {
                pCrtCh++;
                tk = addTk(MOD);
                return MOD;
            }
            else if (ch == '(') {
                pCrtCh++;
                tk = addTk(LPAREN);
                return LPAREN;
            }
            else if (ch == ')') {
                pCrtCh++;
                tk = addTk(RPAREN);
                return RPAREN;
            }
            else if (ch == '{') {
                pCrtCh++;
                tk = addTk(LBRACE);
                return LBRACE;
            }
            else if (ch == '}') {
                pCrtCh++;
                tk = addTk(RBRACE);
                return RBRACE;
            }
            else if (ch == '[') {
                pCrtCh++;
                tk = addTk(LBRACKET);
                return LBRACKET;
            }
            else if (ch == ']') {
                pCrtCh++;
                tk = addTk(RBRACKET);
                return RBRACKET;
            }
            else if (ch == ';') {
                pCrtCh++;
                tk = addTk(SEMICOLON);
                return SEMICOLON;
            }
            else if (ch == ',') {
                pCrtCh++;
                tk = addTk(COMMA);
                return COMMA;
            }
            else if (ch == '.') {
                pCrtCh++;
                tk = addTk(DOT);
                return DOT;
            }
            else if (ch == ' ' || ch == '\r' || ch == '\t') {
                pCrtCh++;
            }
            else if (ch == '\n') {
                line++;
                pCrtCh++;
            }
            else if (ch == '\0') {
                addTk(END);
                return END;
            }
            else {
                tkerr(addTk(END), "invalid character: %c", ch);
            }
            break;

        case 1: // number dispatch
            if (ch == '0') {
                pCrtCh++;
                ch = *pCrtCh;
                if (ch == 'x' || ch == 'X') {
                    pCrtCh++;
                    state = 2;
                } else if (isdigit(ch)) {
                    state = 3;
                } else if (ch == '.') {
                    pCrtCh++;
                    state = 11;
                } else {
                    tk = addTk(CT_INT);
                    tk->attr.i = 0;
                    return CT_INT;
                }
            } else if (isdigit(ch)) {
                state = 6;
            }
            break;

        case 2: // hexadecimal
            if (isxdigit(ch)) {
                pCrtCh++;
            } else if (isalnum(ch)) {
                addTk(END);
                err("error in line %d: invalid hexadecimal digit: %c", line, ch);
            } else {
                tk = addTk(CT_INT);
                tk->attr.i = strtol(pStartCh, NULL, 16);
                return CT_INT;
            }
            break;

        case 3: // octal
            if (ch >= '0' && ch <= '7') {
                pCrtCh++;
            } else if (isdigit(ch)) {
                addTk(END);
                err("error in line %d: invalid octal digit: %c", line, ch);
            } else if (ch == '.') {
                pCrtCh++;
                state = 11;
            } else {
                tk = addTk(CT_INT);
                tk->attr.i = strtol(pStartCh, NULL, 8);
                return CT_INT;
            }
            break;

        case 6: // decimal integer
            if (isdigit(ch)) {
                pCrtCh++;
            } else if (ch == '.') {
                pCrtCh++;
                state = 11;
            } else if(ch == 'e' || ch == 'E'){
                pCrtCh ++;
                state = 12;
            }else {
                tk = addTk(CT_INT);
                tk->attr.i = strtol(pStartCh, NULL, 10);
                return CT_INT;
            }
            break;

        case 11: // fractional part
            if (isdigit(ch)) {
                pCrtCh++;
            } else {
                tk = addTk(CT_REAL);
                tk->attr.r = atof(pStartCh);
                return CT_REAL;
            }
            break;
        
        case 12:
            while(isdigit(ch)){
                pCrtCh++;
                ch = *pCrtCh;
            }
            
            tk = addTk(CT_REAL);
            tk->attr.r = atof(pStartCh);
            return CT_REAL;

            break;
        case 13: // identifier/keyword
            if (isalnum(ch) || ch == '_') {
                pCrtCh++;
            } else {
                nCh = pCrtCh - pStartCh;
                int keywordCode = isKeyword(pStartCh, pCrtCh, nCh);
                tk = addTk(keywordCode);
                if (keywordCode == ID) {
                    tk->attr.text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            }
            break;

        case 17: // character constant
            if (ch == '\\') {
                state = 44;
                pCrtCh++;
            } else if (ch == '\'') {
                pCrtCh++;
                tk = addTk(CT_CHAR);
                tk->attr.i = charValue;
                return CT_CHAR;
            } else if (ch == '\n' || ch == 0) {
                tkerr(addTk(END), "unterminated character constant");
            } else {
                charValue = ch;
                pCrtCh++;
            }
            break;

        case 19: // string constant
            if (ch == '\\') {
                state = 45;
                pCrtCh++;
            } else if (ch == '"') {
                pCrtCh++;
                tk = addTk(CT_STRING);
                tk->attr.text = createString(pStartCh + 1, pCrtCh - 1);
                return CT_STRING;
            } else if (ch == '\n' || ch == 0) {
                tkerr(addTk(END), "unterminated string constant");
            } else {
                pCrtCh++;
            }
            break;

        case 30: // mul/comment check
            if (ch == '/') {
                pCrtCh++;
                state = 25;
            } else {
                tk = addTk(MUL);
                return MUL;
            }
            break;

        case 31: // div/comment check
            if (ch == '/') {
                while (*pCrtCh != '\n' && *pCrtCh != 0) {
                    pCrtCh++;
                }
                state = 0;
            } else if (ch == '*') {
                pCrtCh++;
                state = 25;
            } else {
                tk = addTk(DIV);
                return DIV;
            }
            break;

        case 25: // inside block comment
            if (ch == '*') {
                pCrtCh++;
                state = 26;
            } else if (ch == '\n') {
                line++;
                pCrtCh++;
            } else if (ch == 0) {
                tkerr(addTk(END), "unterminated block comment");
            } else {
                pCrtCh++;
            }
            break;

        case 26: // after * in comment
            if (ch == '/') {
                pCrtCh++;
                state = 0;
            } else if (ch == '*') {
                pCrtCh++;
            } else if (ch == '\n') {
                line++;
                pCrtCh++;
                state = 25;
            } else if (ch == '\0') {
                tkerr(addTk(END), "unterminated block comment");
            } else {
                pCrtCh++;
                state = 25;
            }
            break;

        case 34: // & or &&
            if (ch == '&') {
                pCrtCh++;
                tk = addTk(AND);
                return AND;
            } else {
                tkerr(addTk(END), "invalid token: single &");
            }

        case 37: // | or ||
            if (ch == '|') {
                pCrtCh++;
                tk = addTk(OR);
                return OR;
            } else {
                tkerr(addTk(END), "invalid token: single |");
            }

        case 38: // ! or !=
            if (ch == '=') {
                pCrtCh++;
                tk = addTk(NOTEQUAL);
                return NOTEQUAL;
            } else {
                tk = addTk(NOT);
                return NOT;
            }

        case 39: // = or ==
            if (ch == '=') {
                pCrtCh++;
                tk = addTk(EQUAL);
                return EQUAL;
            } else {
                tk = addTk(ASSIGN);
                return ASSIGN;
            }

        case 42: // < or <=
            if (ch == '=') {
                pCrtCh++;
                tk = addTk(LE);
                return LE;
            } else {
                tk = addTk(LT);
                return LT;
            }

        case 43: // > or >=
            if (ch == '=') {
                pCrtCh++;
                tk = addTk(GE);
                return GE;
            } else {
                tk = addTk(GT);
                return GT;
            }

        case 44: // escape char in char constant
            char esc = *pCrtCh;
            if (esc == 'n') charValue = '\n';
            else if (esc == 't') charValue = '\t';
            else if (esc == '0') charValue = '\0';
            else if (esc == '\\') charValue = '\\';
            else if (esc == '\'') charValue = '\'';
            else if (esc == '"') charValue = '"';
            else charValue = esc;
            if (*pCrtCh != 0) pCrtCh++;
            state = 17;
            break;

        case 45: // escape char in string constant
            if (*pCrtCh != 0) pCrtCh++;
            state = 19;
            break;
        }
    }
}


const char* tokenCodeToString(int code) {
    switch (code) {
        case BREAK: return "BREAK";
        case CHAR: return "CHAR";
        case DOUBLE: return "DOUBLE";
        case ELSE: return "ELSE";
        case FOR: return "FOR";
        case IF: return "IF";
        case INT: return "INT";
        case RETURN: return "RETURN";
        case STRUCT: return "STRUCT";
        case VOID: return "VOID";
        case WHILE: return "WHILE";
        case ASSIGN: return "ASSIGN";
        case EQUAL: return "EQUAL";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case NOT: return "NOT";
        case NOTEQUAL: return "NOTEQUAL";
        case LT: return "LT";
        case LE: return "LE";
        case GT: return "GT";
        case GE: return "GE";
        case AND: return "AND";
        case OR: return "OR";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case DOT: return "DOT";
        case CT_INT: return "CT_INT";
        case CT_CHAR: return "CT_CHAR";
        case CT_REAL: return "CT_REAL";
        case CT_STRING: return "CT_STRING";
        case ID: return "ID";
        case END: return "END";
        default: return "UNKNOWN";
    }
}

void showTokens(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        err("cannot open output file: %s", filename);
    }
    
    Token *tk = tokens;
    int current_line = 1;
    while (tk && tk->code != END) {
        while (tk && tk->line == current_line && tk->code != END) {
            fprintf(f, "%s", tokenCodeToString(tk->code));
            switch (tk->code) {
                case ID:
                    fprintf(f, "(%s)", tk->attr.text);
                    break;
                case CT_INT:
                    fprintf(f, "(%ld)", tk->attr.i);
                    break;
                case CT_CHAR:
                    fprintf(f, "(%c)", (char)tk->attr.i);
                    break;
                case CT_REAL:
                    fprintf(f, "(%f)", tk->attr.r);
                    break;
                case CT_STRING:
                    fprintf(f, "(\"%s\")", tk->attr.text);
                    break;
                default:
                    break;
            }
            if (tk->next && tk->next->line == current_line && tk->next->code != END) fprintf(f, " ");
            tk = tk->next;
        }
        fprintf(f, "\n");
        current_line++;
    }
    
    fclose(f);
}

char* readFile(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        err("cannot open file: %s", filename);
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buffer = (char*)malloc(size + 1);
    if (!buffer) {
        err("not enough memory");
    }
    
    if (fread(buffer, 1, size, f) != (size_t)size) {
        err("error reading file: %s", filename);
    }
    fclose(f);
    
    buffer[size] = '\0';
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    
    char *input = readFile(argv[1]);
    pCrtCh = input;
    line = 1;
    
    mkdir("tests-out", 0755);
    
    char *input_name = argv[1];
    char *base = strrchr(input_name, '/');
    if (base) base++; else base = input_name;
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';
    char output_path[256];
    snprintf(output_path, sizeof(output_path), "tests-out/%s.out", base);
    
    int tokenCode;
    do {
        tokenCode = getNextToken();
    } while (tokenCode != END);
    
    showTokens(output_path);
    
    free(input);
    freeTokens();
    
    return 0;
}