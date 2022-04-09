#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sema.h"

pTable table;

static inline char *newString(char *src)
{
    if (src == NULL)
        return NULL;
    int length = strlen(src) + 1;
    char *p = (char *)malloc(sizeof(char) * length);
    assert(p != NULL);
    strncpy(p, src, length);
    return p;
}

// Type functions
pSchema newType(SymbolKind kind, ...)
{
    pSchema p = (pSchema)malloc(sizeof(SymbolSchema));
    assert(p != NULL);
    p->kind = kind;
    va_list vaList;
    assert(kind == BASIC || kind == ARRAY || kind == STRUCTURE || kind == FUNCTION);
    switch (kind)
    {
    case BASIC:
        va_start(vaList, 1);
        p->data.basic = va_arg(vaList, BasicType);
        break;
    case ARRAY:
        va_start(vaList, 2);
        p->data.array.elem = va_arg(vaList, pSchema);
        p->data.array.size = va_arg(vaList, int);
        break;
    case STRUCTURE:
        va_start(vaList, 2);
        p->data.structure.structName = va_arg(vaList, char *);
        p->data.structure.field = va_arg(vaList, pFieldList);
        break;
    case FUNCTION:
        va_start(vaList, 3);
        p->data.function.argc = va_arg(vaList, int);
        p->data.function.argv = va_arg(vaList, pFieldList);
        p->data.function.returnType = va_arg(vaList, pSchema);
        break;
    }
    va_end(vaList);
    return p;
}

pSchema copyType(pSchema src)
{
    if (src == NULL)
        return NULL;
    pSchema p = (pSchema)malloc(sizeof(SymbolSchema));
    assert(p != NULL);
    p->kind = src->kind;
    assert(p->kind == BASIC || p->kind == ARRAY || p->kind == STRUCTURE || p->kind == FUNCTION);
    switch (p->kind)
    {
    case BASIC:
        p->data.basic = src->data.basic;
        break;
    case ARRAY:
        p->data.array.elem = copyType(src->data.array.elem);
        p->data.array.size = src->data.array.size;
        break;
    case STRUCTURE:
        p->data.structure.structName = newString(src->data.structure.structName);
        p->data.structure.field = copyFieldList(src->data.structure.field);
        break;
    case FUNCTION:
        p->data.function.argc = src->data.function.argc;
        p->data.function.argv = copyFieldList(src->data.function.argv);
        p->data.function.returnType = copyType(src->data.function.returnType);
        break;
    }

    return p;
}

void deleteType(pSchema type)
{
    assert(type != NULL);
    assert(type->kind == BASIC || type->kind == ARRAY ||
           type->kind == STRUCTURE || type->kind == FUNCTION);
    pFieldList temp = NULL;
    // pFieldList tDelete = NULL;
    switch (type->kind)
    {
    case BASIC:
        break;
    case ARRAY:
        deleteType(type->data.array.elem);
        type->data.array.elem = NULL;
        break;
    case STRUCTURE:
        if (type->data.structure.structName)
            free(type->data.structure.structName);
        type->data.structure.structName = NULL;

        temp = type->data.structure.field;
        while (temp)
        {
            pFieldList tDelete = temp;
            temp = temp->next;
            deleteFieldList(tDelete);
        }
        type->data.structure.field = NULL;
        break;
    case FUNCTION:
        deleteType(type->data.function.returnType);
        type->data.function.returnType = NULL;
        temp = type->data.function.argv;
        while (temp)
        {
            pFieldList tDelete = temp;
            temp = temp->next;
            deleteFieldList(tDelete);
        }
        type->data.function.argv = NULL;
        break;
    }
    free(type);
}

bool checkType(pSchema type1, pSchema type2)
{
    if (type1 == NULL || type2 == NULL)
        return 1;
    if (type1->kind == FUNCTION || type2->kind == FUNCTION)
        return 0;
    if (type1->kind != type2->kind)
        return 0;
    else
    {
        assert(type1->kind == BASIC || type1->kind == ARRAY ||
               type1->kind == STRUCTURE);
        switch (type1->kind)
        {
        case BASIC:
            return type1->data.basic == type2->data.basic;
        case ARRAY:
            return checkType(type1->data.array.elem, type2->data.array.elem);
        case STRUCTURE:
            return !strcmp(type1->data.structure.structName,
                           type2->data.structure.structName);
        }
    }
}

