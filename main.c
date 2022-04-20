#include "syntax_tree.h"
#include "sema.h"
#include "inter.h"
#include "C--syntax.tab.h"

extern SyntaxNode *root;

extern int yydebug; // Bison debug flag

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE *);

int lex_error_cnt = 0; // 词法错误个数
int syn_error_cnt = 0; // 语法错误个数

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        yyparse();
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        fprintf(stderr, "Failed to open input file: %s\n", argv[1]);
        return 1;
    }

    FILE* fw = fopen(argv[2], "wt+");
    if (!fw) {
        fprintf(stderr, "Failed to open output file: %s\n", argv[2]);
        return 1;
    }

    yyrestart(f);

#ifdef DEBUG
    yydebug = 1;
#endif
    yyparse();

#ifdef DEBUG
    preorder_traversal(stdout, root, 0);
#endif

    if (!lex_error_cnt && !syn_error_cnt)
    {
        table = initTable();
        traverseTree(root);
        
        interCodeList = newInterCodeList();
        genInterCodes(root);
        if (!interError) {
            printInterCode(fw, interCodeList);
        }
        
        deleteTable(table);
    }

    fclose(f);
    f = NULL;
    fclose(fw);
    fw = NULL;
    
    free_syn_tree(root);
    return 0;
}
