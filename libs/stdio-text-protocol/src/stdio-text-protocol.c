#include "stdio-text-protocol/stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <stdio.h>





#define STDIO_TEXT_PROTOCOL_BS   0x08
#define STDIO_TEXT_PROTOCOL_DEL  0x7f
#define STDIO_TEXT_PROTOCOL_CR   0x0d
#define STDIO_TEXT_PROTOCOL_LF   0x0a


static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];  
static uint32_t line_length = 0;                     
static bool line_truncated = false;               
static command_t current_command;     



void stdio_text_protocol_init(void)
{   
    line_length    = 0;
    line_truncated = false;
    
    stdio_init_all();
}


const command_t *stdio_text_protocol_handle(void)
{
    int c = getchar_timeout_us(0);       
    if (c == PICO_ERROR_TIMEOUT) {
        return NULL;                     
    }

    if (c == STDIO_TEXT_PROTOCOL_CR || c == STDIO_TEXT_PROTOCOL_LF) {
        line[line_length] = '\0';       

        
        if (line_length == 0) {
            line_truncated = false;
            return NULL;
        }

        putchar(STDIO_TEXT_PROTOCOL_CR);   
        putchar(STDIO_TEXT_PROTOCOL_LF);     
    
        current_command.name      = NULL;
        current_command.argc      = 0;
        current_command.truncated = line_truncated;

        char *p = line;
        while (*p) {
            while (*p == ' ' || *p == '\t') p++;   
            if (*p == '\0') break;

            char *token = p;                       
            while (*p && *p != ' ' && *p != '\t') p++;  
            if (*p) *p++ = '\0';                   

            if (current_command.name == NULL) {
                current_command.name = token;      
            } else if (current_command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                current_command.argv[current_command.argc++] = token;
            } else {
                current_command.truncated = true;  // 
            }
        }

        line_length = 0;
        line_truncated = false;

        if (current_command.name == NULL) {
            return NULL;                     
        }
        return &current_command;            
    }

    if (c == STDIO_TEXT_PROTOCOL_BS || c == STDIO_TEXT_PROTOCOL_DEL) {
        if (line_length > 0) {
            line_length--;
            line_truncated = false;          
            putchar(STDIO_TEXT_PROTOCOL_BS);
            putchar(' ');
            putchar(STDIO_TEXT_PROTOCOL_BS);
        }
        return NULL;                         
    }

    if (c < 0x20 || c > 0x7e) {
        return NULL;
    }

    if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE) {
        line[line_length++] = (char)c;
        putchar(c);
    } else {
        line_truncated = true;   // помечаем переполнение
    }
    return NULL;
}