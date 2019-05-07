/*
 * debug.h
 *
 *  Created on: Apr 15, 2019
 *      Author: murphy
 */

#ifndef SYSTEM_DEBUG_H_
#define SYSTEM_DEBUG_H_

#define DEBUG_LVL_NONE 0
#define DEBUG_LVL_ERROR 1
#define DEBUG_LVL_WARNING 2
#define DEBUG_LVL_INFO 3

#define DEBUG_LEVEL DEBUG_LVL_INFO

#if DEBUG_LEVEL == DEBUG_LVL_INFO
#define DEBUG_INFO(format, ...) printf("[INFO] " format, ##__VA_ARGS__)
#define DEBUG_WARNING(format, ...) printf("[WARNING] " format, ##__VA_ARGS__)
#define DEBUG_ERROR(format, ...) printf(f"[ERROR] " ormat, ##__VA_ARGS__)

#elif DEBUG_LEVEL == DEBUG_LVL_WARNING
#define DEBUG_WARNING(format, ...) printf("[WARNING]" format, ##__VA_ARGS__)
#define DEBUG_ERROR(format, ...) printf("[ERROR]" format, ##__VA_ARGS__)
#define DEBUG_INFO(format, ...) ((void)0)


#elif DEBUG_LEVEL == DEBUG_LVL_ERROR
#define DEBUG_ERROR(format, ...) printf("[ERROR]" format, ##__VA_ARGS__)
#define DEBUG_WARNING(format, ...) ((void)0)
#define DEBUG_INFO(format, ...) ((void)0)

#elif DEBUG_LEVEL == DEBUG_LVL_NONE
#define DEBUG_ERROR(format, ...) ((void)0)
#define DEBUG_WARNING(format, ...) ((void)0)
#define DEBUG_INFO(format, ...) ((void)0)
#endif

void debug_init(void);

#endif /* SYSTEM_DEBUG_H_ */
