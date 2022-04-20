#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sematic/sema_util.h"
#include "sematic/sema_error.h"
#include "sema.h"

pTable table;

void traverseTree(SyntaxNode *node)
{
    if (node == NULL)
        return;

    if (!strcmp(node->name, "ExtDef")) // 每当遇到ExtDef，说明有新符号
        ExtDef(node);

    for (size_t i = 0; i < node->child_cnt; i++)
    {
        traverseTree(get_syn_child(node, i));
    }
}

void ExtDef(SyntaxNode *node)
{
    if (node == NULL)
        return;
    // ExtDef -> Specifier ExtDecList SEMI
    //         | Specifier SEMI
    //         | Specifier FunDec CompSt
    pSchema specifierType = Specifier(get_syn_child(node, 0));
    char *secondName = get_syn_child(node, 1)->name;

    // ExtDef -> Specifier ExtDecList SEMI
    if (!strcmp(secondName, "ExtDecList"))
    {
        ExtDecList(get_syn_child(node, 1), specifierType);
    }
    // ExtDef -> Specifier FunDec CompSt
    else if (!strcmp(secondName, "FunDec"))
    {
        FunDec(get_syn_child(node, 1), specifierType);
        CompSt(get_syn_child(node, 2), specifierType);
    }
    if (specifierType)
        deleteType(specifierType);
}

void ExtDecList(SyntaxNode *node, pSchema specifier)
{
    if (node == NULL)
        return;
    // ExtDecList -> VarDec
    //             | VarDec COMMA ExtDecList
    SyntaxNode *temp = node;
    while (temp)
    {
        pItem item = VarDec(get_syn_child(temp, 0), specifier);
        if (checkTableItemConflict(table, item))
        {
            char msg[100] = {0};
            sprintf(msg, "Redefined variable \"%s\".", item->field->name);
            pError(REDEF_VAR, temp->lineno, msg);
            deleteItem(item);
        }
        else
        {
            addTableItem(table, item);
        }
        if (get_syn_child(temp, 1) != NULL)
        {
            // temp = temp->next->next->child;
            temp = get_syn_child(get_syn_brother(temp, 2), 0);
        }
        else
        {
            break;
        }
    }
}

pSchema Specifier(SyntaxNode *node)
{
    assert(node != NULL);
    // Specifier -> TYPE
    //            | StructSpecifier

    SyntaxNode *t = get_syn_child(node, 0);
    // Specifier -> TYPE
    if (!strcmp(t->name, "TYPE"))
    {
        if (!strcmp(t->strval, "float"))
        {
            return newType(BASIC, FLOAT_TYPE);
        }
        else
        {
            return newType(BASIC, INT_TYPE);
        }
    }
    // Specifier -> StructSpecifier
    else
    {
        return StructSpecifier(t);
    }
}

pSchema StructSpecifier(SyntaxNode *node)
{
    assert(node != NULL);
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    //                  | STRUCT Tag

    // OptTag -> ID | e
    // Tag -> ID
    pSchema returnType = NULL;
    SyntaxNode *t = get_syn_child(node, 1);
    if (strcmp(t->name, "Tag"))
    {
        pItem structItem =
            newItem(table->stack->curStackDepth,
                    newFieldList("", newType(STRUCTURE, NULL, NULL)));
        if (!strcmp(t->name, "OptTag"))
        {
            setFieldListName(structItem->field, get_syn_child(t, 0)->strval); // t->child->val
            t = get_syn_brother(t, 1);
        }
        else // 未命名结构体
        {
            table->unnamedStructCnt++;
            char structName[20] = {0};
            sprintf(structName, "%d", table->unnamedStructCnt);
            setFieldListName(structItem->field, structName);
        }
        // 进入结构体。注意报错信息会有不同
        if (!strcmp(get_syn_brother(t, 1)->name, "DefList"))
        {
            DefList(get_syn_brother(t, 1), structItem);
        }

        if (checkTableItemConflict(table, structItem))
        {
            char msg[100] = {0};
            sprintf(msg, "Duplicated name \"%s\".", structItem->field->name);
            pError(DUPLICATED_NAME, node->lineno, msg);
            deleteItem(structItem);
        }
        else
        {
            returnType = newType(
                STRUCTURE, newString(structItem->field->name),
                copyFieldList(structItem->field->schema->data.structure.field));

            if (!strcmp(get_syn_child(node, 1)->name, "OptTag"))
            {
                addTableItem(table, structItem);
            }
            else // OptTag -> e
            {
                deleteItem(structItem);
            }
        }
    }

    // StructSpecifier->STRUCT Tag
    else
    {
        pItem structItem = searchTableItem(table, get_syn_child(t, 0)->strval); // t->child->val
        if (structItem == NULL || !isStructDef(structItem))
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined structure \"%s\".", get_syn_child(t, 0)->strval);
            pError(UNDEF_STRUCT, node->lineno, msg);
        }
        else
            returnType = newType(
                STRUCTURE, newString(structItem->field->name),
                copyFieldList(structItem->field->schema->data.structure.field));
    }
    return returnType;
}

