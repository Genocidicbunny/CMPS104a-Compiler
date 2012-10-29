#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "stringtable.h"
#include "auxlib.h"

#define CPP "/usr/bin/cpp"
#define MAXLINE 256

static int yy_flex_debug = 0, yydebug = 0;
static char * cpp_string = NULL;
static char * prog_name = NULL;
static FILE * cpp_in;
static stringtable_ref table;

void cpp_popen(char *);
void cpp_pclose(void);
void run_scanner(void);

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
    DEBUGSTMT('a', debugdump_stringtable(table, stderr););
    DEBUGF('f', "FILENAME: %s\n", filename);
    cpp_popen(filename);

    DEBUGF('a', "Running scanner");
    fflush(NULL);
    run_scanner();
    DEBUGF('a', "SCANNER DONE");

    cpp_pclose();
    int prog_base_length;
    if(strrchr(prog_name, '.') != NULL)
        prog_base_length = strrchr(prog_name, '.') - prog_name;
    else
        prog_base_length = strlen(prog_name);

    char* str_prog_name = (char *)calloc(prog_base_length + 5, sizeof(char));
    strncpy(str_prog_name, prog_name, prog_base_length);
    strcat(str_prog_name, ".str");

    DEBUGF('f', "STR FILENAME: %s\n", str_prog_name);
    FILE *dump = fopen(str_prog_name, "w");
    debugdump_stringtable(table, dump);
    DEBUGSTMT('a',debugdump_stringtable(table, stderr););
    
    delete_stringtable(table);

    free(cpp_string);
    free(str_prog_name);
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
    cpp_in = popen(cpp_command, "r");
    free(cpp_command);
    if(cpp_in == NULL){
        syserrprintf(cpp_command);
        exit(get_exitstatus());
    }
}

void cpp_pclose(void) {
    int rc = pclose(cpp_in);
    DEBUGF('c', "CPP RC: %d\n", rc);
    if(rc) {
        errprintf("CPP Error! Code %d\n", rc);
    }
}

void run_scanner(void) {
    char buf[MAXLINE];
    char* savepos;
    char* token;
    while(memset(buf, 0, MAXLINE), (fgets(buf, MAXLINE, cpp_in)) != NULL) {
        token = strtok_r(buf, "\040\n\t", &savepos);
        if(token == NULL || token[0] == '#') continue;
        while(token != NULL) {
            DEBUGF('t', "TOKEN: %s\n", token);
            intern_stringtable(table, token);
            token = strtok_r(NULL, "\040\n\t", &savepos);
        }
    }
}
