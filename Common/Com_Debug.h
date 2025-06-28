#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
// 替换__FILE__ 只保留文件名称  取出路径名称
// FILE__NAME => 去除\\的路径名称
#define FILE__NAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// 最终调用__FILE__NAME__  => 去除带有/路径名称  内部包含FILE__NAME
#define __FILE__NAME__              (strrchr(FILE__NAME, '/') ? strrchr(FILE__NAME, '/') + 1 : FILE__NAME)

#define debug_printf(format, ...)   printf("[%s:%d]" format, __FILE__NAME__, __LINE__, ##__VA_ARGS__)
#define debug_printfln(format, ...) printf("[%s:%d]" format "\n", __FILE__NAME__, __LINE__, ##__VA_ARGS__)

#else
#define __FILE_NAME
#define __FILE_NAME__
#define debug_printf(format, ...)
#define debug_printfln(format, ...)

#endif

void Com_Debug_Init(void);

#endif /* __COM_DEBUG_H__ */