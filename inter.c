#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "sematic/sema_util.h"
#include "inter.h"

bool interError = 0;
pInterCodeList interCodeList;

// Operand func
pOperand newOperand(int kind, ...)
{
    pOperand p = (pOperand)malloc(sizeof(Operand));
    assert(p != NULL);
    p->kind = kind;
    va_list vaList;
    assert(kind >= 0 && kind < 6);
    va_start(vaList, kind);
    switch (kind)
    {
    case OP_CONSTANT:
        p->u.value = va_arg(vaList, int);
        break;
    case OP_VARIABLE:
    case OP_ADDRESS:
    case OP_LABEL:
    case OP_FUNCTION:
    case OP_RELOP:
        p->u.name = va_arg(vaList, char *);
        break;
    }

    return p;
}

void setOperand(pOperand p, int kind, void *val)
{
    assert(p != NULL);
    assert(kind >= 0 && kind < 6);
    p->kind = kind;
    switch (kind)
    {
    case OP_CONSTANT:
        p->u.value = (int)val;
        break;
    case OP_VARIABLE:
    case OP_ADDRESS:
    case OP_LABEL:
    case OP_FUNCTION:
    case OP_RELOP:
        if (p->u.name)
            free(p->u.name);
        p->u.name = (char *)val;
        break;
    }
}

void deleteOperand(pOperand p)
{
    if (p == NULL)
        return;
    assert(p->kind >= 0 && p->kind < 6);
    switch (p->kind)
    {
    case OP_CONSTANT:
        break;
    case OP_VARIABLE:
    case OP_ADDRESS:
    case OP_LABEL:
    case OP_FUNCTION:
    case OP_RELOP:
        if (p->u.name)
        {
            free(p->u.name);
            p->u.name = NULL;
        }
        break;
    }
    free(p);
}

void printOp(FILE *fp, pOperand op)
{
    assert(op != NULL);
    if (fp == NULL)
    {
        switch (op->kind)
        {
        case OP_CONSTANT:
            printf("#%d", op->u.value);
            break;
        case OP_VARIABLE:
        case OP_ADDRESS:
        case OP_LABEL:
        case OP_FUNCTION:
        case OP_RELOP:
            printf("%s", op->u.name);
            break;
        }
    }
    else
    {
        switch (op->kind)
        {
        case OP_CONSTANT:
            fprintf(fp, "#%d", op->u.value);
            break;
        case OP_VARIABLE:
        case OP_ADDRESS:
        case OP_LABEL:
        case OP_FUNCTION:
        case OP_RELOP:
            fprintf(fp, "%s", op->u.name);
            break;
        }
    }
}

// InterCode func
pInterCode newInterCode(int kind, ...)
{
    pInterCode p = (pInterCode)malloc(sizeof(InterCode));
    assert(p != NULL);
    p->kind = kind;
    va_list vaList;
    assert(kind >= 0 && kind < 19);

    va_start(vaList, kind);
    switch (kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        p->u.oneOp.op = va_arg(vaList, pOperand);
        break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
    case IR_CALL:
        p->u.assign.left = va_arg(vaList, pOperand);
        p->u.assign.right = va_arg(vaList, pOperand);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        p->u.binOp.result = va_arg(vaList, pOperand);
        p->u.binOp.op1 = va_arg(vaList, pOperand);
        p->u.binOp.op2 = va_arg(vaList, pOperand);
        break;
    case IR_DEC:
        p->u.dec.op = va_arg(vaList, pOperand);
        p->u.dec.size = va_arg(vaList, int);
        break;
    case IR_IF_GOTO:
        p->u.ifGoto.x = va_arg(vaList, pOperand);
        p->u.ifGoto.relop = va_arg(vaList, pOperand);
        p->u.ifGoto.y = va_arg(vaList, pOperand);
        p->u.ifGoto.z = va_arg(vaList, pOperand);
    }
    return p;
}

void deleteInterCode(pInterCode p)
{
    assert(p != NULL);
    assert(p->kind >= 0 && p->kind < 19);
    switch (p->kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        deleteOperand(p->u.oneOp.op);
        break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
    case IR_CALL:
        deleteOperand(p->u.assign.left);
        deleteOperand(p->u.assign.right);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        deleteOperand(p->u.binOp.result);
        deleteOperand(p->u.binOp.op1);
        deleteOperand(p->u.binOp.op2);
        break;
    case IR_DEC:
        deleteOperand(p->u.dec.op);
        break;
    case IR_IF_GOTO:
        deleteOperand(p->u.ifGoto.x);
        deleteOperand(p->u.ifGoto.relop);
        deleteOperand(p->u.ifGoto.y);
        deleteOperand(p->u.ifGoto.z);
    }
    free(p);
}