pItem VarDec(SyntaxNode *node, pSchema specifier)
{
    assert(node != NULL);
    // VarDec -> ID
    //         | VarDec LB INT RB
    SyntaxNode *id = node;
    // get ID
    while (id->child_cnt > 0)
        id = get_syn_child(id, 0);
    pItem p = newItem(table->stack->curStackDepth, newFieldList(id->strval, NULL));

    // VarDec -> ID
    if (!strcmp(get_syn_child(node, 0)->name, "ID"))
    {
        p->field->schema = copyType(specifier);
    }
    // VarDec -> VarDec LB INT RB
    else
    {
        SyntaxNode *varDec = get_syn_child(node, 0);
        pSchema temp = specifier;
        while (varDec->next)
        {
            p->field->schema = newType(ARRAY, copyType(temp), varDec->next->next->intval);
            temp = p->field->schema;
            varDec = get_syn_child(varDec, 0);
        }
    }
    return p;
}

void FunDec(SyntaxNode *node, pSchema returnType)
{
    if (node == NULL)
        return;
    // FunDec -> ID LP VarList RP
    //         | ID LP RP
    pItem p =
        newItem(table->stack->curStackDepth,
                newFieldList(get_syn_child(node, 0)->strval,
                             newType(FUNCTION, 0, NULL, copyType(returnType))));

    // FunDec -> ID LP VarList RP
    if (!strcmp(get_syn_child(node, 2)->name, "VarList"))
    {
        VarList(get_syn_child(node, 2), p);
    }

    // FunDec -> ID LP RP don't need process
    if (checkTableItemConflict(table, p)) // 检查是否重复定义
    {
        char msg[100] = {0};
        sprintf(msg, "Redefined function \"%s\".", p->field->name);
        pError(REDEF_FUNC, node->lineno, msg);
        deleteItem(p);
        p = NULL;
    }
    else
    {
        addTableItem(table, p);
    }
}

void VarList(SyntaxNode *node, pItem func)
{
    if (node == NULL)
        return;
    // VarList -> ParamDec COMMA VarList
    //          | ParamDec
    addStackDepth(table->stack);
    int argc = 0;
    SyntaxNode *temp = get_syn_child(node, 0);
    pFieldList cur = NULL;

    // VarList -> ParamDec
    pFieldList paramDec = ParamDec(temp);
    func->field->schema->data.function.argv = copyFieldList(paramDec);
    cur = func->field->schema->data.function.argv;
    argc++;

    // VarList -> ParamDec COMMA VarList
    while (temp->next)
    {
        // temp = temp->next->next->child;
        temp = get_syn_child(get_syn_brother(temp, 2), 0);
        paramDec = ParamDec(temp);
        if (paramDec)
        {
            cur->next = copyFieldList(paramDec);
            cur = cur->next;
            argc++;
        }
    }

    func->field->schema->data.function.argc = argc;

    popStack(table->stack);
}

pFieldList ParamDec(SyntaxNode *node)
{
    assert(node != NULL);
    // ParamDec -> Specifier VarDec
    pSchema specifierType = Specifier(get_syn_child(node, 0));
    pItem p = VarDec(get_syn_child(node, 1), specifierType);
    if (specifierType)
        deleteType(specifierType);
    if (checkTableItemConflict(table, p))
    {
        char msg[100] = {0};
        sprintf(msg, "Redefined variable \"%s\".", p->field->name);
        pError(REDEF_VAR, node->lineno, msg);
        deleteItem(p);
        return NULL;
    }
    else
    {
        addTableItem(table, p);
        return p->field;
    }
}

