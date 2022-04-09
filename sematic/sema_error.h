#ifndef _SEMA_ERROR_H_
#define _SEMA_ERROR_H_

/**
 * 语义分析错误类型
 * 对应书P39的17个错误类型
 */
typedef enum error_type
{
    UNDEF_VAR = 1,        // 变量未定义即使用
    UNDEF_FUNC,           // 函数未定义即使用
    REDEF_VAR,            // 变量重复定义
    REDEF_FUNC,           // 函数重复定义
    TYPE_MISMATCH_ASSIGN, // 赋值号两边类型不匹配
    LEFT_VAR_ASSIGN,      // 赋值号左边无法赋值
    TYPE_MISMATCH_OP,     // 操作数不匹配
    TYPE_MISMATCH_RETURN, // 返回值类型不对
    FUNC_AGRC_MISMATCH,   // 函数参数有误
    NOT_A_ARRAY,          // 变量非数组
    NOT_A_FUNC,           // 变量非函数
    NOT_A_INT,            // 变量非int
    ILLEGAL_USE_DOT,      // 变量非数组（非法使用取域运算符）
    NONEXISTFIELD,        // struct域不存在
    REDEF_FEILD,          // 域重复定义
    DUPLICATED_NAME,      // 结构体名重复定义（先前值也可能为变量or函数）
    UNDEF_STRUCT          // 结构体未定义即使用
} ErrorType;

void pError(ErrorType type, int line, char *msg);

#endif