void printInterCode(FILE *fp, pInterCodeList interCodeList)
{
    for (pInterCodes cur = interCodeList->head; cur != NULL; cur = cur->next)
    {
        assert(cur->code->kind >= 0 && cur->code->kind < 19);
        if (fp == NULL)
        {
            switch (cur->code->kind)
            {
            case IR_LABEL:
                printf("LABEL ");
                printOp(fp, cur->code->u.oneOp.op);
                printf(" :");
                break;
            case IR_FUNCTION:
                printf("FUNCTION ");
                printOp(fp, cur->code->u.oneOp.op);
                printf(" :");
                break;
            case IR_ASSIGN:
                printOp(fp, cur->code->u.assign.left);
                printf(" := ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_ADD:
                printOp(fp, cur->code->u.binOp.result);
                printf(" := ");
                printOp(fp, cur->code->u.binOp.op1);
                printf(" + ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_SUB:
                printOp(fp, cur->code->u.binOp.result);
                printf(" := ");
                printOp(fp, cur->code->u.binOp.op1);
                printf(" - ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_MUL:
                printOp(fp, cur->code->u.binOp.result);
                printf(" := ");
                printOp(fp, cur->code->u.binOp.op1);
                printf(" * ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_DIV:
                printOp(fp, cur->code->u.binOp.result);
                printf(" := ");
                printOp(fp, cur->code->u.binOp.op1);
                printf(" / ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_GET_ADDR:
                printOp(fp, cur->code->u.assign.left);
                printf(" := &");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_READ_ADDR:
                printOp(fp, cur->code->u.assign.left);
                printf(" := *");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_WRITE_ADDR:
                printf("*");
                printOp(fp, cur->code->u.assign.left);
                printf(" := ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_GOTO:
                printf("GOTO ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_IF_GOTO:
                printf("IF ");
                printOp(fp, cur->code->u.ifGoto.x);
                printf(" ");
                printOp(fp, cur->code->u.ifGoto.relop);
                printf(" ");
                printOp(fp, cur->code->u.ifGoto.y);
                printf(" GOTO ");
                printOp(fp, cur->code->u.ifGoto.z);
                break;
            case IR_RETURN:
                printf("RETURN ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_DEC:
                printf("DEC ");
                printOp(fp, cur->code->u.dec.op);
                printf(" ");
                printf("%d", cur->code->u.dec.size);
                break;
            case IR_ARG:
                printf("ARG ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_CALL:
                printOp(fp, cur->code->u.assign.left);
                printf(" := CALL ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_PARAM:
                printf("PARAM ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_READ:
                printf("READ ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_WRITE:
                printf("WRITE ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            }
            printf("\n");
        }
        else
        {
            switch (cur->code->kind)
            {
            case IR_LABEL:
                fprintf(fp, "LABEL ");
                printOp(fp, cur->code->u.oneOp.op);
                fprintf(fp, " :");
                break;
            case IR_FUNCTION:
                fprintf(fp, "FUNCTION ");
                printOp(fp, cur->code->u.oneOp.op);
                fprintf(fp, " :");
                break;
            case IR_ASSIGN:
                printOp(fp, cur->code->u.assign.left);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_ADD:
                printOp(fp, cur->code->u.binOp.result);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.binOp.op1);
                fprintf(fp, " + ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_SUB:
                printOp(fp, cur->code->u.binOp.result);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.binOp.op1);
                fprintf(fp, " - ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_MUL:
                printOp(fp, cur->code->u.binOp.result);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.binOp.op1);
                fprintf(fp, " * ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_DIV:
                printOp(fp, cur->code->u.binOp.result);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.binOp.op1);
                fprintf(fp, " / ");
                printOp(fp, cur->code->u.binOp.op2);
                break;
            case IR_GET_ADDR:
                printOp(fp, cur->code->u.assign.left);
                fprintf(fp, " := &");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_READ_ADDR:
                printOp(fp, cur->code->u.assign.left);
                fprintf(fp, " := *");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_WRITE_ADDR:
                fprintf(fp, "*");
                printOp(fp, cur->code->u.assign.left);
                fprintf(fp, " := ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_GOTO:
                fprintf(fp, "GOTO ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_IF_GOTO:
                fprintf(fp, "IF ");
                printOp(fp, cur->code->u.ifGoto.x);
                fprintf(fp, " ");
                printOp(fp, cur->code->u.ifGoto.relop);
                fprintf(fp, " ");
                printOp(fp, cur->code->u.ifGoto.y);
                fprintf(fp, " GOTO ");
                printOp(fp, cur->code->u.ifGoto.z);
                break;
            case IR_RETURN:
                fprintf(fp, "RETURN ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_DEC:
                fprintf(fp, "DEC ");
                printOp(fp, cur->code->u.dec.op);
                fprintf(fp, " ");
                fprintf(fp, "%d", cur->code->u.dec.size);
                break;
            case IR_ARG:
                fprintf(fp, "ARG ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_CALL:
                printOp(fp, cur->code->u.assign.left);
                fprintf(fp, " := CALL ");
                printOp(fp, cur->code->u.assign.right);
                break;
            case IR_PARAM:
                fprintf(fp, "PARAM ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_READ:
                fprintf(fp, "READ ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            case IR_WRITE:
                fprintf(fp, "WRITE ");
                printOp(fp, cur->code->u.oneOp.op);
                break;
            }
            fprintf(fp, "\n");
        }
    }
}

// InterCodes func
pInterCodes newInterCodes(pInterCode code)
{
    pInterCodes p = (pInterCodes)malloc(sizeof(InterCodes));
    assert(p != NULL);
    p->code = code;
    p->prev = NULL;
    p->next = NULL;
    return p;
}

void deleteInterCodes(pInterCodes p)
{
    assert(p != NULL);
    deleteInterCode(p->code);
    free(p);
}

// Arg and ArgList func
pArg newArg(pOperand op)
{
    pArg p = (pArg)malloc(sizeof(Arg));
    assert(p != NULL);
    p->op = op;
    p->next = NULL;
}

pArgList newArgList()
{
    pArgList p = (pArgList)malloc(sizeof(ArgList));
    assert(p != NULL);
    p->head = NULL;
    p->cur = NULL;
}

void deleteArg(pArg p)
{
    assert(p != NULL);
    deleteOperand(p->op);
    free(p);
}

void deleteArgList(pArgList p)
{
    assert(p != NULL);
    pArg q = p->head;
    while (q)
    {
        pArg temp = q;
        q = q->next;
        deleteArg(temp);
    }
    free(p);
}

void addArg(pArgList argList, pArg arg)
{
    if (argList->head == NULL)
    {
        argList->head = arg;
        argList->cur = arg;
    }
    else
    {
        argList->cur->next = arg;
        argList->cur = arg;
    }
}

// InterCodeList func
pInterCodeList newInterCodeList()
{
    pInterCodeList p = (pInterCodeList)malloc(sizeof(InterCodeList));
    p->head = NULL;
    p->cur = NULL;
    p->lastArrayName = NULL;
    p->tempVarNum = 1;
    p->labelNum = 1;
}

void deleteInterCodeList(pInterCodeList p)
{
    assert(p != NULL);
    pInterCodes q = p->head;
    while (q)
    {
        pInterCodes temp = q;
        q = q->next;
        deleteInterCodes(temp);
    }
    free(p);
}

void addInterCode(pInterCodeList interCodeList, pInterCodes newCode)
{
    if (interCodeList->head == NULL)
    {
        interCodeList->head = newCode;
        interCodeList->cur = newCode;
    }
    else
    {
        interCodeList->cur->next = newCode;
        newCode->prev = interCodeList->cur;
        interCodeList->cur = newCode;
    }
}

// traverse func
pOperand newTemp()
{
    // printf("newTemp() tempVal:%d\n", interCodeList->tempVarNum);
    char tName[10] = {0};
    sprintf(tName, "t%d", interCodeList->tempVarNum);
    interCodeList->tempVarNum++;
    pOperand temp = newOperand(OP_VARIABLE, newString(tName));
    return temp;
}

pOperand newLabel()
{
    char lName[10] = {0};
    sprintf(lName, "label%d", interCodeList->labelNum);
    interCodeList->labelNum++;
    pOperand temp = newOperand(OP_LABEL, newString(lName));
    return temp;
}

int getSize(pSchema type)
{
    if (type == NULL)
        return 0;
    else if (type->kind == BASIC)
        return 4;
    else if (type->kind == ARRAY)
        return type->data.array.size * getSize(type->data.array.elem);
    else if (type->kind == STRUCTURE)
    {
        int size = 0;
        pFieldList temp = type->data.structure.field;
        while (temp)
        {
            size += getSize(temp->schema);
            temp = temp->next;
        }
        return size;
    }
    return 0;
}

void genInterCodes(pNode node)
{
    if (node == NULL)
        return;
    if (!strcmp(node->name, "ExtDefList"))
        translateExtDefList(node);
    else
    {
        genInterCodes(get_syn_child(node, 0));
        genInterCodes(get_syn_brother(node, 1));
    }
}

void genInterCode(int kind, ...)
{
    va_list vaList;
    pOperand temp = NULL;
    pOperand result = NULL, op1 = NULL, op2 = NULL, relop = NULL;
    int size = 0;
    pInterCodes newCode = NULL;
    assert(kind >= 0 && kind < 19);
    va_start(vaList, kind);
    switch (kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        op1 = va_arg(vaList, pOperand);
        if (op1->kind == OP_ADDRESS)
        {
            temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, op1);
            op1 = temp;
        }
        newCode = newInterCodes(newInterCode(kind, op1));
        addInterCode(interCodeList, newCode);
        break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
    case IR_CALL:
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        if (kind == IR_ASSIGN &&
            (op1->kind == OP_ADDRESS || op2->kind == OP_ADDRESS))
        {
            if (op1->kind == OP_ADDRESS && op2->kind != OP_ADDRESS)
                genInterCode(IR_WRITE_ADDR, op1, op2);
            else if (op2->kind == OP_ADDRESS && op1->kind != OP_ADDRESS)
                genInterCode(IR_READ_ADDR, op1, op2);
            else
            {
                temp = newTemp();
                genInterCode(IR_READ_ADDR, temp, op2);
                genInterCode(IR_WRITE_ADDR, op1, temp);
            }
        }
        else
        {
            newCode = newInterCodes(newInterCode(kind, op1, op2));
            addInterCode(interCodeList, newCode);
        }
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        result = va_arg(vaList, pOperand);
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        if (op1->kind == OP_ADDRESS)
        {
            temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, op1);
            op1 = temp;
        }
        if (op2->kind == OP_ADDRESS)
        {
            temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, op2);
            op2 = temp;
        }
        newCode = newInterCodes(newInterCode(kind, result, op1, op2));
        addInterCode(interCodeList, newCode);
        break;
    case IR_DEC:
        op1 = va_arg(vaList, pOperand);
        size = va_arg(vaList, int);
        newCode = newInterCodes(newInterCode(kind, op1, size));
        addInterCode(interCodeList, newCode);
        break;
    case IR_IF_GOTO:
        result = va_arg(vaList, pOperand);
        relop = va_arg(vaList, pOperand);
        op1 = va_arg(vaList, pOperand);
        op2 = va_arg(vaList, pOperand);
        newCode =
            newInterCodes(newInterCode(kind, result, relop, op1, op2));
        addInterCode(interCodeList, newCode);
        break;
    }
}
void translateExtDefList(pNode node)
{
    // ExtDefList -> ExtDef ExtDefList
    //             | e
    while (node)
    {
        translateExtDef(get_syn_child(node, 0));
        node = get_syn_child(node, 1);
    }
}

void translateExtDef(pNode node)
{
    if (node == NULL)
        return;
    if (interError)
        return;
    // ExtDef -> Specifier ExtDecList SEMI
    //         | Specifier SEMI
    //         | Specifier FunDec CompSt

    // 因为没有全局变量使用，
    // ExtDecList不涉及中间代码生成，类型声明也不涉及，所以只需要处理FunDec和CompSt
    if (!strcmp(get_syn_child(node, 1)->name, "FunDec"))
    {
        translateFunDec(get_syn_child(node, 1));
        translateCompSt(get_syn_child(node, 2));
    }
}

void translateFunDec(pNode node)
{
    assert(node != NULL);
    if (interError)
        return;
    // FunDec -> ID LP VarList RP
    //         | ID LP RP
    genInterCode(IR_FUNCTION,
                 newOperand(OP_FUNCTION, newString(get_syn_child(node, 0)->strval)));
    // pInterCodes func = newInterCodes(newInterCode(
    //     IR_FUNCTION, newOperand(OP_FUNCTION, newString(node->child->val))));
    // addInterCode(interCodeList, func);

    pItem funcItem = searchTableItem(table, get_syn_child(node, 0)->strval);
    pFieldList temp = funcItem->field->schema->data.function.argv;
    while (temp)
    {
        genInterCode(IR_PARAM, newOperand(OP_VARIABLE, newString(temp->name)));
        // pInterCodes arg = newInterCodes(newInterCode(
        //     IR_PARAM, newOperand(OP_VARIABLE, newString(temp->name))));
        // addInterCode(interCodeList, arg);
        temp = temp->next;
    }
}

void translateCompSt(pNode node)
{
    assert(node != NULL);
    if (interError)
        return;
    // CompSt -> LC DefList StmtList RC
    pNode temp = get_syn_child(node, 1);
    if (!strcmp(temp->name, "DefList"))
    {
        translateDefList(temp);
        temp = get_syn_brother(temp, 1);
    }
    if (!strcmp(temp->name, "StmtList"))
    {
        translateStmtList(temp);
    }
}

void translateDefList(pNode node)
{
    if (interError)
        return;
    // DefList -> Def DefList
    //          | e
    while (node)
    {
        translateDef(get_syn_child(node, 0));
        node = get_syn_child(node, 1);
    }
}

void translateDef(pNode node)
{
    if (node == NULL)
        return;
    if (interError)
        return;
    // Def -> Specifier DecList SEMI
    translateDecList(get_syn_child(node, 1));
}

void translateDecList(pNode node)
{
    assert(node != NULL);
    if (interError)
        return;
    // DecList -> Dec
    //          | Dec COMMA DecList
    pNode temp = node;
    while (temp)
    {
        translateDec(get_syn_child(temp, 0));
        if (get_syn_child(temp, 1))
            temp = get_syn_child(temp, 2);
        else
            break;
    }
}

void translateDec(pNode node)
{
    assert(node != NULL);
    if (interError)
        return;
    // Dec -> VarDec
    //      | VarDec ASSIGNOP Exp

    // Dec -> VarDec
    if (get_syn_child(node, 1) == NULL)
    {
        translateVarDec(get_syn_child(node, 0), NULL);
    }
    // Dec -> VarDec ASSIGNOP Exp
    else
    {
        pOperand t1 = newTemp();
        translateVarDec(get_syn_child(node, 0), t1);
        pOperand t2 = newTemp();
        translateExp(get_syn_child(node, 2), t2);
        genInterCode(IR_ASSIGN, t1, t2);
    }
}

void translateVarDec(pNode node, pOperand place)
{
    assert(node != NULL);
    if (interError)
        return;
    // VarDec -> ID
    //         | VarDec LB INT RB

    if (!strcmp(get_syn_child(node, 0)->name, "ID"))
    {
        pItem temp = searchTableItem(table, get_syn_child(node, 0)->strval);
        pSchema type = temp->field->schema;
        if (type->kind == BASIC)
        {
            if (place)
            {
                interCodeList->tempVarNum--;
                setOperand(place, OP_VARIABLE,
                           (void *)newString(temp->field->name));
            }
        }
        else if (type->kind == ARRAY)
        {
            // 不需要完成高维数组情况
            if (type->data.array.elem->kind == ARRAY)
            {
                interError = 1;
                printf(
                    "Cannot translate: Code containsvariables of "
                    "multi-dimensional array type or parameters of array "
                    "type.\n");
                return;
            }
            else
            {
                genInterCode(
                    IR_DEC,
                    newOperand(OP_VARIABLE, newString(temp->field->name)),
                    getSize(type));
            }
        }
        else if (type->kind == STRUCTURE)
        {
            // 3.1选做
            genInterCode(IR_DEC,
                         newOperand(OP_VARIABLE, newString(temp->field->name)),
                         getSize(type));
        }
    }
    else
    {
        translateVarDec(get_syn_child(node, 0), place);
    }
}

void translateStmtList(pNode node)
{
    if (interError)
        return;
    // StmtList -> Stmt StmtList
    //           | e
    while (node)
    {
        translateStmt(get_syn_child(node, 0));
        node = get_syn_child(node, 1);
    }
}

void translateStmt(pNode node)
{
    if (node == NULL)
        return;
    if (interError)
        return;
    // Stmt -> Exp SEMI
    //       | CompSt
    //       | RETURN Exp SEMI
    //       | IF LP Exp RP Stmt
    //       | IF LP Exp RP Stmt ELSE Stmt
    //       | WHILE LP Exp RP Stmt

    // Stmt -> Exp SEMI

    if (!strcmp(get_syn_child(node, 0)->name, "Exp"))
    {
        translateExp(get_syn_child(node, 0), NULL);
    }

    // Stmt -> CompSt
    else if (!strcmp(get_syn_child(node, 0)->name, "CompSt"))
    {
        translateCompSt(get_syn_child(node, 0));
    }

    // Stmt -> RETURN Exp SEMI
    else if (!strcmp(get_syn_child(node, 0)->name, "RETURN"))
    {
        pOperand t1 = newTemp();
        translateExp(get_syn_child(node, 1), t1);
        genInterCode(IR_RETURN, t1);
    }

    // Stmt -> IF LP Exp RP Stmt
    else if (!strcmp(get_syn_child(node, 0)->name, "IF"))
    {
        pNode exp = get_syn_child(node, 2);
        pNode stmt = get_syn_brother(exp, 2);
        pOperand label1 = newLabel();
        pOperand label2 = newLabel();

        translateCond(exp, label1, label2);
        genInterCode(IR_LABEL, label1);
        translateStmt(stmt);
        if (stmt->next == NULL)
        {
            genInterCode(IR_LABEL, label2);
        }
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        else
        {
            pOperand label3 = newLabel();
            genInterCode(IR_GOTO, label3);
            genInterCode(IR_LABEL, label2);
            translateStmt(get_syn_brother(stmt, 2));
            genInterCode(IR_LABEL, label3);
        }
    }

    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(get_syn_child(node, 0)->name, "WHILE"))
    {
        pOperand label1 = newLabel();
        pOperand label2 = newLabel();
        pOperand label3 = newLabel();

        genInterCode(IR_LABEL, label1);
        translateCond(get_syn_child(node, 2), label2, label3);
        genInterCode(IR_LABEL, label2);
        translateStmt(get_syn_child(node, 4));
        genInterCode(IR_GOTO, label1);
        genInterCode(IR_LABEL, label3);
    }
}

void translateExp(pNode node, pOperand place)
{
    assert(node != NULL);
    if (interError)
        return;
    // Exp -> Exp ASSIGNOP Exp
    //      | Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | Exp PLUS Exp
    //      | Exp MINUS Exp
    //      | Exp STAR Exp
    //      | Exp DIV Exp

    //      | MINUS Exp
    //      | NOT Exp
    //      | ID LP Args RP
    //      | ID LP RP
    //      | Exp LB Exp RB
    //      | Exp DOT ID
    //      | ID
    //      | INT
    //      | FLOAT

    // Exp -> LP Exp RP
    if (!strcmp(get_syn_child(node, 0)->name, "("))
        translateExp(get_syn_child(node, 1), place);

    else if (!strcmp(get_syn_child(node, 0)->name, "Exp") ||
             !strcmp(get_syn_child(node, 0)->name, "!"))
    {
        // 条件表达式 和 基本表达式
        if (strcmp(get_syn_child(node, 1)->name, "[") &&
            strcmp(get_syn_child(node, 1)->name, "."))
        {
            // Exp -> Exp AND Exp
            //      | Exp OR Exp
            //      | Exp RELOP Exp
            //      | NOT Exp
            if (!strcmp(get_syn_child(node, 1)->name, "&&") ||
                !strcmp(get_syn_child(node, 1)->name, "||") ||
                !strcmp(get_syn_child(node, 1)->name, "RELOP") ||
                !strcmp(get_syn_child(node, 0)->name, "!"))
            {
                pOperand label1 = newLabel();
                pOperand label2 = newLabel();
                pOperand true_num = newOperand(OP_CONSTANT, 1);
                pOperand false_num = newOperand(OP_CONSTANT, 0);
                genInterCode(IR_ASSIGN, place, false_num);
                translateCond(node, label1, label2);
                genInterCode(IR_LABEL, label1);
                genInterCode(IR_ASSIGN, place, true_num);
            }
            else
            {
                // Exp -> Exp ASSIGNOP Exp
                if (!strcmp(get_syn_child(node, 1)->name, "="))
                {
                    pOperand t2 = newTemp();
                    translateExp(get_syn_child(node, 2), t2);
                    pOperand t1 = newTemp();
                    translateExp(get_syn_child(node, 0), t1);
                    genInterCode(IR_ASSIGN, t1, t2);
                }
                else
                {
                    pOperand t1 = newTemp();
                    translateExp(get_syn_child(node, 0), t1);
                    pOperand t2 = newTemp();
                    translateExp(get_syn_child(node, 2), t2);
                    // Exp -> Exp PLUS Exp
                    if (!strcmp(get_syn_child(node, 1)->name, "+"))
                    {
                        genInterCode(IR_ADD, place, t1, t2);
                    }
                    // Exp -> Exp MINUS Exp
                    else if (!strcmp(get_syn_child(node, 1)->name, "-"))
                    {
                        genInterCode(IR_SUB, place, t1, t2);
                    }
                    // Exp -> Exp STAR Exp
                    else if (!strcmp(get_syn_child(node, 1)->name, "*"))
                    {
                        genInterCode(IR_MUL, place, t1, t2);
                    }
                    // Exp -> Exp DIV Exp
                    else if (!strcmp(get_syn_child(node, 1)->name, "/"))
                    {
                        genInterCode(IR_DIV, place, t1, t2);
                    }
                }
            }
        }
        // 数组和结构体访问
        else
        {
            // Exp -> Exp LB Exp RB
            if (!strcmp(get_syn_child(node, 1)->name, "["))
            {
                //数组
                if (get_syn_child(get_syn_child(node, 0), 1) &&
                    !strcmp(get_syn_child(get_syn_child(node, 0), 1)->name, "["))
                {
                    //多维数组，报错
                    interError = 1;
                    printf(
                        "Cannot translate: Code containsvariables of "
                        "multi-dimensional array type or parameters of array "
                        "type.\n");
                    return;
                }
                else
                {
                    pOperand idx = newTemp();
                    translateExp(get_syn_child(node, 2), idx);
                    pOperand base = newTemp();
                    translateExp(get_syn_child(node, 0), base);

                    pOperand width;
                    pOperand offset = newTemp();
                    pOperand target;
                    // 根据假设，Exp1只会展开为 Exp DOT ID 或 ID
                    // 我们让前一种情况吧ID作为name回填进place返回到这里的base处，在语义分析时将结构体变量也填进表（因为假设无重名），这样两种情况都可以查表得到。
                    pItem item = searchTableItem(table, base->u.name);
                    assert(item->field->schema->kind == ARRAY);
                    width = newOperand(
                        OP_CONSTANT, getSize(item->field->schema->data.array.elem));
                    genInterCode(IR_MUL, offset, idx, width);
                    // 如果是ID[Exp],
                    // 则需要对ID取址，如果前面是结构体内访问，则会返回一个地址类型，不需要再取址
                    if (base->kind == OP_VARIABLE)
                    {
                        // printf("非结构体数组访问\n");
                        target = newTemp();
                        genInterCode(IR_GET_ADDR, target, base);
                    }
                    else
                    {
                        // printf("结构体数组访问\n");
                        target = base;
                    }
                    genInterCode(IR_ADD, place, target, offset);
                    place->kind = OP_ADDRESS;
                    interCodeList->lastArrayName = base->u.name;
                }
            }
            // Exp -> Exp DOT ID
            else
            {
                //结构体
                pOperand temp = newTemp();
                translateExp(get_syn_child(node, 0), temp);
                // 两种情况，Exp直接为一个变量，则需要先取址，若Exp为数组或者多层结构体访问或结构体形参，则target会被填成地址，可以直接用。
                pOperand target;

                if (temp->kind == OP_ADDRESS)
                {
                    target = newOperand(temp->kind, temp->u.name);
                    // target->isAddr = TRUE;
                }
                else
                {
                    target = newTemp();
                    genInterCode(IR_GET_ADDR, target, temp);
                }

                pOperand id = newOperand(
                    OP_VARIABLE, newString(get_syn_child(node, 2)->strval));
                int offset = 0;
                pItem item = searchTableItem(table, temp->u.name);
                //结构体数组，temp是临时变量，查不到表，需要用处理数组时候记录下的数组名老查表
                if (item == NULL)
                {
                    item = searchTableItem(table, interCodeList->lastArrayName);
                }

                pFieldList tmp;
                // 结构体数组 eg: a[5].b
                if (item->field->schema->kind == ARRAY)
                {
                    tmp = item->field->schema->data.array.elem->data.structure.field;
                }
                // 一般结构体
                else
                {
                    tmp = item->field->schema->data.structure.field;
                }
                // 遍历获得offset
                while (tmp)
                {
                    if (!strcmp(tmp->name, id->u.name))
                        break;
                    offset += getSize(tmp->schema);
                    tmp = tmp->next;
                }

                pOperand tOffset = newOperand(OP_CONSTANT, offset);
                if (place)
                {
                    genInterCode(IR_ADD, place, target, tOffset);
                    // 为了处理结构体里的数组把id名通过place回传给上层
                    setOperand(place, OP_ADDRESS, (void *)newString(id->u.name));
                    // place->isAddr = TRUE;
                }
            }
        }
    }
    //单目运算符
    // Exp -> MINUS Exp
    else if (!strcmp(get_syn_child(node, 0)->name, "-"))
    {
        pOperand t1 = newTemp();
        translateExp(get_syn_child(node, 1), t1);
        pOperand zero = newOperand(OP_CONSTANT, 0);
        genInterCode(IR_SUB, place, zero, t1);
    }
    // // Exp -> NOT Exp
    // else if (!strcmp(node->child->name, "NOT")) {
    //     pOperand label1 = newLabel();
    //     pOperand label2 = newLabel();
    //     pOperand true_num = newOperand(OP_CONSTANT, 1);
    //     pOperand false_num = newOperand(OP_CONSTANT, 0);
    //     genInterCode(IR_ASSIGN, place, false_num);
    //     translateCond(node, label1, label2);
    //     genInterCode(IR_LABEL, label1);
    //     genInterCode(IR_ASSIGN, place, true_num);
    // }
    // Exp -> ID LP Args RP
    //		| ID LP RP
    else if (!strcmp(get_syn_child(node, 0)->name, "ID") && get_syn_child(node, 1))
    {
        pOperand funcTemp =
            newOperand(OP_FUNCTION, newString(get_syn_child(node, 0)->strval));
        // Exp -> ID LP Args RP
        if (!strcmp(get_syn_child(node, 2)->name, "Args"))
        {
            pArgList argList = newArgList();
            translateArgs(get_syn_child(node, 2), argList);
            if (!strcmp(get_syn_child(node, 0)->strval, "write"))
            {
                genInterCode(IR_WRITE, argList->head->op);
            }
            else
            {
                pArg argTemp = argList->head;
                while (argTemp)
                {
                    if (argTemp->op == OP_VARIABLE)
                    {
                        pItem item =
                            searchTableItem(table, argTemp->op->u.name);

                        // 结构体作为参数需要传址
                        if (item && item->field->schema->kind == STRUCTURE)
                        {
                            pOperand varTemp = newTemp();
                            genInterCode(IR_GET_ADDR, varTemp, argTemp->op);
                            pOperand varTempCopy = newOperand(OP_ADDRESS, varTemp->u.name);
                            // varTempCopy->isAddr = TRUE;
                            genInterCode(IR_ARG, varTempCopy);
                        }
                    }
                    // 一般参数直接传值
                    else
                    {
                        genInterCode(IR_ARG, argTemp->op);
                    }
                    argTemp = argTemp->next;
                }
                if (place)
                {
                    genInterCode(IR_CALL, place, funcTemp);
                }
                else
                {
                    pOperand temp = newTemp();
                    genInterCode(IR_CALL, temp, funcTemp);
                }
            }
        }
        // Exp -> ID LP RP
        else
        {
            if (!strcmp(get_syn_child(node, 0)->strval, "read"))
            {
                genInterCode(IR_READ, place);
            }
            else
            {
                if (place)
                {
                    genInterCode(IR_CALL, place, funcTemp);
                }
                else
                {
                    pOperand temp = newTemp();
                    genInterCode(IR_CALL, temp, funcTemp);
                }
            }
        }
    }
    // Exp -> ID
    else if (!strcmp(get_syn_child(node, 0)->name, "ID"))
    {
        // 不考虑结构体
        interCodeList->tempVarNum--;
        setOperand(place, OP_VARIABLE, (void *)newString(get_syn_child(node, 0)->strval));

        // pOperand t1 = newOperand(OP_VARIABLE, id_name->field->name);
        // genInterCode(IR_ASSIGN, place, t1);
    }
    else
    {
        // // Exp -> FLOAT
        // 无浮点数常数
        // if (!strcmp(node->child->name, "FLOAT")) {
        //     pOperand t1 = newOperand(OP_CONSTANT, node->child->val);
        //     genInterCode(IR_ASSIGN, place, t1);
        // }

        // Exp -> INT
        interCodeList->tempVarNum--;
        setOperand(place, OP_CONSTANT, (void *)get_syn_child(node, 0)->intval);
        // pOperand t1 = newOperand(OP_CONSTANT, node->child->val);
        // genInterCode(IR_ASSIGN, place, t1);
    }
}

void translateCond(pNode node, pOperand labelTrue, pOperand labelFalse)
{
    assert(node != NULL);
    if (interError)
        return;
    // Exp -> Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | NOT Exp

    // Exp -> NOT Exp
    if (!strcmp(get_syn_child(node, 0)->name, "!"))
    {
        translateCond(get_syn_child(node, 1), labelFalse, labelTrue);
    }
    // Exp -> Exp RELOP Exp
    else if (!strcmp(get_syn_child(node, 1)->name, "RELOP"))
    {
        pOperand t1 = newTemp();
        pOperand t2 = newTemp();
        translateExp(get_syn_child(node, 0), t1);
        translateExp(get_syn_child(node, 2), t2);

        pOperand relop =
            newOperand(OP_RELOP, newString(get_syn_child(node, 1)->strval));

        if (t1->kind == OP_ADDRESS)
        {
            pOperand temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, t1);
            t1 = temp;
        }
        if (t2->kind == OP_ADDRESS)
        {
            pOperand temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, t2);
            t2 = temp;
        }

        genInterCode(IR_IF_GOTO, t1, relop, t2, labelTrue);
        genInterCode(IR_GOTO, labelFalse);
    }
    // Exp -> Exp AND Exp
    else if (!strcmp(get_syn_child(node, 1)->name, "&&"))
    {
        pOperand label1 = newLabel();
        translateCond(get_syn_child(node, 0), label1, labelFalse);
        genInterCode(IR_LABEL, label1);
        translateCond(get_syn_child(node, 2), labelTrue, labelFalse);
    }
    // Exp -> Exp OR Exp
    else if (!strcmp(get_syn_child(node, 1)->name, "||"))
    {
        pOperand label1 = newLabel();
        translateCond(get_syn_child(node, 0), labelTrue, label1);
        genInterCode(IR_LABEL, label1);
        translateCond(get_syn_child(node, 2), labelTrue, labelFalse);
    }
    // other cases
    else
    {
        pOperand t1 = newTemp();
        translateExp(node, t1);
        pOperand t2 = newOperand(OP_CONSTANT, 0);
        pOperand relop = newOperand(OP_RELOP, newString("!="));

        if (t1->kind == OP_ADDRESS)
        {
            pOperand temp = newTemp();
            genInterCode(IR_READ_ADDR, temp, t1);
            t1 = temp;
        }
        genInterCode(IR_IF_GOTO, t1, relop, t2, labelTrue);
        genInterCode(IR_GOTO, labelFalse);
    }
}

void translateArgs(pNode node, pArgList argList)
{
    assert(node != NULL);
    assert(argList != NULL);
    if (interError)
        return;
    // Args -> Exp COMMA Args
    //       | Exp

    // Args -> Exp
    pArg temp = newArg(newTemp());
    translateExp(get_syn_child(node, 0), temp->op);

    if (temp->op->kind == OP_VARIABLE)
    {
        pItem item = searchTableItem(table, temp->op->u.name);
        if (item && item->field->schema->kind == ARRAY)
        {
            interError = 1;
            printf(
                "Cannot translate: Code containsvariables of "
                "multi-dimensional array type or parameters of array "
                "type.\n");
            return;
        }
    }
    addArg(argList, temp);

    // Args -> Exp COMMA Args
    if (get_syn_child(node, 1) != NULL)
    {
        translateArgs(get_syn_child(node, 2), argList);
    }
}
