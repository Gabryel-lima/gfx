#pragma once

/**
 * @brief Carregador mínimo de símbolos OpenGL via dlopen/dlsym.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/gl_loader.h
 */

/** Tipo para enums OpenGL */
typedef unsigned int  GLenum; // Tipo para enums do OpenGL
/** Tipo para identificadores de objetos OpenGL */
typedef unsigned int  GLuint; // Tipo para identificadores de objetos OpenGL
/** Tipo inteiro usado por chamadas OpenGL */
typedef int           GLint;  // Tipo inteiro usado por chamadas OpenGL
/** Tipo inteiro para contagens e tamanhos em chamadas OpenGL */
typedef int           GLsizei; // Tipo inteiro para contagens e tamanhos
/** Tipo float usado por OpenGL */
typedef float         GLfloat; // Tipo float usado por OpenGL
/** Tipo char usado em strings GLSL */
typedef char          GLchar; // Tipo char usado em strings GLSL
/** Tipo booleano usado por algumas chamadas OpenGL */
typedef unsigned char GLboolean; // Tipo booleano OpenGL
/** Tipo inteiro para tamanhos e offsets de buffer */
typedef signed long   GLsizeiptr; // Tipo para tamanhos/offsets de buffer
/** Tipo para máscaras de bits OpenGL */
typedef unsigned int  GLbitfield; // Máscaras de bits OpenGL

/** Ponteiros para funções OpenGL carregadas dinamicamente */
typedef void  (*PFN_glGenBuffers)    (GLint, GLuint*);                         // Gera buffers
typedef void  (*PFN_glBindBuffer)    (GLenum, GLuint);                         // Faz bind em buffer
typedef void  (*PFN_glBufferData)    (GLenum, GLsizeiptr, const void*, GLenum); // Envia dados ao buffer
typedef void  (*PFN_glClearColor)     (GLfloat, GLfloat, GLfloat, GLfloat);     // Define a cor de limpeza
typedef void  (*PFN_glClear)          (GLbitfield);                             // Limpa buffers
typedef void  (*PFN_glViewport)       (GLint, GLint, GLsizei, GLsizei);         // Ajusta o viewport
typedef void  (*PFN_glEnable)         (GLenum);                                 // Habilita uma capacidade
typedef void  (*PFN_glDepthFunc)      (GLenum);                                 // Configura teste de profundidade
typedef GLuint(*PFN_glCreateShader)  (GLenum);                                 // Cria shader
typedef void  (*PFN_glShaderSource)   (GLuint, GLsizei, const GLchar* const*, const GLint*); // Define a fonte do shader
typedef void  (*PFN_glCompileShader)   (GLuint);                                // Compila shader
typedef void  (*PFN_glGetShaderiv)     (GLuint, GLenum, GLint*);                // Consulta estado do shader
typedef void  (*PFN_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);    // Obtém log do shader
typedef void  (*PFN_glDeleteShader)    (GLuint);                                // Remove shader
typedef GLuint(*PFN_glCreateProgram)   (void);                                  // Cria programa
typedef void  (*PFN_glBindAttribLocation)(GLuint, GLuint, const GLchar*);       // Vincula índice de atributo
typedef void  (*PFN_glAttachShader)    (GLuint, GLuint);                        // Anexa shader ao programa
typedef void  (*PFN_glLinkProgram)     (GLuint);                                // Faz link do programa
typedef void  (*PFN_glGetProgramiv)    (GLuint, GLenum, GLint*);                // Consulta estado do programa
typedef void  (*PFN_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);   // Obtém log do programa
typedef void  (*PFN_glDeleteProgram)   (GLuint);                                // Remove programa
typedef void  (*PFN_glUseProgram)      (GLuint);                                 // Ativa programa
typedef GLint (*PFN_glGetUniformLocation)(GLuint, const GLchar*);                // Localiza uniform
typedef void  (*PFN_glUniformMatrix4fv) (GLint, GLsizei, GLboolean, const GLfloat*); // Envia matriz 4x4
typedef void  (*PFN_glDeleteBuffers)   (GLsizei, const GLuint*);                 // Remove buffers
typedef void  (*PFN_glEnableVertexAttribArray)(GLuint);                          // Habilita atributo de vértice
typedef void  (*PFN_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*); // Define atributo de vértice
typedef void  (*PFN_glDrawArrays)      (GLenum, GLint, GLsizei);                 // Desenha arrays

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
    PFN_glClearColor    ClearColor; /**< Ponteiro para glClearColor */
    PFN_glClear         Clear;      /**< Ponteiro para glClear */
    PFN_glViewport      Viewport;   /**< Ponteiro para glViewport */
    PFN_glEnable        Enable;     /**< Ponteiro para glEnable */
    PFN_glDepthFunc     DepthFunc;  /**< Ponteiro para glDepthFunc */
    PFN_glCreateShader  CreateShader;/**< Ponteiro para glCreateShader */
    PFN_glShaderSource  ShaderSource; /**< Ponteiro para glShaderSource */
    PFN_glCompileShader CompileShader; /**< Ponteiro para glCompileShader */
    PFN_glGetShaderiv   GetShaderiv;   /**< Ponteiro para glGetShaderiv */
    PFN_glGetShaderInfoLog GetShaderInfoLog; /**< Ponteiro para glGetShaderInfoLog */
    PFN_glDeleteShader  DeleteShader;  /**< Ponteiro para glDeleteShader */
    PFN_glCreateProgram CreateProgram; /**< Ponteiro para glCreateProgram */
    PFN_glBindAttribLocation BindAttribLocation; /**< Ponteiro para glBindAttribLocation */
    PFN_glAttachShader  AttachShader;  /**< Ponteiro para glAttachShader */
    PFN_glLinkProgram   LinkProgram;   /**< Ponteiro para glLinkProgram */
    PFN_glGetProgramiv  GetProgramiv;  /**< Ponteiro para glGetProgramiv */
    PFN_glGetProgramInfoLog GetProgramInfoLog; /**< Ponteiro para glGetProgramInfoLog */
    PFN_glDeleteProgram DeleteProgram; /**< Ponteiro para glDeleteProgram */
    PFN_glUseProgram    UseProgram;    /**< Ponteiro para glUseProgram */
    PFN_glGetUniformLocation GetUniformLocation; /**< Ponteiro para glGetUniformLocation */
    PFN_glUniformMatrix4fv UniformMatrix4fv; /**< Ponteiro para glUniformMatrix4fv */
    PFN_glDeleteBuffers DeleteBuffers; /**< Ponteiro para glDeleteBuffers */
    PFN_glEnableVertexAttribArray EnableVertexAttribArray; /**< Ponteiro para glEnableVertexAttribArray */
    PFN_glVertexAttribPointer VertexAttribPointer; /**< Ponteiro para glVertexAttribPointer */
    PFN_glDrawArrays    DrawArrays;    /**< Ponteiro para glDrawArrays */
} GLProcs;

/**
 * Carrega o subconjunto de símbolos OpenGL usado pelo projeto.
 * @return GLProcs preenchido com os ponteiros disponíveis.
 */
GLProcs gfx_gl_load(void);

/** Libera a biblioteca dinâmica carregada por `gfx_gl_load`. */
void gfx_gl_close(GLProcs *gl);