void CompSt(SyntaxNode *node, pSchema returnType)
{
    if (node == NULL)
        return;
    // CompSt -> LC DefList StmtList RC
    addStackDepth(table->stack);
    SyntaxNode *temp = get_syn_child(node, 1);
    if (!strcmp(temp->name, "DefList"))
    {
        DefList(temp, NULL);
        temp = temp->next;
    }
    if (!strcmp(temp->name, "StmtList"))
    {
        StmtList(temp, returnType);
    }

    // 为了中间代码生成
    // clearCurDepthStackList(table);
}

void StmtList(SyntaxNode *node, pSchema returnType)
{
    // assert(node != NULL);
    // StmtList -> Stmt StmtList
    //           | e
    while (node)
    {
        Stmt(get_syn_child(node, 0), returnType);
        node = get_syn_child(node, 1);
    }
}

void Stmt(SyntaxNode *node, pSchema returnType)
{
    if (node == NULL)
        return;
    // Stmt -> Exp SEMI
    //       | CompSt
    //       | RETURN Exp SEMI
    //       | IF LP Exp RP Stmt
    //       | IF LP Exp RP Stmt ELSE Stmt
    //       | WHILE LP Exp RP Stmt

    pSchema expType = NULL;
    // Stmt -> Exp SEMI
    if (!strcmp(get_syn_child(node, 0)->name, "Exp"))
        expType = Exp(get_syn_child(node, 0));

    // Stmt -> CompSt
    else if (!strcmp(get_syn_child(node, 0)->name, "CompSt"))
        CompSt(get_syn_child(node, 0), returnType);

    // Stmt -> RETURN Exp SEMI
    else if (!strcmp(get_syn_child(node, 0)->name, "RETURN"))
    {
        expType = Exp(get_syn_child(node, 1));

        // check return type
        if (!checkType(returnType, expType))
            pError(TYPE_MISMATCH_RETURN, node->lineno, "Type mismatched for return.");
    }

    // Stmt -> IF LP Exp RP Stmt
    else if (!strcmp(get_syn_child(node, 0)->name, "IF"))
    {
        SyntaxNode *stmt = get_syn_child(node, 4);
        expType = Exp(get_syn_child(node, 2));
        Stmt(stmt, returnType);
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        if (stmt->next != NULL)
            Stmt(stmt->next->next, returnType);
    }

    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(get_syn_child(node, 0)->name, "WHILE"))
    {
        expType = Exp(get_syn_child(node, 2));
        Stmt(get_syn_child(node, 4), returnType);
    }

    if (expType)
        deleteType(expType);
}

void DefList(SyntaxNode *node, pItem structInfo)
{
    // assert(node != NULL);
    // DefList -> Def DefList
    //          | e
    while (node)
    {
        Def(get_syn_child(node, 0), structInfo);
        node = get_syn_child(node, 1);
    }
}

void Def(SyntaxNode *node, pItem structInfo)
{
    if (node == NULL) // 空产生式
        return;

    // Def -> Specifier DecList SEMI
    pSchema dectype;
    if (node->child_cnt)
        dectype = Specifier(get_syn_child(node, 0));
    else
        dectype = Specifier(NULL);

    DecList(get_syn_child(node, 1), dectype, structInfo);
    if (dectype)
        deleteType(dectype);
}

void DecList(SyntaxNode *node, pSchema specifier, pItem structInfo)
{
    if (node == NULL)
        return;
    // DecList -> Dec
    //          | Dec COMMA DecList
    SyntaxNode *temp = node;
    while (temp)
    {
        Dec(get_syn_child(temp, 0), specifier, structInfo);
        if (get_syn_child(temp, 1))
            temp = get_syn_child(temp, 2); // temp = temp->child->next->next;
        else
            break;
    }
}