void printType(pSchema type)
{
    if (type == NULL)
    {
        printf("type is NULL.\n");
    }
    else
    {
        printf("type kind: %d\n", type->kind);
        switch (type->kind)
        {
        case BASIC:
            printf("type basic: %d\n", type->data.basic);
            break;
        case ARRAY:
            printf("array size: %d\n", type->data.array.size);
            printType(type->data.array.elem);
            break;
        case STRUCTURE:
            if (!type->data.structure.structName)
                printf("struct name is NULL\n");
            else
            {
                printf("struct name is %s\n", type->data.structure.structName);
            }
            printFieldList(type->data.structure.field);
            break;
        case FUNCTION:
            printf("function argc is %d\n", type->data.function.argc);
            printf("function args:\n");
            printFieldList(type->data.function.argv);
            printf("function return type:\n");
            printType(type->data.function.returnType);
            break;
        }
    }
}

// FieldList functions
pFieldList newFieldList(char *newName, pSchema newType)
{
    pFieldList p = (pFieldList)malloc(sizeof(FieldList));
    assert(p != NULL);
    p->name = newString(newName);
    p->schema = newType;
    p->next = NULL;
    return p;
}

pFieldList copyFieldList(pFieldList src)
{
    assert(src != NULL);
    pFieldList head = NULL, cur = NULL;
    pFieldList temp = src;

    while (temp)
    {
        if (!head)
        {
            head = newFieldList(temp->name, copyType(temp->schema));
            cur = head;
            temp = temp->next;
        }
        else
        {
            cur->next = newFieldList(temp->name, copyType(temp->schema));
            cur = cur->next;
            temp = temp->next;
        }
    }
    return head;
}

void deleteFieldList(pFieldList fieldList)
{
    assert(fieldList != NULL);
    if (fieldList->name)
    {
        free(fieldList->name);
        fieldList->name = NULL;
    }
    if (fieldList->schema)
        deleteType(fieldList->schema);
    fieldList->schema = NULL;
    free(fieldList);
}

void setFieldListName(pFieldList p, char *newName)
{
    assert(p != NULL && newName != NULL);
    if (p->name != NULL)
    {
        free(p->name);
    }
    // int length = strlen(newName) + 1;
    // p->name = (char*)malloc(sizeof(char) * length);
    // strncpy(p->name, newName, length);
    p->name = newString(newName);
}

void printFieldList(pFieldList fieldList)
{
    if (fieldList == NULL)
        printf("fieldList is NULL\n");
    else
    {
        printf("fieldList name is: %s\n", fieldList->name);
        printf("FieldList Type:\n");
        printType(fieldList->schema);
        printFieldList(fieldList->next);
    }
}

// tableItem functions
pItem newItem(int symbolDepth, pFieldList pfield)
{
    pItem p = (pItem)malloc(sizeof(TableItem));
    assert(p != NULL);
    p->symbolDepth = symbolDepth;
    p->field = pfield;
    p->nextHash = NULL;
    p->nextSymbol = NULL;
    return p;
}

void deleteItem(pItem item)
{
    assert(item != NULL);
    if (item->field != NULL)
        deleteFieldList(item->field);
    free(item);
}

// Hash functions
pHash newHash()
{
    pHash p = (pHash)malloc(sizeof(HashTable));
    assert(p != NULL);
    p->hashArray = (pItem *)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->hashArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->hashArray[i] = NULL;
    }
    return p;
}

void deleteHash(pHash hash)
{
    assert(hash != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        pItem temp = hash->hashArray[i];
        while (temp)
        {
            pItem tdelete = temp;
            temp = temp->nextHash;
            deleteItem(tdelete);
        }
        hash->hashArray[i] = NULL;
    }
    free(hash->hashArray);
    hash->hashArray = NULL;
    free(hash);
}

pItem getHashHead(pHash hash, int index)
{
    assert(hash != NULL);
    return hash->hashArray[index];
}

void setHashHead(pHash hash, int index, pItem newVal)
{
    assert(hash != NULL);
    hash->hashArray[index] = newVal;
}
// Table functions

pTable initTable()
{
    pTable table = (pTable)malloc(sizeof(Table));
    assert(table != NULL);
    table->hash = newHash();
    table->stack = newStack();
    table->unNamedStructNum = 0;
    return table;
};

