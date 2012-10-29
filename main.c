#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "stringtable.h"
#include "auxlib.h"
#include "toklist.h"

#define CPP "/usr/bin/cpp"
#define MAXLINE 256

extern int yy_flex_debug;
extern int yydebug;
extern FILE * yyin;
extern stringtable_ref table;
extern toklist_ref token_list;

static char * cpp_string = NULL;
static char * prog_name = NULL;

void cpp_popen(char *);
int cpp_pclose(void);
void run_scanner(void);
int yylex(void);

int main(int argc, char**argv) {
    char * filename;
    set_execname(argv[0]);
    
    //Parse options
    int option;
    for(;;) {
        option = getopt(argc, argv, "@:D:ly");
        if(option == EOF) {
            break;
        }
        DEBUGF('o', "Opt %c got!\n\t%s\n", option, optarg);
        switch(option) {
            case 'l': yy_flex_debug = 1; break;
            case 'y': yydebug = 1; break;
            case '@': set_debugflags(optarg); break;
            case 'D': 
                cpp_string = (char *)malloc(strlen(optarg) + 1);
                strncpy(cpp_string, optarg, strlen(optarg));
                break;
            default: 
                errprintf("Bad option (%c)\n", optopt);
                break;
        }
    }
    if(optind >= argc) {
        errprintf("Usage: %s [-ly] [-@flag] [-Dstring] program.oc\n", 
            get_execname());
        exit(get_exitstatus());
    }
    filename = argv[optind];
    prog_name = (char *)malloc(strlen(basename(filename)) + 1);
    strncpy(prog_name, basename(filename), strlen(basename(filename)));

    table = new_stringtable();

    token_list = new_toklist();
    DEBUGSTMT('a', debugdump_stringtable(table, stderr););
    DEBUGF('f', "FILENAME: %s\n", filename);
    cpp_popen(filename);

    DEBUGF('a', "Running scanner");
    fflush(NULL);
    run_scanner();
    DEBUGF('a', "SCANNER DONE");

    if(cpp_pclose() != 0) return get_exitstatus();
    int prog_base_length;
    if(strrchr(prog_name, '.') != NULL)
        prog_base_length = strrchr(prog_name, '.') - prog_name;
    else
        prog_base_length = strlen(prog_name);

    char* str_prog_name = (char *)calloc(prog_base_length + 5, sizeof(char));
    strncpy(str_prog_name, prog_name, prog_base_length);
    strcat(str_prog_name, ".str");

    char *tok_prog_name = (char *)calloc(strlen(str_prog_name) + 1, sizeof(char));
    strncpy(tok_prog_name, prog_name, prog_base_length);
    strcat(tok_prog_name, ".tok");

    DEBUGF('f', "STR FILENAME: %s\n", str_prog_name);
    FILE *dump_str = fopen(str_prog_name, "w");
    FILE *dump_tok = fopen(tok_prog_name, "w");
    debugdump_stringtable(table, dump_str);
    DEBUGSTMT('a',debugdump_stringtable(table, stderr););
    dump_toklist(token_list, dump_tok);
    fclose(dump_str);
    fclose(dump_tok);

    delete_stringtable(table);
    delete_toklist(token_list);

    free(cpp_string);
    free(str_prog_name);
    free(tok_prog_name);
    free(prog_name);
    return get_exitstatus();
}

void cpp_popen(char * filename) {
    int cpp_string_len = cpp_string == NULL ? 0 : strlen(cpp_string);
    char * cpp_command = (char *)malloc(strlen(CPP) + cpp_string_len + strlen(filename) + 5);
    strcpy(cpp_command, CPP);
    if(cpp_string != NULL) {
        strcat(cpp_command, " -D");
        strcat(cpp_command, cpp_string);
    }
    strcat(cpp_command, " ");
    strcat(cpp_command, filename);
    yyin = popen(cpp_command, "r");
    free(cpp_command);
    if(yyin == NULL){
        syserrprintf(cpp_command);
        exit(get_exitstatus());
    }
}

int cpp_pclose(void) {
    int rc = pclose(yyin);
    DEBUGF('c', "CPP RC: %d\n", rc);
    if(rc) {
        errprintf("CPP Error! Code %d\n", rc);
    }
    return rc;
}

void run_scanner(void) {
    while(yylex() != 0);

    // char buf[MAXLINE];
    // char* savepos;
    // char* token;
    // while(memset(buf, 0, MAXLINE), (fgets(buf, MAXLINE, yyin)) != NULL) {
    //     token = strtok_r(buf, "\040\n\t", &savepos);
    //     if(token == NULL || token[0] == '#') continue;
    //     while(token != NULL) {
    //         DEBUGF('t', "TOKEN: %s\n", token);
    //         intern_stringtable(table, token);
    //         token = strtok_r(NULL, "\040\n\t", &savepos);
    //     }
    // }
}