void Dec(SyntaxNode *node, pSchema specifier, pItem structInfo)
{
    if (node == NULL)
        return;
    // Dec -> VarDec
    //      | VarDec ASSIGNOP Exp

    // Dec -> VarDec
    if (get_syn_child(node, 1) == NULL)
    {
        if (structInfo != NULL)
        {
            // 结构体内，将VarDec返回的Item中的filedList
            // Copy判断是否重定义，无错则到结构体链表尾 记得delete掉Item
            pItem decitem = VarDec(get_syn_child(node, 0), specifier);
            pFieldList payload = decitem->field;
            pFieldList structField = structInfo->field->schema->data.structure.field;
            pFieldList last = NULL;
            while (structField != NULL)
            {
                // then we have to check
                if (!strcmp(payload->name, structField->name))
                {
                    //出现重定义，报错
                    char msg[100] = {0};
                    sprintf(msg, "Redefined field \"%s\".", decitem->field->name);
                    pError(REDEF_FEILD, node->lineno, msg);
                    deleteItem(decitem);
                    return;
                }
                else
                {
                    last = structField;
                    structField = structField->next;
                }
            }
            //新建一个fieldlist,删除之前的item
            if (last == NULL)
            {
                // that is good
                structInfo->field->schema->data.structure.field =
                    copyFieldList(decitem->field);
            }
            else
            {
                last->next = copyFieldList(decitem->field);
            }
            addTableItem(table, decitem);
        }
        else
        {
            // 非结构体内，判断返回的item有无冲突，无冲突放入表中，有冲突报错delete
            pItem decitem = VarDec(get_syn_child(node, 0), specifier);
            if (checkTableItemConflict(table, decitem))
            {
                //出现冲突，报错
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".", decitem->field->name);
                pError(REDEF_VAR, node->lineno, msg);
                deleteItem(decitem);
            }
            else
            {
                addTableItem(table, decitem);
            }
        }
    }
    // Dec -> VarDec ASSIGNOP Exp
    else
    {
        if (structInfo != NULL)
        {
            // 结构体内不能赋值，报错
            pError(REDEF_FEILD, node->lineno, "Illegal initialize variable in struct.");
        }
        else
        {
            // 判断赋值类型是否相符
            //如果成功，注册该符号
            pItem decitem = VarDec(get_syn_child(node, 0), specifier);
            pSchema exptype = Exp(get_syn_child(node, 2));
            if (checkTableItemConflict(table, decitem))
            {
                //出现冲突，报错
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".",
                        decitem->field->name);
                pError(REDEF_VAR, node->lineno, msg);
                deleteItem(decitem);
            }
            if (!checkType(decitem->field->schema, exptype))
            {
                //类型不相符
                //报错
                pError(TYPE_MISMATCH_ASSIGN, node->lineno,
                       "Type mismatchedfor assignment.");
                deleteItem(decitem);
            }
            if (decitem->field->schema && decitem->field->schema->kind == ARRAY)
            {
                //报错，对非basic类型赋值
                pError(TYPE_MISMATCH_ASSIGN, node->lineno,
                       "Illegal initialize variable.");
                deleteItem(decitem);
            }
            else
            {
                addTableItem(table, decitem);
            }
            // exp不出意外应该返回一个无用的type，删除
            if (exptype)
                deleteType(exptype);
        }
    }
}