void deleteTable(pTable table)
{
    deleteHash(table->hash);
    table->hash = NULL;
    deleteStack(table->stack);
    table->stack = NULL;
    free(table);
};

pItem searchTableItem(pTable table, char *name)
{
    unsigned hashCode = getHashCode(name);
    pItem temp = getHashHead(table->hash, hashCode);
    if (temp == NULL)
        return NULL;
    while (temp)
    {
        if (!strcmp(temp->field->name, name))
            return temp;
        temp = temp->nextHash;
    }
    return NULL;
}

// Return false -> no confliction, true -> has confliction
bool checkTableItemConflict(pTable table, pItem item)
{
    pItem temp = searchTableItem(table, item->field->name);
    if (temp == NULL)
        return 0;
    while (temp)
    {
        if (!strcmp(temp->field->name, item->field->name))
        {
            if (temp->field->schema->kind == STRUCTURE ||
                item->field->schema->kind == STRUCTURE)
                return 1;
            if (temp->symbolDepth == table->stack->curStackDepth)
                return 1;
        }
        temp = temp->nextHash;
    }
    return 0;
}

void addTableItem(pTable table, pItem item)
{
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    pHash hash = table->hash;
    pStack stack = table->stack;
    // if (getCurDepthStackHead(stack) == NULL)
    //     setCurDepthStackHead(stack, item);
    // else {
    //     item->nextHash = getCurDepthStackHead(stack);
    //     setCurDepthStackHead(stack, item);
    // }
    item->nextSymbol = getCurDepthStackHead(stack);
    setCurDepthStackHead(stack, item);

    item->nextHash = getHashHead(hash, hashCode);
    setHashHead(hash, hashCode, item);
}

void deleteTableItem(pTable table, pItem item)
{
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    if (item == getHashHead(table->hash, hashCode))
        setHashHead(table->hash, hashCode, item->nextHash);
    else
    {
        pItem cur = getHashHead(table->hash, hashCode);
        pItem last = cur;
        while (cur != item)
        {
            last = cur;
            cur = cur->nextHash;
        }
        last->nextHash = cur->nextHash;
    }
    deleteItem(item);
}

bool isStructDef(pItem src)
{
    if (src == NULL)
        return 0;
    if (src->field->schema->kind != STRUCTURE)
        return 0;
    if (src->field->schema->data.structure.structName)
        return 0;
    return 1;
}

// void addStructLayer(pTable table) { table->enterStructLayer++; }

// void minusStructLayer(pTable table) { table->enterStructLayer--; }

// bool isInStructLayer(pTable table) { return table->enterStructLayer > 0; }

void clearCurDepthStackList(pTable table)
{
    assert(table != NULL);
    pStack stack = table->stack;
    pItem temp = getCurDepthStackHead(stack);
    while (temp)
    {
        pItem tDelete = temp;
        temp = temp->nextSymbol;
        deleteTableItem(table, tDelete);
    }
    setCurDepthStackHead(stack, NULL);
    minusStackDepth(stack);
}

// for Debug
void printTable(pTable table)
{
    printf("----------------hash_table----------------\n");
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        pItem item = getHashHead(table->hash, i);
        if (item)
        {
            printf("[%d]", i);
            while (item)
            {
                printf(" -> name: %s depth: %d\n", item->field->name,
                       item->symbolDepth);
                printf("========FiledList========\n");
                printFieldList(item->field);
                printf("===========End===========\n");
                item = item->nextHash;
            }
            printf("\n");
        }
    }
    printf("-------------------end--------------------\n");
}

// Stack functions
pStack newStack()
{
    pStack p = (pStack)malloc(sizeof(Stack));
    assert(p != NULL);
    p->stackArray = (pItem *)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->stackArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->stackArray[i] = NULL;
    }
    p->curStackDepth = 0;
    return p;
}

void deleteStack(pStack stack)
{
    assert(stack != NULL);
    free(stack->stackArray);
    stack->stackArray = NULL;
    stack->curStackDepth = 0;
    free(stack);
}

void addStackDepth(pStack stack)
{
    assert(stack != NULL);
    stack->curStackDepth++;
}

void minusStackDepth(pStack stack)
{
    assert(stack != NULL);
    stack->curStackDepth--;
}

