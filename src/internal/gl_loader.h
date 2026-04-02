#pragma once

/**
 * @brief Carregador mínimo de símbolos OpenGL via dlopen/dlsym.
 * @author Gabryel-lima
 * @date 2026-05-02
 * @file src/internal/gl_loader.h
 */

/** Tipo para enums OpenGL */
typedef unsigned int  GLenum; // Tipo para enums do OpenGL
/** Tipo para identificadores de objetos OpenGL */
typedef unsigned int  GLuint; // Tipo para identificadores de objetos OpenGL
/** Tipo inteiro usado por chamadas OpenGL */
typedef int           GLint;  // Tipo inteiro usado por chamadas OpenGL
/** Tipo float usado por OpenGL */
typedef float         GLfloat; // Tipo float usado por OpenGL
/** Tipo char usado em strings GLSL */
typedef char          GLchar; // Tipo char usado em strings GLSL
/** Tipo inteiro para tamanhos e offsets de buffer */
typedef signed long   GLsizeiptr; // Tipo para tamanhos/offsets de buffer

/** Ponteiros para funções OpenGL carregadas dinamicamente */
typedef void  (*PFN_glGenBuffers)    (GLint, GLuint*);                         // Gera buffers
typedef void  (*PFN_glBindBuffer)    (GLenum, GLuint);                         // Faz bind em buffer
typedef void  (*PFN_glBufferData)    (GLenum, GLsizeiptr, const void*, GLenum); // Envia dados ao buffer
typedef GLuint(*PFN_glCreateShader)  (GLenum);                                 // Cria shader

/**
 * Tabela com funções OpenGL carregadas dinamicamente.
 * @param handle Handle da biblioteca dinâmica (usado para dlclose)
 * @param GenBuffers Ponteiro para função glGenBuffers
 * @param BindBuffer Ponteiro para função glBindBuffer
 * @param BufferData Ponteiro para função glBufferData
 * @param CreateShader Ponteiro para função glCreateShader
 * @note Campos serão nulos se o carregamento falhar.
 */
typedef struct GLProcs {
    void *handle;                 /**< Handle da biblioteca dinâmica */
    PFN_glGenBuffers    GenBuffers; /**< Ponteiro para glGenBuffers */
    PFN_glBindBuffer    BindBuffer; /**< Ponteiro para glBindBuffer */
    PFN_glBufferData    BufferData; /**< Ponteiro para glBufferData */
    PFN_glCreateShader  CreateShader;/**< Ponteiro para glCreateShader */
} GLProcs;

/**
 * Carrega o subconjunto de símbolos OpenGL usado pelo projeto.
 * @return GLProcs preenchido com os ponteiros disponíveis.
 */
GLProcs gl_load(void);
