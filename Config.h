#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* CONFIGURATION */
#define ALLOCATION_SIZE 1024
#define BLOCK_SIZE 128
#define EXTENSION 256


#ifdef DEBUG_CONFIG
/*  ---- USED FOR DEBUGGGING PURPOSES ---- */
#define SEND_UART 0u
#define PRINT_DEBUG_TRACE 1u
#else
/*  ---- DISABLE ALL DEBUG FEATURES ---- */
#define PRINT_DEBUG_TRACE 0u
#define SEND_UART 1u
#endif

#define PRINT_BLOCK_RAW 0u
#define PRINT_BLOCK_UART 0u
#define PRINT_BLOCK_CRC 1u
#endif // CONFIG_H_INCLUDED