pItem getCurDepthStackHead(pStack stack)
{
    assert(stack != NULL);
    return stack->stackArray[stack->curStackDepth];
    // return p == NULL ? NULL : p->stackArray[p->curStackDepth];
}

void setCurDepthStackHead(pStack stack, pItem newVal)
{
    assert(stack != NULL);
    stack->stackArray[stack->curStackDepth] = newVal;
}

// Global function
void traverseTree(pNode node)
{
    if (node == NULL)
        return;

    if (!strcmp(node->name, "ExtDef"))
        ExtDef(node);

    for (size_t i = 0; i < node->child_cnt; i++) // TODO Edited
    {
        traverseTree(get_syn_child(node, i));
    }
}

// Generate symbol table functions
void ExtDef(pNode node)
{
    if (node == NULL)
        return;
    // ExtDef -> Specifier ExtDecList SEMI
    //         | Specifier SEMI
    //         | Specifier FunDec CompSt
    pSchema specifierType = Specifier(get_syn_child(node, 0));
    char *secondName = get_syn_child(node, 1)->name;

    // printType(specifierType);
    // ExtDef -> Specifier ExtDecList SEMI
    if (!strcmp(secondName, "ExtDecList"))
    {
        // TODO: process first situation
        ExtDecList(get_syn_child(node, 1), specifierType);
    }
    // ExtDef -> Specifier FunDec CompSt
    else if (!strcmp(secondName, "FunDec"))
    {
        // TODO: process third situation
        FunDec(get_syn_child(node, 1), specifierType);
        CompSt(get_syn_child(node, 2), specifierType);
    }
    if (specifierType)
        deleteType(specifierType);
    // printTable(table);
    // Specifier SEMI
    // this situation has no meaning
    // or is struct define(have been processe inSpecifier())
}