pSchema Exp(SyntaxNode *node)
{
    assert(node != NULL);
    // Exp -> Exp ASSIGNOP Exp
    //      | Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | Exp PLUS Exp
    //      | Exp MINUS Exp
    //      | Exp STAR Exp
    //      | Exp DIV Exp
    //      | LP Exp RP
    //      | MINUS Exp
    //      | NOT Exp
    //      | ID LP Args RP
    //      | ID LP RP
    //      | Exp LB Exp RB
    //      | Exp DOT ID
    //      | ID
    //      | INT
    //      | FLOAT
    SyntaxNode *t = get_syn_child(node, 0);
    // exp will only check if the cal is right
    //  printTable(table);
    //二值运算
    if (!strcmp(t->name, "Exp"))
    {
        // 基本数学运算符
        if (strcmp(t->next->name, "[") && strcmp(t->next->name, "."))
        {
            pSchema p1 = Exp(t);
            pSchema p2 = Exp(t->next->next);
            pSchema returnType = NULL;

            // Exp -> Exp ASSIGNOP Exp
            if (!strcmp(t->next->name, "="))
            {
                //检查左值
                SyntaxNode *tchild = get_syn_child(t, 0);

                if (!strcmp(tchild->name, "FLOAT") || !strcmp(tchild->name, "INT"))
                {
                    //报错，左值
                    pError(LEFT_VAR_ASSIGN, t->lineno,
                           "The left-hand side of an assignment must be "
                           "a variable.");
                }
                else if (!strcmp(tchild->name, "ID") ||
                         !strcmp(tchild->next->name, "[") ||
                         !strcmp(tchild->next->name, "."))
                {
                    if (!checkType(p1, p2))
                    {
                        //报错，类型不匹配
                        pError(TYPE_MISMATCH_ASSIGN, t->lineno,
                               "Type mismatched for assignment.");
                    }
                    else
                        returnType = copyType(p1);
                }
                else
                {
                    //报错，左值
                    pError(LEFT_VAR_ASSIGN, t->lineno,
                           "The left-hand side of an assignment must be "
                           "a variable.");
                }
            }
            // Exp -> Exp AND Exp
            //      | Exp OR Exp
            //      | Exp RELOP Exp
            //      | Exp PLUS Exp
            //      | Exp MINUS Exp
            //      | Exp STAR Exp
            //      | Exp DIV Exp
            else
            {
                if (p1 && p2 && (p1->kind == ARRAY || p2->kind == ARRAY))
                {
                    //报错，数组，结构体运算
                    pError(TYPE_MISMATCH_OP, t->lineno,
                           "Type mismatched for operands.");
                }
                else if (!checkType(p1, p2))
                {
                    //报错，类型不匹配
                    pError(TYPE_MISMATCH_OP, t->lineno,
                           "Type mismatched for operands.");
                }
                else
                {
                    if (p1 && p2)
                    {
                        returnType = copyType(p1);
                    }
                }
            }

            if (p1)
                deleteType(p1);
            if (p2)
                deleteType(p2);
            return returnType;
        }
        else // 数组和结构体访问
        {
            // Exp -> Exp LB Exp RB
            if (!strcmp(t->next->name, "["))
            {
                //数组
                pSchema p1 = Exp(t);
                pSchema p2 = Exp(t->next->next);
                pSchema returnType = NULL;

                if (!p1)
                { // 第一个exp为null，上层报错，这里不用再管
                }
                else if (p1 && p1->kind != ARRAY)
                {
                    //报错，非数组使用[]运算符
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an array.", get_syn_child(t, 0)->strval);
                    pError(NOT_A_ARRAY, t->lineno, msg);
                }
                else if (!p2 || p2->kind != BASIC || p2->data.basic != INT_TYPE)
                {
                    //报错，不用int索引[]
                    char msg[100] = {0};
                    SyntaxNode *target = get_syn_child(get_syn_brother(t, 2), 0);
                    switch (target->node_type)
                    {
                    case ENUM_FLOAT:
                        sprintf(msg, "\"%g\" is not an integer.", target->floatval);
                        break;
                    default:
                        sprintf(msg, "\"%s\" is not an integer.", target->strval);
                        break;
                    }
                    pError(NOT_A_INT, t->lineno, msg);
                }
                else
                {
                    returnType = copyType(p1->data.array.elem);
                }
                if (p1)
                    deleteType(p1);
                if (p2)
                    deleteType(p2);
                return returnType;
            }
            // Exp -> Exp DOT ID
            else
            {
                pSchema p1 = Exp(t);
                pSchema returnType = NULL;
                if (!p1 || p1->kind != STRUCTURE || !p1->data.structure.structName)
                {
                    pError(ILLEGAL_USE_DOT, t->lineno, "Illegal use of \".\".");
                }
                else
                {
                    SyntaxNode *ref_id = t->next->next;
                    pFieldList structfield = p1->data.structure.field;
                    while (structfield != NULL)
                    {
                        if (!strcmp(structfield->name, ref_id->strval))
                        {
                            break;
                        }
                        structfield = structfield->next;
                    }
                    if (structfield == NULL)
                    {
                        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",
                               t->lineno, get_syn_child(node, 2)->strval);
                    }
                    else
                    {
                        returnType = copyType(structfield->schema);
                    }
                }
                if (p1)
                    deleteType(p1);
                return returnType;
            }
        }
    }
    //单目运算符
    // Exp -> MINUS Exp
    //      | NOT Exp
    else if (!strcmp(t->name, "-") || !strcmp(t->name, "!"))
    {
        pSchema p1 = Exp(t->next);
        pSchema returnType = NULL;
        if (!p1 || p1->kind != BASIC)
        {
            //报错，数组，结构体运算
            printf("Error type %d at Line %d: %s.\n", 7, t->lineno,
                   "TYPE_MISMATCH_OP");
        }
        else
        {
            returnType = copyType(p1);
        }
        if (p1)
            deleteType(p1);
        return returnType;
    }
    else if (!strcmp(t->name, "("))
    {
        return Exp(t->next);
    }
    // Exp -> ID LP Args RP
    //		| ID LP RP
    else if (!strcmp(t->name, "ID") && t->next)
    {
        pItem funcInfo = searchTableItem(table, t->strval);

        // function not find
        if (funcInfo == NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined function \"%s\".", t->strval);
            pError(UNDEF_FUNC, node->lineno, msg);
            return NULL;
        }
        else if (funcInfo->field->schema->kind != FUNCTION)
        {
            char msg[100] = {0};
            sprintf(msg, "\"%s\" is not a function.", t->strval);
            pError(NOT_A_FUNC, node->lineno, msg);
            return NULL;
        }
        // Exp -> ID LP Args RP
        else if (!strcmp(t->next->next->name, "Args"))
        {
            Args(t->next->next, funcInfo);
            return copyType(funcInfo->field->schema->data.function.returnType);
        }
        // Exp -> ID LP RP
        else
        {
            if (funcInfo->field->schema->data.function.argc != 0)
            {
                char msg[100] = {0};
                sprintf(msg,
                        "too few arguments to function \"%s\", except %d args.",
                        funcInfo->field->name,
                        funcInfo->field->schema->data.function.argc);
                pError(FUNC_AGRC_MISMATCH, node->lineno, msg);
            }
            return copyType(funcInfo->field->schema->data.function.returnType);
        }
    }
    // Exp -> ID
    else if (!strcmp(t->name, "ID"))
    {
        pItem tp = searchTableItem(table, t->strval);
        if (tp == NULL || isStructDef(tp))
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined variable \"%s\".", t->strval);
            pError(UNDEF_VAR, t->lineno, msg);
            return NULL;
        }
        else
        {
            return copyType(tp->field->schema);
        }
    }
    else
    {
        // Exp -> FLOAT
        if (!strcmp(t->name, "FLOAT"))
        {
            return newType(BASIC, FLOAT_TYPE);
        }
        // Exp -> INT
        else
        {
            return newType(BASIC, INT_TYPE);
        }
    }
}

