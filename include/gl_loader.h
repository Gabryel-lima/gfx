#pragma once

/**
 * @file include/gl_loader.h
 * @brief Carregador de funções OpenGL dinâmicas (procs) utilizado pelo projeto.
 */

/** OpenGL enum type */
typedef unsigned int  GLenum; // Tipo para enums do OpenGL
/** OpenGL uint object handle */
typedef unsigned int  GLuint; // Tipo para identificadores de objetos OpenGL
/** OpenGL int type */
typedef int           GLint;  // Tipo inteiro usado por chamadas OpenGL
/** OpenGL float type */
typedef float         GLfloat; // Tipo float usado por OpenGL
/** OpenGL char type for source strings */
typedef char          GLchar; // Tipo char usado em strings GLSL
/** Pointer-sized integer for buffer sizes */
typedef signed long   GLsizeiptr; // Tipo para tamanhos/offsets de buffer

/** Ponteiros para funções OpenGL carregadas dinamicamente */
typedef void  (*PFN_glGenBuffers)    (GLint, GLuint*);                         // Gera buffers
typedef void  (*PFN_glBindBuffer)    (GLenum, GLuint);                         // Faz bind em buffer
typedef void  (*PFN_glBufferData)    (GLenum, GLsizeiptr, const void*, GLenum); // Envia dados ao buffer
typedef GLuint(*PFN_glCreateShader)  (GLenum);                                 // Cria shader

/**
 * Estrutura com funções carregadas do OpenGL.
 * @note Campos serão nulos se o carregamento falhar.
 */
typedef struct {
    void *handle;                 /**< Handle da biblioteca dinâmica */
    PFN_glGenBuffers    GenBuffers; /**< Ponteiro para glGenBuffers */
    PFN_glBindBuffer    BindBuffer; /**< Ponteiro para glBindBuffer */
    PFN_glBufferData    BufferData; /**< Ponteiro para glBufferData */
    PFN_glCreateShader  CreateShader;/**< Ponteiro para glCreateShader */
} GLProcs;

/**
 * Carrega as funções OpenGL necessárias e retorna uma tabela de ponteiros.
 * @return GLProcs preenchido com os ponteiros disponíveis.
 */
GLProcs gl_load(void);