void ExtDecList(pNode node, pSchema specifier)
{
    if (node == NULL)
        return;
    // ExtDecList -> VarDec
    //             | VarDec COMMA ExtDecList
    pNode temp = node;
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

pSchema Specifier(pNode node)
{
    assert(node != NULL);
    // Specifier -> TYPE
    //            | StructSpecifier

    pNode t = get_syn_child(node, 0);
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

pSchema StructSpecifier(pNode node)
{
    assert(node != NULL);
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    //                  | STRUCT Tag

    // OptTag -> ID | e
    // Tag -> ID
    pSchema returnType = NULL;
    pNode t = get_syn_child(node, 1);
    // StructSpecifier->STRUCT OptTag LC DefList RC
    // printTreeInfo(t, 0);
    if (strcmp(t->name, "Tag"))
    {
        // addStructLayer(table);
        pItem structItem =
            newItem(table->stack->curStackDepth,
                    newFieldList("", newType(STRUCTURE, NULL, NULL)));
        if (!strcmp(t->name, "OptTag"))
        {
            setFieldListName(structItem->field, get_syn_child(t, 0)->strval); // t->child->val
            t = t->next;
        }
        // unnamed struct
        else
        {
            table->unNamedStructNum++;
            char structName[20] = {0};
            sprintf(structName, "%d", table->unNamedStructNum);
            // printf("unNamed struct's name is %s.\n", structName);
            setFieldListName(structItem->field, structName);
        }
        //现在我们进入结构体了！注意，报错信息会有不同！
        // addStackDepth(table->stack);
        if (!strcmp(t->next->name, "DefList"))
        {
            DefList(t->next, structItem);
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

            // printf("\nnew Type:\n");
            // printType(returnType);
            // printf("\n");

            if (!strcmp(get_syn_child(node, 1)->name, "OptTag"))
            {
                addTableItem(table, structItem);
            }
            // OptTag -> e
            else
            {
                deleteItem(structItem);
            }
        }

        //我们出了结构体
        // minusStackDepth(table->stack);
        // minusStructLayer(table);
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
    // printType(returnType);
    return returnType;
}

pItem VarDec(pNode node, pSchema specifier)
{
    assert(node != NULL);
    // VarDec -> ID
    //         | VarDec LB INT RB
    pNode id = node;
    // get ID
    while (id->child_cnt > 0)
        id = get_syn_child(id, 0);
    pItem p = newItem(table->stack->curStackDepth, newFieldList(id->strval, NULL));
    // return newItem(table->stack->curStackDepth,
    //                newFieldList(id->val, generateVarDecType(node,
    //                specifier)));

    // VarDec -> ID
    // printTreeInfo(node, 0);
    if (!strcmp(get_syn_child(node, 0)->name, "ID"))
    {
        // printf("copy type tp %s.\n", node->child->val);
        p->field->schema = copyType(specifier);
    }
    // VarDec -> VarDec LB INT RB
    else
    {
        pNode varDec = get_syn_child(node, 0);
        pSchema temp = specifier;
        // printf("VarDec -> VarDec LB INT RB.\n");
        while (varDec->next)
        {
            // printTreeInfo(varDec, 0);
            // printf("number: %s\n", varDec->next->next->val);
            // printf("temp type: %d\n", temp->kind);
            p->field->schema = newType(ARRAY, copyType(temp), varDec->next->next->intval);
            // printf("newType. newType: elem type: %d, elem size: %d.\n",
            //        p->field->type->u.array.elem->kind,
            //        p->field->type->u.array.size);
            temp = p->field->schema;
            varDec = get_syn_child(varDec, 0);
        }
    }
    // printf("-------test VarDec ------\n");
    // printType(specifier);
    // printFieldList(p->field);
    // printf("-------test End ------\n");
    return p;
}

// pType generateVarDecType(pNode node, pType type) {
//     // VarDec -> ID
//     if (!strcmp(node->child->name, "ID")) return copyType(type);
//     // VarDec -> VarDec LB INT RB
//     else
//         return newType(ARRAY, atoi(get_syn_child(node, 2)->val),
//                        generateVarDecType(node, type));
// }

void FunDec(pNode node, pSchema returnType)
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

    // check redefine
    if (checkTableItemConflict(table, p))
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

void VarList(pNode node, pItem func)
{
    if (node == NULL)
        return;
    // VarList -> ParamDec COMMA VarList
    //          | ParamDec
    addStackDepth(table->stack);
    int argc = 0;
    pNode temp = get_syn_child(node, 0);
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

    minusStackDepth(table->stack);
}

pFieldList ParamDec(pNode node)
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

void CompSt(pNode node, pSchema returnType)
{
    if (node == NULL)
        return;
    // CompSt -> LC DefList StmtList RC
    // printTreeInfo(node, 0);
    addStackDepth(table->stack);
    pNode temp = get_syn_child(node, 1);
    if (!strcmp(temp->name, "DefList"))
    {
        DefList(temp, NULL);
        temp = temp->next;
    }
    if (!strcmp(temp->name, "StmtList"))
    {
        StmtList(temp, returnType);
    }

    clearCurDepthStackList(table);
}

void StmtList(pNode node, pSchema returnType)
{
    // assert(node != NULL);
    // StmtList -> Stmt StmtList
    //           | e
    // printTreeInfo(node, 0);
    while (node)
    {
        Stmt(get_syn_child(node, 0), returnType);
        node = get_syn_child(node, 1);
    }
}

void Stmt(pNode node, pSchema returnType)
{
    if (node == NULL)
        return;
    // Stmt -> Exp SEMI
    //       | CompSt
    //       | RETURN Exp SEMI
    //       | IF LP Exp RP Stmt
    //       | IF LP Exp RP Stmt ELSE Stmt
    //       | WHILE LP Exp RP Stmt
    // printTreeInfo(node, 0);

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
            pError(TYPE_MISMATCH_RETURN, node->lineno,
                   "Type mismatched for return.");
    }

    // Stmt -> IF LP Exp RP Stmt
    else if (!strcmp(get_syn_child(node, 0)->name, "IF"))
    {
        pNode stmt = get_syn_child(node, 4);
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

void DefList(pNode node, pItem structInfo)
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

void Def(pNode node, pItem structInfo)
{
    if (node == NULL) // 空产生式
        return;

    // Def -> Specifier DecList SEMI
    // TODO:调用接口
    pSchema dectype;
    if (node->child_cnt)
        dectype = Specifier(get_syn_child(node, 0));
    else
        dectype = Specifier(NULL);
    //你总会得到一个正确的type
    DecList(get_syn_child(node, 1), dectype, structInfo);
    if (dectype)
        deleteType(dectype);
}

void DecList(pNode node, pSchema specifier, pItem structInfo)
{
    if (node == NULL)
        return;
    // DecList -> Dec
    //          | Dec COMMA DecList
    pNode temp = node;
    while (temp)
    {
        Dec(get_syn_child(temp, 0), specifier, structInfo);
        if (get_syn_child(temp, 1))
            temp = get_syn_child(temp, 2); // temp = temp->child->next->next;
        else
            break;
    }
}

void Dec(pNode node, pSchema specifier, pItem structInfo)
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
                    sprintf(msg, "Redefined field \"%s\".",
                            decitem->field->name);
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
            deleteItem(decitem);
        }
        else
        {
            // 非结构体内，判断返回的item有无冲突，无冲突放入表中，有冲突报错delete
            pItem decitem = VarDec(get_syn_child(node, 0), specifier);
            if (checkTableItemConflict(table, decitem))
            {
                //出现冲突，报错
                char msg[100] = {0};
                sprintf(msg, "Redefined variable \"%s\".",
                        decitem->field->name);
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
            pError(REDEF_FEILD, node->lineno,
                   "Illegal initialize variable in struct.");
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

pSchema Exp(pNode node)
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
    pNode t = get_syn_child(node, 0);
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
                pNode tchild = get_syn_child(t, 0);

                if (!strcmp(tchild->name, "FLOAT") ||
                    !strcmp(tchild->name, "INT"))
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
        // 数组和结构体访问
        else
        {
            // Exp -> Exp LB Exp RB
            if (!strcmp(t->next->name, "["))
            {
                //数组
                pSchema p1 = Exp(t);
                pSchema p2 = Exp(t->next->next);
                pSchema returnType = NULL;

                if (!p1)
                {
                    // 第一个exp为null，上层报错，这里不用再管
                }
                else if (p1 && p1->kind != ARRAY)
                {
                    //报错，非数组使用[]运算符
                    char msg[100] = {0};
                    sprintf(msg, "\"%s\" is not an array.", get_syn_child(t, 0)->strval);
                    pError(NOT_A_ARRAY, t->lineno, msg);
                }
                else if (!p2 || p2->kind != BASIC ||
                         p2->data.basic != INT_TYPE)
                {
                    //报错，不用int索引[]
                    char msg[100] = {0};
                    pNode target = get_syn_child(get_syn_brother(t, 2), 0);
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
                if (!p1 || p1->kind != STRUCTURE ||
                    !p1->data.structure.structName)
                {
                    //报错，对非结构体使用.运算符
                    pError(ILLEGAL_USE_DOT, t->lineno, "Illegal use of \".\".");
                }
                else
                {
                    pNode ref_id = t->next->next;
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
                        //报错，没有可以匹配的域名
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
            // good
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

void Args(pNode node, pItem funcInfo)
{
    if (node == NULL)
        return;
    // Args -> Exp COMMA Args
    //       | Exp
    // printTreeInfo(node, 0);
    pNode temp = node;
    pFieldList arg = funcInfo->field->schema->data.function.argv;
    // printf("-----function atgs-------\n");
    // printFieldList(arg);
    // printf("---------end-------------\n");
    while (temp)
    {
        if (arg == NULL)
        {
            char msg[100] = {0};

            // 输出正确定义
            sprintf(msg, "Function \"%s(", funcInfo->field->name);
            for (pFieldList arg = funcInfo->field->schema->data.function.argv; arg != NULL; arg = arg->next)
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
            for (pNode arg_node = node; arg_node != NULL; arg_node = get_syn_child(arg_node, 2))
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

            pError(FUNC_AGRC_MISMATCH, node->lineno, msg);
            break;
        }
        pSchema realType = Exp(get_syn_child(temp, 0));
        // printf("=======arg type=========\n");
        // printType(realType);
        // printf("===========end==========\n");
        if (!checkType(realType, arg->schema))
        {
            char msg[100] = {0};
            sprintf(msg, "Function \"%s\" is not applicable for arguments.",
                    funcInfo->field->name);
            pError(FUNC_AGRC_MISMATCH, node->lineno, msg);
            if (realType)
                deleteType(realType);
            return;
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
        sprintf(msg, "too few arguments to function \"%s\", except %d args.",
                funcInfo->field->name, funcInfo->field->schema->data.function.argc);
        pError(FUNC_AGRC_MISMATCH, node->lineno, msg);
    }
}