void func_argc_mismatch_err(char *msg, pItem correct_info, SyntaxNode *err_info)
{
    sprintf(msg, "Function \"%s(", correct_info->field->name);
    for (pFieldList arg = correct_info->field->schema->data.function.argv; arg != NULL; arg = arg->next)
    {
        switch (arg->schema->data.basic)
        {
        case INT_TYPE:
            sprintf(msg + strlen(msg), "int");
            break;
        case FLOAT_TYPE:
            sprintf(msg + strlen(msg), "float");
            break;
        default:
            break;
        }
        if (arg->next != NULL) // 若不是最后一个参数，则用逗号隔开
            sprintf(msg + strlen(msg), ", ");
    }
    sprintf(msg + strlen(msg), ")\" is not applicable for arguments \"(");

    // 输出错误参数
    for (SyntaxNode *arg_node = err_info; arg_node != NULL; arg_node = get_syn_child(arg_node, 2))
    {
        switch (get_syn_child(get_syn_child(arg_node, 0), 0)->node_type)
        {
        case ENUM_INT:
            sprintf(msg + strlen(msg), "int");
            break;
        case ENUM_FLOAT:
            sprintf(msg + strlen(msg), "float");
            break;
        default:
            break;
        }
        if (arg_node->child_cnt == 3) // 若不是最后一个参数，则用逗号隔开
            sprintf(msg + strlen(msg), ", ");
    }
    sprintf(msg + strlen(msg), ")\".");

    pError(FUNC_AGRC_MISMATCH, err_info->lineno, msg);
}

void Args(SyntaxNode *node, pItem funcInfo)
{
    if (node == NULL)
        return;
    // Args -> Exp COMMA Args
    //       | Exp
    SyntaxNode *temp = node;
    pFieldList arg = funcInfo->field->schema->data.function.argv;
    while (temp)
    {
        pSchema realType = NULL;
        if (arg != NULL)
            realType = Exp(get_syn_child(temp, 0));

        if (arg == NULL || !checkType(realType, arg->schema))
        {
            char msg[100] = {0};
            func_argc_mismatch_err(msg, funcInfo, node);
            break;
        }

        if (realType)
            deleteType(realType);

        arg = arg->next;
        if (get_syn_child(temp, 1))
        {
            temp = get_syn_child(temp, 2);
        }
        else
        {
            break;
        }
    }
    if (arg != NULL)
    {
        char msg[100] = {0};
        func_argc_mismatch_err(msg, funcInfo, node);
    }
}
