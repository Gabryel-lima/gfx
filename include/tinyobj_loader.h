#pragma once

/**
 * @file tiny_loader.h
 * @brief Interface pública do tinyobjloader, um carregador de arquivos .obj e .mtl.
 * @author Gabryel-lima
 * @date 2026-05-01
 */

#ifndef TINYOBJ_LOADER_C_H_  // Include guard
#define TINYOBJ_LOADER_C_H_  // Evita múltiplas inclusões do mesmo header

#include <stddef.h>

#ifdef __cplusplus  // Permite que o header seja incluído em código C++
extern "C" {
#endif

/** Estrutura que representa um material
 * @param name Nome do material, pode ser usado para lookup
 * @param ambient Coeficiente de reflexão ambiente, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
 * @param diffuse Coeficiente de reflexão difusa, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
 * @param specular Coeficiente de reflexão especular, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
 * @param transmittance Coeficiente de transmitância, onde 0.0 é opaco e 1.0 é totalmente transmissivo
 * @param emission Coeficiente de emissão, onde 0.0 é sem emissão e 1.0 é totalmente emissivo
 * @param shininess Seria o Ns na MTL, mas aqui é só para controle de brilho especular, sem relação com o shader
 * @param ior Índice de refração
 * @param dissolve Dissolve é a opacidade, onde 1.0 é totalmente opaco e 0.0 é totalmente transparente
 * @param illum Illumination model (ver http://www.fileformat.info/format/material/)
 * @param ambient_texname Nome do arquivo de textura para map_Ka
 * @param diffuse_texname Nome do arquivo de textura para map_Kd
 * @param specular_texname Nome do arquivo de textura para map_Ks
 * @param specular_highlight_texname Nome do arquivo de textura para map_Ns
 * @param bump_texname Nome do arquivo de textura para map_bump ou bump
 * @param displacement_texname Nome do arquivo de textura para disp
 * @param alpha_texname Nome do arquivo de textura para map_d
 * @note Após chamar esta função, o ponteiro `s` não deve ser usado novamente
 */
typedef struct Tinyobj_material {
  char *name;               // nome do material, pode ser usado para lookup

  float ambient[3];         // coeficiente de reflexão ambiente, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
  float diffuse[3];         // coeficiente de reflexão difusa, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
  float specular[3];        // coeficiente de reflexão especular, onde 0.0 é sem reflexão e 1.0 é totalmente reflexivo
  float transmittance[3];   // coeficiente de transmitância, onde 0.0 é opaco e 1.0 é totalmente transmissivo
  float emission[3];        // coeficiente de emissão, onde 0.0 é sem emissão e 1.0 é totalmente emissivo
  float shininess;          // seria o Ns na MTL, mas aqui é só para controle de brilho especular, sem relação com o shader
  float ior;      // índice de refração
  float dissolve; // dissolve é a opacidade, onde 1.0 é totalmente opaco e 0.0 é totalmente transparente
  /* illumination model (see http://www.fileformat.info/format/material/) */
  int illum;

  int pad0;       // padding para alinhamento de 4 bytes

  char *ambient_texname;            /* map_Ka */
  char *diffuse_texname;            /* map_Kd */
  char *specular_texname;           /* map_Ks */
  char *specular_highlight_texname; /* map_Ns */
  char *bump_texname;               /* map_bump, bump */
  char *displacement_texname;       /* disp */
  char *alpha_texname;              /* map_d */
} Tinyobj_material;

/** Estrutura que representa uma forma (shape) 
 * @param name Nome da forma, pode ser usado para lookup
 * @param face_offset Índice de início dos vértices dessa forma no array `faces` de `Tinyobj_attrib`
 * @param length Número de vértices dessa forma, ou seja, quantos elementos do array `faces` pertencem a essa forma
*/
typedef struct Tinyobj_shape {
  char *name; /* group name or object name. */
  unsigned int face_offset;
  unsigned int length;
} Tinyobj_shape;

/** Estrutura que representa um índice de vértice
 * @param v_idx Índice do vértice
 * @param vt_idx Índice da coordenada de textura
 * @param vn_idx Índice da normal
 */
typedef struct Tinyobj_vertex_index { int v_idx, vt_idx, vn_idx; } Tinyobj_vertex_index;

/** Estrutura que representa os atributos de um objeto
 * @param num_vertices Número de vértices
 * @param num_normals Número de normais
 * @param num_texcoords Número de coordenadas de textura
 * @param num_faces Número de faces
 * @param num_face_num_verts Número de vértices por face (usado para formas com faces de tamanho variável, como quads ou ngons)
 * @param pad0 Padding para alinhamento de 4 bytes
 * @param vertices Array de vértices, onde cada vértice é representado por 3 floats (x, y, z)
 * @param normals Array de normais, onde cada normal é representada por 3 floats (x, y, z)
 * @param texcoords Array de coordenadas de textura, onde cada coordenada é representada por 2 floats (u, v)
 * @param faces Array de índices de vértice, onde cada face é representada por um array de `Tinyobj_vertex_index` (v_idx, vt_idx, vn_idx)
 * @param face_num_verts Array que indica quantos vértices cada face possui, usado para formas com faces de tamanho variável
 * @param material_ids Array de índices de material para cada face, onde cada elemento é um índice que referencia um material em `Tinyobj_material`
 * @note Após chamar esta função, os ponteiros `vertices`, `normals`, `texcoords`, `faces`, `face_num_verts` e `material_ids` não devem ser usados novamente
 */
typedef struct Tinyobj_attrib {
  unsigned int num_vertices; // Número de vértices
  unsigned int num_normals; // Número de normais
  unsigned int num_texcoords; // Número de coordenadas de textura
  unsigned int num_faces; // Número de faces
  unsigned int num_face_num_verts; // Número de vértices por face (usado para formas com faces de tamanho variável, como quads ou ngons)

  int pad0; // Padding para alinhamento de 4 bytes

  float *vertices; // Array de vértices, onde cada vértice é representado por 3 floats (x, y, z)
  float *normals; // Array de normais, onde cada normal é representada por 3 floats (x, y, z)
  float *texcoords; // Array de coordenadas de textura, onde cada coordenada é representada por 2 floats (u, v)
  Tinyobj_vertex_index *faces; // Array de índices de vértice, onde cada face é representada por um array de `Tinyobj_vertex_index` (v_idx, vt_idx, vn_idx)
  int *face_num_verts; // Array que indica quantos vértices cada face possui, usado para formas com faces de tamanho variável
  int *material_ids; // Array de índices de material para cada face, onde cada elemento é um índice que referencia um material em `Tinyobj_material`
} Tinyobj_attrib;


#define TINYOBJ_FLAG_TRIANGULATE (1 << 0)   // Triangulariza faces com mais de 3 vértices (quads, ngons) em triângulos

#define TINYOBJ_INVALID_INDEX (0x80000000)  // Valor especial para indicar um índice inválido, usado quando um vértice, normal ou coordenada de textura não está presente

#define TINYOBJ_SUCCESS (0)  // Operação bem-sucedida
#define TINYOBJ_ERROR_EMPTY (-1)  // Arquivo vazio
#define TINYOBJ_ERROR_INVALID_PARAMETER (-2)  // Parâmetro inválido
#define TINYOBJ_ERROR_FILE_OPERATION (-3)  // Erro de operação de arquivo

/** Callback para leitura de arquivos
 * @param[in] ctx Ponteiro de contexto fornecido pelo usuário
 * @param[in] filename Nome do arquivo a ser carregado
 * @param[in] is_mtl 1 quando o callback é invocado para carregar .mtl, 0 para .obj
 * @param[in] obj_filename Nome do arquivo .obj. Útil quando você deseja carregar .mtl do mesmo local do .obj. Quando o callback é chamado para carregar .obj, `filename` e `obj_filename` são iguais.
 * @param[out] buf Conteúdo do arquivo carregado
 * @param[out] len Tamanho do conteúdo (arquivo)
 */
typedef void (*File_reader_callback)(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len);

/** Parse wavefront .obj
 * @param[out] attrib Atributos
 * @param[out] shapes Array de formas analisadas
 * @param[out] num_shapes Tamanho do array `shapes`
 * @param[out] materials Array de materiais analisados
 * @param[out] num_materials Tamanho do array `materials`
 * @param[in] file_name Nome do arquivo .obj
 * @param[in] file_reader Função de callback para leitura de arquivos (para ler .obj e .mtl)
 * @param[in] ctx Ponteiro de contexto passado para o callback file_reader
 * @param[in] flags Combinação de TINYOBJ_FLAG_***
 *
 * Retorna TINYOBJ_SUCCESS se tudo ocorrer bem.
 * Retorna TINYOBJ_ERROR_*** em caso de erro.
 */
extern int tinyobj_load_obj(Tinyobj_attrib *attrib, Tinyobj_shape **shapes,
                             size_t *num_shapes, Tinyobj_material **materials,
                             size_t *num_materials, const char *file_name, File_reader_callback file_reader,
                             void *ctx, unsigned int flags);

/** load wavefront .mtl
 * @param[out] materials_out Array de materiais analisados
 * @param[out] num_materials_out Tamanho do array `materials_out`
 * @param[in] filename Nome do arquivo .mtl
 * @param[in] obj_filename Nome do arquivo .obj. Pode ser NULL se você apenas deseja analisar o arquivo .mtl.
 * @param[in] file_reader Função de callback para leitura de arquivos
 * @param[in] ctx Ponteiro de contexto passado para o callback file_reader
 *
 * Retorna TINYOBJ_SUCCESS se tudo ocorrer bem.
 * Retorna TINYOBJ_ERROR_*** em caso de erro.
 */
extern int tinyobj_load_mtl(Tinyobj_material **materials_out,
                                  size_t *num_materials_out,
                                  const char *filename, const char *obj_filename, 
                                  File_reader_callback file_reader, void *ctx);

/** Inicializa os atributos de um objeto 
 * @param attrib Ponteiro para a estrutura `Tinyobj_attrib` a ser inicializada
 */
extern void tinyobj_attrib_init(Tinyobj_attrib *attrib);


/** Libera os atributos de um objeto
 * @param attrib Ponteiro para a estrutura `Tinyobj_attrib` a ser liberada 
 */
extern void tinyobj_attrib_free(Tinyobj_attrib *attrib);

/** Libera as formas analisadas
 * @param shapes Array de formas a ser liberado
 * @param num_shapes Tamanho do array `shapes`
 */
extern void tinyobj_shapes_free(Tinyobj_shape *shapes, size_t num_shapes);

/** Libera os materiais analisados
 * @param materials Array de materiais a ser liberado
 * @param num_materials Tamanho do array `materials`
 */
extern void tinyobj_materials_free(Tinyobj_material *materials,
                                   size_t num_materials);

#ifdef __cplusplus
}
#endif
#endif /* TINYOBJ_LOADER_C_H_ */

#ifdef TINYOBJ_LOADER_C_IMPLEMENTATION
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#if defined(TINYOBJ_MALLOC) && defined(TINYOBJ_CALLOC) && defined(TINYOBJ_FREE) && (defined(TINYOBJ_REALLOC) || defined(TINYOBJ_REALLOC_SIZED))
/* ok */
#elif !defined(TINYOBJ_MALLOC) && !defined(TINYOBJ_CALLOC) && !defined(TINYOBJ_FREE) && !defined(TINYOBJ_REALLOC) && !defined(TINYOBJ_REALLOC_SIZED)
/* ok */
#else
#error "Must define all or none of TINYOBJ_MALLOC, TINYOBJ_CALLOC, TINYOBJ_FREE, and TINYOBJ_REALLOC (or TINYOBJ_REALLOC_SIZED)."
#endif

#ifndef TINYOBJ_MALLOC

#include <stdlib.h>

/** Default memory allocation functions */

/** alias para o malloc */
#define TINYOBJ_MALLOC malloc
/** alias para o realloc */
#define TINYOBJ_REALLOC realloc
/** alias para o calloc */
#define TINYOBJ_CALLOC calloc
/** alias para o free */
#define TINYOBJ_FREE free
#endif

#ifndef TINYOBJ_REALLOC_SIZED
/** alias para o realloc com tamanho */
#define TINYOBJ_REALLOC_SIZED(p, oldsz, newsz) TINYOBJ_REALLOC(p,newsz)
#endif

/** Maximo de faces por linha 'f' */
#define TINYOBJ_MAX_FACES_PER_F_LINE (16)
/** Maximo comprimento do caminho do arquivo */
#define TINYOBJ_MAX_FILEPATH (8192)

/** Verifica se o caractere é um espaço */
#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
/** Verifica se o caractere é um dígito */
#define IS_DIGIT(x) ((unsigned int)((x) - '0') < (unsigned int)(10))
/** Verifica se o caractere é uma nova linha */
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

/** Skip spaces in a token 
 *  @param token Ponteiro para o ponteiro de string a ser processado. 
 *  @note O ponteiro apontado por `token` será avançado 
 *  para pular os espaços iniciais.
*/
static void skip_space(const char **token) {
  while ((*token)[0] == ' ' || (*token)[0] == '\t') {
    (*token)++;
  }
}

/** Skip spaces and carriage returns in a token 
 *  @param token Ponteiro para o ponteiro de string a ser processado. 
 *  @note O ponteiro apontado por `token` será avançado 
 *  para pular os espaços iniciais e retornos de carro.
*/
static void skip_space_and_cr(const char **token) {
  while ((*token)[0] == ' ' || (*token)[0] == '\t' || (*token)[0] == '\r') {
    (*token)++;
  }
}

/** Skip until the end of the line 
 *  @param token Ponteiro para o ponteiro de string a ser processado. 
 *  @note O ponteiro apontado por `token` será avançado 
 *  para pular até o final da linha.
*/
static int until_space(const char *token) {
  const char *p = token;
  while (p[0] != '\0' && p[0] != ' ' && p[0] != '\t' && p[0] != '\r') {
    p++;
  }

  return (int)(p - token);
}

/** Calculate the length of a token until a newline character
 *  @param token Ponteiro para a string a ser processada.
 *  @param n Tamanho da string `token`.
 *  @return Comprimento da string até o próximo caractere de nova linha.
*/
static size_t length_until_newline(const char *token, size_t n) {
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  assert(token[n-1] == '\0');
  for (len = 0; len < n - 1; len++) {
    if (token[len] == '\n') {
      break;
    }
    if ((token[len] == '\r') && ((len < (n - 2)) && (token[len + 1] != '\n'))) {
      break;
    }
  }
  /* remove trailing whitespace */
  while (len > 0 && IS_SPACE(token[len - 1])) {
    len--;
  }

  return len;
}

/** Calculate the length of a token until a line feed character
 *  @param token Ponteiro para a string a ser processada.
 *  @param n Tamanho da string `token`.
 *  @return Comprimento da string até o próximo caractere de line feed.
*/
static size_t length_until_line_feed(const char *token, size_t n) {
  size_t len = 0;

  /* Assume token[n-1] = '\0' */
  for (len = 0; len < n; len++) {
    if ((token[len] == '\n') || (token[len] == '\r')) {
      break;
    }
  }

  return len;
}

/** Convert a string to an integer
 *  @param c Ponteiro para a string a ser convertida.
 *  @return Valor inteiro convertido.
 *  @note Esta função é baseada em http://stackoverflow.com/questions/5710091/how-does-atoi-function-in-c-work
 */
static int my_atoi(const char *c) {
  int value = 0;
  int sign = 1;
  if (*c == '+' || *c == '-') {
    if (*c == '-') sign = -1;
    c++;
  }
  while (((*c) >= '0') && ((*c) <= '9')) { /* isdigit(*c) */
    value *= 10;
    value += (int)(*c - '0');
    c++;
  }
  return value * sign;
}

/** Make index zero-base, and also support relative index.
 *  @param idx Índice a ser ajustado.
 *  @param n Tamanho do array.
 *  @return Índice ajustado.
 */
static int fixIndex(int idx, size_t n) {
  if (idx > 0) return idx - 1;
  if (idx == 0) return 0;
  return (int)n + idx; /* negative value = relative */
}

/** Parse a raw triple string (e.g., "1/2/3") into a Tinyobj_vertex_index structure
 *  @param token Ponteiro para o ponteiro de string a ser processado. 
 *  @return Estrutura Tinyobj_vertex_index contendo os índices de vértice, coordenada de textura e normal.
 *  @note O ponteiro apontado por `token` será avançado para o próximo caractere após o triple.
 */
static Tinyobj_vertex_index parseRawTriple(const char **token) {
  Tinyobj_vertex_index vi;
  /* 0x80000000 = -2147483648 = invalid */
  vi.v_idx = (int)(0x80000000);
  vi.vn_idx = (int)(0x80000000);
  vi.vt_idx = (int)(0x80000000);

  vi.v_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }
  (*token)++;

  /* i//k */
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = my_atoi((*token));
    while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
           (*token)[0] != '\t' && (*token)[0] != '\r') {
      (*token)++;
    }
    return vi;
  }

  /* i/j/k or i/j */
  vi.vt_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  if ((*token)[0] != '/') {
    return vi;
  }

  /* i/j/k */
  (*token)++; /* skip '/' */
  vi.vn_idx = my_atoi((*token));
  while ((*token)[0] != '\0' && (*token)[0] != '/' && (*token)[0] != ' ' &&
         (*token)[0] != '\t' && (*token)[0] != '\r') {
    (*token)++;
  }
  return vi;
}

/** Parse an integer from a token
 *  @param token Ponteiro para o ponteiro de string a ser processado.
 *  @return Valor inteiro convertido.
 *  @note O ponteiro apontado por `token` será avançado para o próximo caractere após o inteiro.
 */
static int parseInt(const char **token) {
  int i = 0;
  skip_space(token);
  i = my_atoi((*token));
  (*token) += until_space((*token));
  return i;
}

/** Parse a double from a token
 *  @param s Ponteiro para o ponteiro de string a ser processado.
 *  @param s_end Ponteiro para o final da string a ser processada, usado para evitar leitura além do limite.
 *  @param result Ponteiro para a variável onde o valor double convertido será armazenado.
 *  @return Valor double convertido.
 *  @note O ponteiro apontado por `s` será avançado para o próximo caractere após o double.
 */
static int tryParseDouble(const char *s, const char *s_end, double *result) {
  double mantissa = 0.0;
  /* This exponent is base 2 rather than 10.
   * However the exponent we parse is supposed to be one of ten,
   * thus we must take care to convert the exponent/and or the
   * mantissa to a * 2^E, where a is the mantissa and E is the
   * exponent.
   * To get the final double we will use ldexp, it requires the
   * exponent to be in base 2.
   */
  int exponent = 0;

  /* NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
   * TO JUMP OVER DEFINITIONS.
   */
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  /* How many characters were read in a loop. */
  int read = 0;
  /* Tells whether a loop terminated due to reaching s_end. */
  int end_not_reached = 0;

  /*
     BEGIN PARSING.
     */

  if (s >= s_end) {
    return 0; /* fail */
  }

  /* Find out what sign we've got. */
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
  } else if (IS_DIGIT(*curr)) { /* Pass through. */
  } else {
    goto fail;
  }

  /* Read the integer part. */
  end_not_reached = (curr != s_end);
  while (end_not_reached && IS_DIGIT(*curr)) {
    mantissa *= 10;
    mantissa += (int)(*curr - 0x30);
    curr++;
    read++;
    end_not_reached = (curr != s_end);
  }

  /* We must make sure we actually got something. */
  if (read == 0) goto fail;
  /* We allow numbers of form "#", "###" etc. */
  if (!end_not_reached) goto assemble;

  /* Read the decimal part. */
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      /* pow(10.0, -read) */
      double frac_value = 1.0;
      int f;
      for (f = 0; f < read; f++) {
        frac_value *= 0.1;
      }
      mantissa += (int)(*curr - 0x30) * frac_value;
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  /* Read the exponent part. */
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    /* Figure out if a sign is present and if it is. */
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) { /* Pass through. */
    } else {
      /* Empty E is not allowed. */
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      exponent *= 10;
      exponent += (int)(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    if (read == 0) goto fail;
  }

assemble :

  {
    double a = 1.0; /* = pow(5.0, exponent); */
    double b  = 1.0; /* = 2.0^exponent */
    int i;
    for (i = 0; i < exponent; i++) {
      a = a * 5.0;
    }

    for (i = 0; i < exponent; i++) {
      b = b * 2.0;
    }

    if (exp_sign == '-') {
      a = 1.0 / a;
      b = 1.0 / b;
    }

    *result =
      /* (sign == '+' ? 1 : -1) * ldexp(mantissa * pow(5.0, exponent),
         exponent); */
      (sign == '+' ? 1 : -1) * (mantissa * a * b);
  }

  return 1;
fail:
  return 0;
}

/** Parse a float from a token
 *  @param token Ponteiro para o ponteiro de string a ser processado.
 *  @return Valor float convertido.
 *  @note O ponteiro apontado por `token` será avançado para o próximo caractere após o float.
 */
static float parseFloat(const char **token) {
  const char *end;
  double val = 0.0;
  float f = 0.0f;
  skip_space(token);
  end = (*token) + until_space((*token));
  val = 0.0;
  tryParseDouble((*token), end, &val);
  f = (float)(val);
  (*token) = end;
  return f;
}

/** Parse two floats from a token
 *  @param x Ponteiro para a variável onde o primeiro float convertido será armazenado.
 *  @param y Ponteiro para a variável onde o segundo float convertido será armazenado.
 *  @param token Ponteiro para o ponteiro de string a ser processado.
 *  @note O ponteiro apontado por `token` será avançado para o próximo caractere após os dois floats.
 */
static void parseFloat2(float *x, float *y, const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
}

/** Parse three floats from a token
 *  @param x Ponteiro para a variável onde o primeiro float convertido será armazenado.
 *  @param y Ponteiro para a variável onde o segundo float convertido será armazenado.
 *  @param z Ponteiro para a variável onde o terceiro float convertido será armazenado.
 *  @param token Ponteiro para o ponteiro de string a ser processado.
 *  @note O ponteiro apontado por `token` será avançado para o próximo caractere após os três floats.
 */
static void parseFloat3(float *x, float *y, float *z, const char **token) {
  (*x) = parseFloat(token);
  (*y) = parseFloat(token);
  (*z) = parseFloat(token);
}

/** Calculate the length of a string up to a maximum length, stopping at the first null terminator
 *  @param s Ponteiro para a string a ser processada.
 *  @param n Tamanho máximo da string a ser processada.
 *  @return Comprimento da string até o primeiro null terminator ou até `n`, o que ocorrer primeiro.
*/
static size_t my_strnlen(const char *s, size_t n) {
    const char *p = (char *)memchr(s, 0, n);
    return p ? (size_t)(p - s) : n;
}

/** Duplicate a string up to a maximum length, trimming line endings
 *  @param s Ponteiro para a string a ser duplicada.
 *  @param max_length Tamanho máximo da string a ser duplicada.
 *  @return Ponteiro para a nova string duplicada, ou NULL em caso de erro.
 *  @note O ponteiro retornado deve ser liberado usando TINYOBJ_FREE quando não for mais necessário.
*/
static char *my_strdup(const char *s, size_t max_length) {
  char *d;
  size_t len;

  if (s == NULL) return NULL;

  /* Do not consider CRLF line ending(#19) */
  len = length_until_line_feed(s, max_length);
  /* len = strlen(s); */

  /* trim line ending and append '\0' */
  d = (char *)TINYOBJ_MALLOC(len + 1); /* + '\0' */
  memcpy(d, s, (size_t)(len));
  d[len] = '\0';

  return d;
}

/** Duplicate a string up to a maximum length, trimming line endings
 *  @param s Ponteiro para a string a ser duplicada.
 *  @param max_length Tamanho máximo da string a ser duplicada.
 *  @return Ponteiro para a nova string duplicada, ou NULL em caso de erro.
 *  @note O ponteiro retornado deve ser liberado usando TINYOBJ_FREE quando não for mais necessário.
*/
static char *my_strndup(const char *s, size_t len) {
  char *d;
  size_t slen;

  if (s == NULL) return NULL;
  if (len == 0) return NULL;

  slen = my_strnlen(s, len);
  d = (char *)TINYOBJ_MALLOC(slen + 1); /* + '\0' */
  if (!d) {
    return NULL;
  }
  memcpy(d, s, slen);
  d[slen] = '\0';

  return d;
}

/** Read a line from a file, dynamically resizing the buffer as needed
 *  @param buf Ponteiro para o buffer onde a linha lida será armazenada. O buffer será realocado conforme necessário.
 *  @param size Ponteiro para o tamanho do buffer. O valor apontado por `size` será atualizado para refletir o novo tamanho do buffer após realocação.
 *  @param file Ponteiro para o arquivo de onde a linha será lida.
 *  @return Ponteiro para a linha lida, ou NULL em caso de erro ou se o final do arquivo for alcançado.
 *  @note O ponteiro retornado é o mesmo que `*buf`, e deve ser liberado usando TINYOBJ_FREE quando não for mais necessário.
*/
static char *dynamic_fgets(char **buf, size_t *size, FILE *file) {
  char *offset;
  char *ret;
  size_t old_size;

  if (!(ret = fgets(*buf, (int)*size, file))) {
    return ret;
  }

  if (NULL != strchr(*buf, '\n')) {
    return ret;
  }

  do {
    old_size = *size;
    *size *= 2;
    *buf = (char*)TINYOBJ_REALLOC_SIZED(*buf, old_size, *size);
    offset = &((*buf)[old_size - 1]);

    ret = fgets(offset, (int)(old_size + 1), file);
  } while(ret && (NULL == strchr(*buf, '\n')));

  return ret;
}

/** Initialize a Tinyobj_material structure with default values
 *  @param material Ponteiro para a estrutura `Tinyobj_material` a ser inicializada.
 *  @note Esta função define os campos da estrutura `Tinyobj_material` para valores padrão, como NULL para strings e 0 ou 1 para os campos numéricos.
*/
static void initMaterial(Tinyobj_material *material) {
  int i;
  material->name = NULL;
  material->ambient_texname = NULL;
  material->diffuse_texname = NULL;
  material->specular_texname = NULL;
  material->specular_highlight_texname = NULL;
  material->bump_texname = NULL;
  material->displacement_texname = NULL;
  material->alpha_texname = NULL;
  for (i = 0; i < 3; i++) {
    material->ambient[i] = 0.f;
    material->diffuse[i] = 0.f;
    material->specular[i] = 0.f;
    material->transmittance[i] = 0.f;
    material->emission[i] = 0.f;
  }
  material->illum = 0;
  material->dissolve = 1.f;
  material->shininess = 1.f;
  material->ior = 1.f;
}

/* Implementation of string to int hashtable */

/** Error code indicating a hash table operation failed */
#define HASH_TABLE_ERROR 1
/** Success code indicating a hash table operation succeeded */
#define HASH_TABLE_SUCCESS 0

/** Default size for a hash table */
#define HASH_TABLE_DEFAULT_SIZE 10

/** Estrutura de entrada da tabela hash 
 *  @param hash Valor hash da chave
 *  @param filled Indica se a entrada está preenchida (1) ou vazia (0)
 *  @param pad0 Padding para alinhamento de 4 bytes
 *  @param value Valor associado à chave hash
 *  @param next Ponteiro para a próxima entrada na cadeia de colisão (usado para lidar com colisões de hash)
*/
typedef struct HASH_TABLE_ENTRY {
  unsigned long hash; // Valor hash da chave
  int filled;         // Indica se a entrada está preenchida (1) ou vazia (0)
  int pad0;           // Padding para alinhamento de 4 bytes
  long value;         // Valor associado à chave hash

  struct HASH_TABLE_ENTRY* next; // Ponteiro para a próxima entrada na cadeia de colisão (usado para lidar com colisões de hash)
} HASH_TABLE_ENTRY;

/** Estrutura da tabela hash
 *  @param hashes Array de valores hash
 *  @param entries Array de entradas da tabela hash
 *  @param capacity Capacidade da tabela hash
 *  @param n Número de entradas na tabela hash
*/
typedef struct HASH_TABLE {
  unsigned long* hashes;      // Array de valores hash
  HASH_TABLE_ENTRY* entries;  // Array de entradas da tabela hash
  size_t capacity;            // Capacidade da tabela hash
  size_t n;                   // Número de entradas na tabela hash
} HASH_TABLE;

/** Função de hash DJB2
 *  @param str String de entrada
 *  @return Valor hash calculado
 */
static unsigned long hash_djb2(const unsigned char* str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + (unsigned long)(c);
  }

  return hash;
}

/** Cria uma tabela hash
 *  @param start_capacity Capacidade inicial da tabela hash
 *  @param hash_table Ponteiro para a tabela hash a ser criada
 */
static void create_hash_table(size_t start_capacity, HASH_TABLE* hash_table) {
  if (start_capacity < 1)
    start_capacity = HASH_TABLE_DEFAULT_SIZE;

  hash_table->hashes = (unsigned long*) TINYOBJ_MALLOC(start_capacity * sizeof(unsigned long));
  hash_table->entries = (HASH_TABLE_ENTRY*) TINYOBJ_CALLOC(start_capacity, sizeof(HASH_TABLE_ENTRY));
  hash_table->capacity = start_capacity;
  hash_table->n = 0;
}

/** Destroi uma tabela hash
 *  @param hash_table Ponteiro para a tabela hash a ser destruída
 */
static void destroy_hash_table(HASH_TABLE* hash_table) {
  TINYOBJ_FREE(hash_table->entries);
  TINYOBJ_FREE(hash_table->hashes);
}

/** Insere um valor na tabela hash
 *  @param hash Valor hash da chave
 *  @param value Valor a ser inserido
 *  @param hash_table Ponteiro para a tabela hash
 *  @return Código de sucesso ou erro
 */
static int hash_table_insert_value(unsigned long hash, long value, HASH_TABLE* hash_table) {
  /* Insert value */
  size_t start_index = hash % hash_table->capacity;
  size_t index = start_index;
  HASH_TABLE_ENTRY* start_entry = hash_table->entries + start_index;
  size_t i;
  HASH_TABLE_ENTRY* entry;

  for (i = 1; hash_table->entries[index].filled; i++) {
    if (i >= hash_table->capacity)
      return HASH_TABLE_ERROR;
    index = (start_index + (i * i)) % hash_table->capacity;
  }

  entry = hash_table->entries + index;
  entry->hash = hash;
  entry->filled = 1;
  entry->value = value;

  if (index != start_index) {
    /* This is a new entry, but not the start entry, hence we need to add a next pointer to our entry */
    entry->next = start_entry->next;
    start_entry->next = entry;
  }

  return HASH_TABLE_SUCCESS;
}

/** Insere um valor na tabela hash
 *  @param hash Valor hash da chave
 *  @param value Valor a ser inserido
 *  @param hash_table Ponteiro para a tabela hash
 *  @return Código de sucesso ou erro
 */
static int hash_table_insert(unsigned long hash, long value, HASH_TABLE* hash_table) {
  int ret = hash_table_insert_value(hash, value, hash_table);

  if (ret == HASH_TABLE_SUCCESS) {
    hash_table->hashes[hash_table->n] = hash;
    hash_table->n++;
  }
  return ret;
}

/** Encontra uma entrada na tabela hash
 *  @param hash Valor hash da chave
 *  @param hash_table Ponteiro para a tabela hash
 *  @return Ponteiro para a entrada encontrada ou NULL se não encontrada
 */
static HASH_TABLE_ENTRY* hash_table_find(unsigned long hash, HASH_TABLE* hash_table) {
  HASH_TABLE_ENTRY* entry = hash_table->entries + (hash % hash_table->capacity);

  while (entry) {
    if (entry->hash == hash && entry->filled) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

/** Cresce a tabela hash
 *  @param hash_table Ponteiro para a tabela hash a ser crescida
 */
static void hash_table_grow(HASH_TABLE* hash_table) {
  size_t new_capacity;        // Nova capacidade da tabela hash
  HASH_TABLE new_hash_table;  // Nova tabela hash para armazenar os dados rehashados
  size_t i;                   // Índice para iterar sobre as entradas da tabela hash

  new_capacity = 2 * hash_table->capacity;

  /* Create a new hash table. We're not calling create_hash_table because we want to realloc the hash array */
  new_hash_table.hashes = hash_table->hashes = (unsigned long*) TINYOBJ_REALLOC_SIZED(
      (void*) hash_table->hashes, sizeof(unsigned long) * hash_table->capacity, sizeof(unsigned long) * new_capacity);

  new_hash_table.entries = (HASH_TABLE_ENTRY*) TINYOBJ_CALLOC(new_capacity, sizeof(HASH_TABLE_ENTRY));
  new_hash_table.capacity = new_capacity;
  new_hash_table.n = hash_table->n;

  /* Rehash */
  for (i = 0; i < hash_table->capacity; i++) {
    HASH_TABLE_ENTRY* entry = &hash_table->entries[i];
    if (entry->filled) {
            hash_table_insert_value(entry->hash, entry->value, &new_hash_table);
    }
  }

  TINYOBJ_FREE(hash_table->entries);
  (*hash_table) = new_hash_table;
}

/** Verifica se uma chave existe na tabela hash
 *  @param name Nome da chave a ser verificada
 *  @param hash_table Ponteiro para a tabela hash
 *  @return 1 se a chave existir, 0 caso contrário
 */
static int hash_table_exists(const char* name, HASH_TABLE* hash_table) {
  return hash_table_find(hash_djb2((const unsigned char*)name), hash_table) != NULL;
}

/** Define um valor para uma chave na tabela hash
 *  @param name Nome da chave a ser definida
 *  @param val Valor a ser associado à chave
 *  @param hash_table Ponteiro para a tabela hash
 */
static void hash_table_set(const char* name, size_t val, HASH_TABLE* hash_table) {
  /* Hash name */
  unsigned long hash = hash_djb2((const unsigned char *)name);

  HASH_TABLE_ENTRY* entry = hash_table_find(hash, hash_table);
  if (entry) {
    entry->value = (long)val;
    return;
  }

  /* Expand if necessary
   * Grow until the element has been added
   */
  while (hash_table_insert(hash, (long)val, hash_table) != HASH_TABLE_SUCCESS) {
    hash_table_grow(hash_table);
  }
}

/** Obtém o valor associado a uma chave na tabela hash
 *  @param name Nome da chave a ser obtida
 *  @param hash_table Ponteiro para a tabela hash
 *  @return Valor associado à chave, ou 0 se a chave não existir
 */
static long hash_table_get(const char* name, HASH_TABLE* hash_table) {
  HASH_TABLE_ENTRY* ret = hash_table_find(hash_djb2((const unsigned char*)(name)), hash_table);
  return ret->value;
}

/** Adiciona um novo material à lista de materiais
 *  @param prev Ponteiro para a lista de materiais existente
 *  @param num_materials Número de materiais existentes
 *  @param new_mat Ponteiro para o novo material a ser adicionado
 *  @return Ponteiro para a lista de materiais atualizada
 */
static Tinyobj_material *Tinyobj_material_add(Tinyobj_material *prev,
                                                size_t num_materials,
                                                Tinyobj_material *new_mat) {
  Tinyobj_material *dst;
  size_t num_bytes = sizeof(Tinyobj_material) * num_materials;
  dst = (Tinyobj_material *)TINYOBJ_REALLOC_SIZED(
                                      prev, num_bytes, num_bytes + sizeof(Tinyobj_material));

  dst[num_materials] = (*new_mat); /* Just copy pointer for char* members */
  return dst;
}

/** Verifica se o caractere na posição `i` na string `p` é um caractere de final de linha, considerando o índice final `end_i` para evitar acesso fora dos limites.
 *  @param p Ponteiro para a string a ser verificada.
 *  @param i Índice do caractere a ser verificado.
 *  @param end_i Índice do final da string para evitar acesso fora dos limites.
 *  @return 1 se o caractere for um terminador de linha, 0 caso contrário.
 */
static int is_line_ending(const char *p, size_t i, size_t end_i) {
  if (p[i] == '\0') return 1;
  if (p[i] == '\n') return 1; /* this includes \r\n */
  if (p[i] == '\r') {
    if (((i + 1) < end_i) && (p[i + 1] != '\n')) { /* detect only \r case */
      return 1;
    }
  }
  return 0;
}

/** Estrutura para armazenar informações de linha, incluindo a posição inicial e o comprimento da linha.
 *  @param pos Posição inicial da linha no buffer.
 *  @param len Comprimento da linha.
*/
typedef struct LINE_INFO {
  size_t pos;   // Posição inicial da linha no buffer
  size_t len;   // Comprimento da linha
} LINE_INFO;

/** Analisa um buffer de texto para extrair informações sobre as linhas, incluindo a posição inicial e o comprimento de cada linha.
 *  @param buf Ponteiro para o buffer de texto a ser analisado.
 *  @param buf_len Tamanho do buffer de texto.
 *  @param line_infos Ponteiro para um array de LINE_INFO onde as informações das linhas serão armazenadas. O array será alocado dinamicamente dentro da função.
 *  @param num_lines Ponteiro para uma variável onde o número de linhas encontradas será armazenado.
 *  @return Código de erro indicando o resultado da operação (0 para sucesso, TINYOBJ_ERROR_EMPTY se o buffer estiver vazio, ou outro código de erro em caso de falha).
 *  @note O array `line_infos` deve ser liberado usando TINYOBJ_FREE quando não for mais necessário.
*/
static int get_line_infos(const char *buf, size_t buf_len, LINE_INFO **line_infos, size_t *num_lines) {
  size_t i = 0;
  size_t end_idx = buf_len;
  size_t prev_pos = 0;
  size_t line_no = 0;
  size_t last_line_ending = 0;

  /* Count # of lines. */
  for (i = 0; i < end_idx; i++) {
    if (is_line_ending(buf, i, end_idx)) {
      (*num_lines)++;
      last_line_ending = i;
    }
  }
  /* The last char from the input may not be a line
    * ending character so add an extra line if there
    * are more characters after the last line ending
    * that was found. */
  if (end_idx - last_line_ending > 1) {
      (*num_lines)++;
  }

  if (*num_lines == 0) return TINYOBJ_ERROR_EMPTY;

  *line_infos = (LINE_INFO *)TINYOBJ_MALLOC(sizeof(LINE_INFO) * (*num_lines));

  /* Fill line infos. */
  for (i = 0; i < end_idx; i++) {
    if (is_line_ending(buf, i, end_idx)) {
      (*line_infos)[line_no].pos = prev_pos;
      (*line_infos)[line_no].len = i - prev_pos;
      prev_pos = i + 1;
      line_no++;
    }
  }
  if (end_idx - last_line_ending > 1) {
    (*line_infos)[line_no].pos = prev_pos;
    (*line_infos)[line_no].len = end_idx - 1 - last_line_ending;
  }

  return 0;
}

/** Carrega um arquivo MTL e indexa os materiais encontrados, armazenando-os em uma tabela hash para acesso rápido.
 *  @param materials_out Ponteiro para um array de Tinyobj_material onde os materiais carregados serão armazenados. O array será alocado dinamicamente dentro da função.
 *  @param num_materials_out Ponteiro para uma variável onde o número de materiais carregados será armazenado.
 *  @param mtl_filename Nome do arquivo MTL a ser carregado.
 *  @param obj_filename Nome do arquivo OBJ associado ao MTL, usado para resolver caminhos relativos.
 *  @param file_reader Função de callback para ler arquivos, que deve ser fornecida pelo usuário.
 *  @param ctx Contexto a ser passado para a função de leitura de arquivos.
 *  @param material_table Ponteiro para uma tabela hash onde os materiais serão indexados por nome.
 *  @return Código de erro indicando o resultado da operação (0 para sucesso, ou outro código de erro em caso de falha).
 *  @note O array `materials_out` deve ser liberado usando TINYOBJ_FREE quando não for mais necessário. Os materiais individuais dentro do array também devem ser liberados conforme necessário.
*/
static int tinyobj_load_and_index_mtl_file(Tinyobj_material **materials_out,
                                            size_t *num_materials_out,
                                            const char *mtl_filename, const char *obj_filename, File_reader_callback file_reader, void *ctx,
                                            HASH_TABLE* material_table) {
  Tinyobj_material material;
  size_t num_materials = 0;
  Tinyobj_material *materials = NULL;
  int has_previous_material = 0;
  const char *line_end = NULL;
  size_t num_lines = 0;
  LINE_INFO *line_infos = NULL;
  size_t i = 0;
  char *buf = NULL;
  size_t len = 0;

  if (materials_out == NULL) {
    return TINYOBJ_ERROR_INVALID_PARAMETER;
  }

  if (num_materials_out == NULL) {
    return TINYOBJ_ERROR_INVALID_PARAMETER;
  }

  (*materials_out) = NULL;
  (*num_materials_out) = 0;

  file_reader(ctx, mtl_filename, 1, obj_filename, &buf, &len);
  if (len < 1) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (buf == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;

  if (get_line_infos(buf, len, &line_infos, &num_lines) != 0) {
		TINYOBJ_FREE(line_infos);
    return TINYOBJ_ERROR_EMPTY;
  }

  /* Create a default material */
  initMaterial(&material);

  for (i = 0; i < num_lines; i++) {
    const char *p = &buf[line_infos[i].pos];
    size_t p_len = line_infos[i].len;

    char linebuf[4096];
    const char *token;
    assert(p_len < 4095);

    memcpy(linebuf, p, p_len);
    linebuf[p_len] = '\0';

    token = linebuf;
    line_end = token + p_len;

    /* Skip leading space. */
    token += strspn(token, " \t");

    assert(token);
    if (token[0] == '\0') continue; /* empty line */

    if (token[0] == '#') continue; /* comment line */

    /* new mtl */
    if ((0 == strncmp(token, "newmtl", 6)) && IS_SPACE((token[6]))) {
      char namebuf[4096];

      /* flush previous material. */
      if (has_previous_material) {
        materials = Tinyobj_material_add(materials, num_materials, &material);
        num_materials++;
      } else {
        has_previous_material = 1;
      }

      /* initial temporary material */
      initMaterial(&material);

      /* set new mtl name */
      token += 7;
#ifdef _MSC_VER
      sscanf_s(token, "%s", namebuf, (unsigned)_countof(namebuf));
#else
      sscanf(token, "%s", namebuf);
#endif
      material.name = my_strdup(namebuf, (size_t) (line_end - token));

      /* Add material to material table */
      if (material_table)
        hash_table_set(material.name, num_materials, material_table);

      continue;
    }

    /* ambient */
    if (token[0] == 'K' && token[1] == 'a' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.ambient[0] = r;
      material.ambient[1] = g;
      material.ambient[2] = b;
      continue;
    }

    /* diffuse */
    if (token[0] == 'K' && token[1] == 'd' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.diffuse[0] = r;
      material.diffuse[1] = g;
      material.diffuse[2] = b;
      continue;
    }

    /* specular */
    if (token[0] == 'K' && token[1] == 's' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.specular[0] = r;
      material.specular[1] = g;
      material.specular[2] = b;
      continue;
    }

    /* transmittance */
    if (token[0] == 'K' && token[1] == 't' && IS_SPACE((token[2]))) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.transmittance[0] = r;
      material.transmittance[1] = g;
      material.transmittance[2] = b;
      continue;
    }

    /* ior(index of refraction) */
    if (token[0] == 'N' && token[1] == 'i' && IS_SPACE((token[2]))) {
      token += 2;
      material.ior = parseFloat(&token);
      continue;
    }

    /* emission */
    if (token[0] == 'K' && token[1] == 'e' && IS_SPACE(token[2])) {
      float r, g, b;
      token += 2;
      parseFloat3(&r, &g, &b, &token);
      material.emission[0] = r;
      material.emission[1] = g;
      material.emission[2] = b;
      continue;
    }

    /* shininess */
    if (token[0] == 'N' && token[1] == 's' && IS_SPACE(token[2])) {
      token += 2;
      material.shininess = parseFloat(&token);
      continue;
    }

    /* illum model */
    if (0 == strncmp(token, "illum", 5) && IS_SPACE(token[5])) {
      token += 6;
      material.illum = parseInt(&token);
      continue;
    }

    /* dissolve */
    if ((token[0] == 'd' && IS_SPACE(token[1]))) {
      token += 1;
      material.dissolve = parseFloat(&token);
      continue;
    }
    if (token[0] == 'T' && token[1] == 'r' && IS_SPACE(token[2])) {
      token += 2;
      /* Invert value of Tr(assume Tr is in range [0, 1]) */
      material.dissolve = 1.0f - parseFloat(&token);
      continue;
    }

    /* ambient texture */
    if ((0 == strncmp(token, "map_Ka", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.ambient_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* diffuse texture */
    if ((0 == strncmp(token, "map_Kd", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.diffuse_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* specular texture */
    if ((0 == strncmp(token, "map_Ks", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* specular highlight texture */
    if ((0 == strncmp(token, "map_Ns", 6)) && IS_SPACE(token[6])) {
      token += 7;
      material.specular_highlight_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* bump texture */
    if ((0 == strncmp(token, "map_bump", 8)) && IS_SPACE(token[8])) {
      token += 9;
      material.bump_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* alpha texture */
    if ((0 == strncmp(token, "map_d", 5)) && IS_SPACE(token[5])) {
      token += 6;
      material.alpha_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* bump texture */
    if ((0 == strncmp(token, "bump", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.bump_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* displacement texture */
    if ((0 == strncmp(token, "disp", 4)) && IS_SPACE(token[4])) {
      token += 5;
      material.displacement_texname = my_strdup(token, (size_t) (line_end - token));
      continue;
    }

    /* @todo { unknown parameter } */
  }

	TINYOBJ_FREE(line_infos);

  if (material.name) {
    /* Flush last material element */
    materials = Tinyobj_material_add(materials, num_materials, &material);
    num_materials++;
  }

  (*num_materials_out) = num_materials;
  (*materials_out) = materials;

  return TINYOBJ_SUCCESS;
}

/** Carrega um arquivo MTL e indexa os materiais encontrados, armazenando-os em uma tabela hash para acesso rápido.
 *  @param materials_out Ponteiro para um array de Tinyobj_material onde os materiais carregados serão armazenados. O array será alocado dinamicamente dentro da função.
 *  @param num_materials_out Ponteiro para uma variável onde o número de materiais carregados será armazenado.
 *  @param mtl_filename Nome do arquivo MTL a ser carregado.
 *  @param obj_filename Nome do arquivo OBJ associado ao MTL, usado para resolver caminhos relativos.
 *  @param file_reader Função de callback para ler arquivos, que deve ser fornecida pelo usuário.
 *  @param ctx Contexto a ser passado para a função de leitura de arquivos.
 *  @return Código de erro indicando o resultado da operação (0 para sucesso, ou outro código de erro em caso de falha).
 *  @note O array `materials_out` deve ser liberado usando TINYOBJ_FREE quando não for mais necessário. Os materiais individuais dentro do array também devem ser liberados conforme necessário.
*/
int tinyobj_load_mtl(Tinyobj_material **materials_out,
                           size_t *num_materials_out,
                           const char *mtl_filename, const char *obj_filename, File_reader_callback file_reader,
                           void *ctx) {
  return tinyobj_load_and_index_mtl_file(materials_out, num_materials_out, mtl_filename, obj_filename, file_reader, ctx, NULL);
}

/** Command types para representar os diferentes tipos de comandos em um arquivo OBJ */
typedef enum COMAND_TYPE {
  COMMAND_EMPTY,    // Linha vazia ou comentário
  COMMAND_V,        // Vertex
  COMMAND_VN,       // Normal
  COMMAND_VT,       // Texture coordinate
  COMMAND_F,        // Face
  COMMAND_G,        // Group name
  COMMAND_O,        // Object name
  COMMAND_USEMTL,   // Use material
  COMMAND_MTLLIB    // Material library 
} COMAND_TYPE;

/** Estrutura para representar um comando em um arquivo OBJ */
typedef struct COMAND {
  float vx, vy, vz;   // Coordenadas do vértice
  float nx, ny, nz;   // Coordenadas da normal
  float tx, ty;       // Coordenadas da textura

  /* @todo { Use dynamic array } */
  Tinyobj_vertex_index f[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f;   // Número de vértices na face (para comandos de face)

  int f_num_verts[TINYOBJ_MAX_FACES_PER_F_LINE];
  size_t num_f_num_verts;   // Número de elementos em f_num_verts (para comandos de face)

  const char *group_name;       // Nome do grupo
  unsigned int group_name_len;  // Comprimento do nome do grupo
  int pad0;                     // Padding para alinhamento de 4 bytes

  const char *object_name;      // Nome do objeto
  unsigned int object_name_len; // Comprimento do nome do objeto
  int pad1;                     // Padding para alinhamento de 4 bytes

  const char *material_name;      // Nome do material
  unsigned int material_name_len; // Comprimento do nome do material
  int pad2;                       // Padding para alinhamento de 4 bytes

  const char *mtllib_name;        // Nome do arquivo de biblioteca de materiais
  unsigned int mtllib_name_len;   // Comprimento do nome do arquivo de biblioteca de materiais
  int pad3;                       // Padding para alinhamento de 4 bytes

  COMAND_TYPE type; // Tipo do comando
} COMAND;

/** Analisa uma linha de um arquivo OBJ e preenche uma estrutura COMAND com as informações extraídas.
 *  @param command Ponteiro para a estrutura COMAND onde as informações extraídas serão armazenadas.
 *  @param p Ponteiro para a linha a ser analisada.
 *  @param p_len Comprimento da linha a ser analisada.
 *  @param triangulate Indica se as faces devem ser trianguladas (1 para sim, 0 para não).
 *  @return 1 se a linha foi analisada com sucesso e um comando válido foi preenchido, ou 0 caso contrário.
 */
static int parseLine(COMAND *command, const char *p, size_t p_len,
                     int triangulate) {
  char linebuf[4096];
  const char *token;
  assert(p_len < 4095);

  memcpy(linebuf, p, p_len);
  linebuf[p_len] = '\0';

  token = linebuf;

  command->type = COMMAND_EMPTY;

  /* Skip leading space. */
  skip_space(&token);

  assert(token);
  if (token[0] == '\0') { /* empty line */
    return 0;
  }

  if (token[0] == '#') { /* comment line */
    return 0;
  }

  /* vertex */
  if (token[0] == 'v' && IS_SPACE((token[1]))) {
    float x, y, z;
    token += 2;
    parseFloat3(&x, &y, &z, &token);
    command->vx = x;
    command->vy = y;
    command->vz = z;
    command->type = COMMAND_V;
    return 1;
  }

  /* normal */
  if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
    float x, y, z;
    token += 3;
    parseFloat3(&x, &y, &z, &token);
    command->nx = x;
    command->ny = y;
    command->nz = z;
    command->type = COMMAND_VN;
    return 1;
  }

  /* texcoord */
  if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
    float x, y;
    token += 3;
    parseFloat2(&x, &y, &token);
    command->tx = x;
    command->ty = y;
    command->type = COMMAND_VT;
    return 1;
  }

  /* line */
  if (token[0] == 'l' && IS_SPACE((token[1]))) {
    size_t num_f = 0;
    
    Tinyobj_vertex_index f[2];
    token += 2;
    skip_space(&token);

    while (!IS_NEW_LINE(token[0])) {
      Tinyobj_vertex_index vi = parseRawTriple(&token);
      skip_space_and_cr(&token);

      f[num_f] = vi;
      num_f++;
    }

    assert(num_f == 2);
    command->type = COMMAND_F;
    command->f[0] = f[0];
    command->f[1] = f[1];
    command->num_f = 2;
    command->f_num_verts[0] = (int)num_f;
    command->num_f_num_verts = 1;

    return 1;
  }

  /* face */
  if (token[0] == 'f' && IS_SPACE((token[1]))) {
    size_t num_f = 0;

    Tinyobj_vertex_index f[TINYOBJ_MAX_FACES_PER_F_LINE];
    token += 2;
    skip_space(&token);

    while (!IS_NEW_LINE(token[0])) {
      Tinyobj_vertex_index vi = parseRawTriple(&token);
      skip_space_and_cr(&token);

      f[num_f] = vi;
      num_f++;
    }

    command->type = COMMAND_F;

    if (triangulate) {
      size_t k;
      size_t n = 0;

      Tinyobj_vertex_index i0 = f[0];
      Tinyobj_vertex_index i1;
      Tinyobj_vertex_index i2 = f[1];

      assert(3 * num_f < TINYOBJ_MAX_FACES_PER_F_LINE);

      for (k = 2; k < num_f; k++) {
        i1 = i2;
        i2 = f[k];
        command->f[3 * n + 0] = i0;
        command->f[3 * n + 1] = i1;
        command->f[3 * n + 2] = i2;

        command->f_num_verts[n] = 3;
        n++;
      }
      command->num_f = 3 * n;
      command->num_f_num_verts = n;

    } else {
      size_t k = 0;
      assert(num_f < TINYOBJ_MAX_FACES_PER_F_LINE);
      for (k = 0; k < num_f; k++) {
        command->f[k] = f[k];
      }

      command->num_f = num_f;
      command->f_num_verts[0] = (int)num_f;
      command->num_f_num_verts = 1;
    }

    return 1;
  }

  /* use mtl */
  if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
    token += 7;

    skip_space(&token);
    command->material_name = p + (token - linebuf);
    command->material_name_len = (unsigned int)length_until_newline(
                                                                    token, (p_len - (size_t)(token - linebuf)) + 1);
    command->type = COMMAND_USEMTL;

    return 1;
  }

  /* load mtl */
  if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
    /* By specification, `mtllib` should be appear only once in .obj */
    token += 7;

    skip_space(&token);
    command->mtllib_name = p + (token - linebuf);
    command->mtllib_name_len = (unsigned int)length_until_newline(
                                                                  token, (p_len - (size_t)(token - linebuf)) + 1);
    command->type = COMMAND_MTLLIB;

    return 1;
  }

  /* group name */
  if (token[0] == 'g' && IS_SPACE((token[1]))) {
    /* @todo { multiple group name. } */
    token += 2;

    command->group_name = p + (token - linebuf);
    command->group_name_len = (unsigned int)length_until_newline(
                                                                 token, (p_len - (size_t)(token - linebuf)) + 1);
    command->type = COMMAND_G;

    return 1;
  }

  /* object name */
  if (token[0] == 'o' && IS_SPACE((token[1]))) {
    /* @todo { multiple object name? } */
    token += 2;

    command->object_name = p + (token - linebuf);
    command->object_name_len = (unsigned int)length_until_newline(
                                                                  token, (p_len - (size_t)(token - linebuf)) + 1);
    command->type = COMMAND_O;

    return 1;
  }

  return 0;
}

static size_t basename_len(const char *filename, size_t filename_length) {
  /* Count includes NUL terminator. */
  const char *p = &filename[filename_length - 1];
  size_t count = 1;

  /* On Windows, the directory delimiter is '\' and both it and '/' is
   * reserved by the filesystem. On *nix platforms, only the '/' character 
   * is reserved, so account for the two cases separately. */
  #if _WIN32
    while (p[-1] != '/' && p[-1] != '\\') {
      if (p == filename) {
        count = filename_length;
        return count;
      }
      count++;
      p--;
    }
    p++;
    return count;
  #else
    while (*(--p) != '/') {
      if (p == filename) {
        count = filename_length;
        return count;
      }
      count++;
    }
    return count;
  #endif
}

static char *generate_mtl_filename(const char *obj_filename,
                                   size_t obj_filename_length,
                                   const char *mtllib_name,
                                   size_t mtllib_name_length) {
  /* Create a dynamically-allocated material filename. This allows the material
   * and obj files to be separated, however the mtllib name in the OBJ file
   * must be a relative path to the material file from the OBJ's directory.
   * This does not support the matllib name as an absolute address. */
  char *mtl_filename;
  char *p;
  size_t mtl_filename_length;
  size_t obj_basename_length;

  /* Calculate required size of mtl_filename and allocate */
  obj_basename_length = basename_len(obj_filename, obj_filename_length);
  mtl_filename_length = (obj_filename_length - obj_basename_length) + mtllib_name_length;
  mtl_filename = (char *)TINYOBJ_MALLOC(mtl_filename_length);

  /* Copy over the obj's path */
  memcpy(mtl_filename, obj_filename, (obj_filename_length - obj_basename_length));

  /* Overwrite the obj basename with the mtllib name, filling the string */
  p = &mtl_filename[mtl_filename_length - mtllib_name_length];
  strcpy(p, mtllib_name);
  return mtl_filename;
}

int tinyobj_load_obj(Tinyobj_attrib *attrib, Tinyobj_shape **shapes,
                      size_t *num_shapes, Tinyobj_material **materials_out,
                      size_t *num_materials_out, const char *obj_filename,
                      File_reader_callback file_reader, void *ctx,
                      unsigned int flags) {
  LINE_INFO *line_infos = NULL;
  COMAND *commands = NULL;
  size_t num_lines = 0;

  size_t num_v = 0;
  size_t num_vn = 0;
  size_t num_vt = 0;
  size_t num_f = 0;
  size_t num_faces = 0;

  int mtllib_line_index = -1;

  Tinyobj_material *materials = NULL;
  size_t num_materials = 0;

  HASH_TABLE material_table;

  char *buf = NULL;
  size_t len = 0;
  file_reader(ctx, obj_filename, /* is_mtl */0, obj_filename, &buf, &len);

  if (len < 1) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (attrib == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (shapes == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (num_shapes == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (buf == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (materials_out == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;
  if (num_materials_out == NULL) return TINYOBJ_ERROR_INVALID_PARAMETER;

  tinyobj_attrib_init(attrib);

  /* 1. create line data */
  if (get_line_infos(buf, len, &line_infos, &num_lines) != 0) {
    return TINYOBJ_ERROR_EMPTY;
  }

  commands = (COMAND *)TINYOBJ_MALLOC(sizeof(COMAND) * num_lines);

  create_hash_table(HASH_TABLE_DEFAULT_SIZE, &material_table);

  /* 2. parse each line */ 
  {
    size_t i = 0;
    for (i = 0; i < num_lines; i++) {
      int ret = parseLine(&commands[i], &buf[line_infos[i].pos],
                          line_infos[i].len, flags & TINYOBJ_FLAG_TRIANGULATE);
      if (ret) {
        if (commands[i].type == COMMAND_V) {
          num_v++;
        } else if (commands[i].type == COMMAND_VN) {
          num_vn++;
        } else if (commands[i].type == COMMAND_VT) {
          num_vt++;
        } else if (commands[i].type == COMMAND_F) {
          num_f += commands[i].num_f;
          num_faces += commands[i].num_f_num_verts;
        }

        if (commands[i].type == COMMAND_MTLLIB) {
          mtllib_line_index = (int)i;
        }
      }
    }
  }

  /* line_infos are not used anymore. Release memory. */
  if (line_infos) {
    TINYOBJ_FREE(line_infos);
  }

  /* Load material (if it exists) */
  if (mtllib_line_index >= 0 && commands[mtllib_line_index].mtllib_name &&
      commands[mtllib_line_index].mtllib_name_len > 0) {
    /* Maximum length allowed by Linux - higher than Windows and macOS */
    size_t obj_filename_len = my_strnlen(obj_filename, 4096 + 255) + 1;
    char *mtl_filename;
    char *mtllib_name;
    size_t mtllib_name_len = 0;
    int ret;

    mtllib_name_len = length_until_line_feed(commands[mtllib_line_index].mtllib_name,
                                             commands[mtllib_line_index].mtllib_name_len);

    mtllib_name = my_strndup(commands[mtllib_line_index].mtllib_name,
                             mtllib_name_len);

    /* allow for NUL terminator */
    mtllib_name_len++;
    mtl_filename = generate_mtl_filename(obj_filename, obj_filename_len,
                                         mtllib_name, mtllib_name_len);

    ret = tinyobj_load_and_index_mtl_file(&materials, &num_materials,
                                           mtl_filename, obj_filename,
                                           file_reader, ctx,
                                           &material_table);

    if (ret != TINYOBJ_SUCCESS) {
      /* warning. */
      fprintf(stderr, "TINYOBJ: Failed to parse material file '%s': %d\n", mtl_filename, ret);
    }
    TINYOBJ_FREE(mtl_filename);
    TINYOBJ_FREE(mtllib_name);
  }

  /* Construct attributes */

  {
    size_t v_count = 0;
    size_t n_count = 0;
    size_t t_count = 0;
    size_t f_count = 0;
    size_t face_count = 0;
    int material_id = -1; /* -1 = default unknown material. */
    size_t i = 0;

    attrib->vertices = (float *)TINYOBJ_MALLOC(sizeof(float) * num_v * 3);
    attrib->num_vertices = (unsigned int)num_v;
    attrib->normals = (float *)TINYOBJ_MALLOC(sizeof(float) * num_vn * 3);
    attrib->num_normals = (unsigned int)num_vn;
    attrib->texcoords = (float *)TINYOBJ_MALLOC(sizeof(float) * num_vt * 2);
    attrib->num_texcoords = (unsigned int)num_vt;
    attrib->faces = (Tinyobj_vertex_index *)TINYOBJ_MALLOC(
                                                     sizeof(Tinyobj_vertex_index) * num_f);
    attrib->num_faces = (unsigned int)num_f;
    attrib->face_num_verts = (int *)TINYOBJ_MALLOC(sizeof(int) * num_faces);
    attrib->material_ids = (int *)TINYOBJ_MALLOC(sizeof(int) * num_faces);
    attrib->num_face_num_verts = (unsigned int)num_faces;

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_EMPTY) {
        continue;
      } else if (commands[i].type == COMMAND_USEMTL) {
        /* @todo
           if (commands[t][i].material_name &&
           commands[t][i].material_name_len > 0) {
           std::string material_name(commands[t][i].material_name,
           commands[t][i].material_name_len);

           if (material_map.find(material_name) != material_map.end()) {
           material_id = material_map[material_name];
           } else {
        // Assign invalid material ID
        material_id = -1;
        }
        }
        */
        if (commands[i].material_name &&
           commands[i].material_name_len >0)
        {
          /* Create a null terminated string */
          char* material_name_null_term = (char*) TINYOBJ_MALLOC(commands[i].material_name_len + 1);
          memcpy((void*) material_name_null_term, (const void*) commands[i].material_name, commands[i].material_name_len);
          material_name_null_term[commands[i].material_name_len] = 0;

          if (hash_table_exists(material_name_null_term, &material_table))
            material_id = (int)hash_table_get(material_name_null_term, &material_table);
          else
            material_id = -1;

          TINYOBJ_FREE(material_name_null_term);
        }
      } else if (commands[i].type == COMMAND_V) {
        attrib->vertices[3 * v_count + 0] = commands[i].vx;
        attrib->vertices[3 * v_count + 1] = commands[i].vy;
        attrib->vertices[3 * v_count + 2] = commands[i].vz;
        v_count++;
      } else if (commands[i].type == COMMAND_VN) {
        attrib->normals[3 * n_count + 0] = commands[i].nx;
        attrib->normals[3 * n_count + 1] = commands[i].ny;
        attrib->normals[3 * n_count + 2] = commands[i].nz;
        n_count++;
      } else if (commands[i].type == COMMAND_VT) {
        attrib->texcoords[2 * t_count + 0] = commands[i].tx;
        attrib->texcoords[2 * t_count + 1] = commands[i].ty;
        t_count++;
      } else if (commands[i].type == COMMAND_F) {
        size_t k = 0;
        for (k = 0; k < commands[i].num_f; k++) {
          Tinyobj_vertex_index vi = commands[i].f[k];
          int v_idx = fixIndex(vi.v_idx, v_count);
          int vn_idx = fixIndex(vi.vn_idx, n_count);
          int vt_idx = fixIndex(vi.vt_idx, t_count);
          attrib->faces[f_count + k].v_idx = v_idx;
          attrib->faces[f_count + k].vn_idx = vn_idx;
          attrib->faces[f_count + k].vt_idx = vt_idx;
        }

        for (k = 0; k < commands[i].num_f_num_verts; k++) {
          attrib->material_ids[face_count + k] = material_id;
          attrib->face_num_verts[face_count + k] = commands[i].f_num_verts[k];
        }

        f_count += commands[i].num_f;
        face_count += commands[i].num_f_num_verts;
      }
    }
  }

  /* 5. Construct shape information. */
  {
    unsigned int face_count = 0;
    size_t i = 0;
    size_t n = 0;
    size_t shape_idx = 0;

    const char *shape_name = NULL;
    unsigned int shape_name_len = 0;
    const char *prev_shape_name = NULL;
    unsigned int prev_shape_name_len = 0;
    unsigned int prev_shape_face_offset = 0;
    unsigned int prev_face_offset = 0;
    Tinyobj_shape prev_shape = {NULL, 0, 0};

    /* Find the number of shapes in .obj */
    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        n++;
      }
    }

    /* Allocate array of shapes with maximum possible size(+1 for unnamed
     * group/object).
     * Actual # of shapes found in .obj is determined in the later */
    (*shapes) = (Tinyobj_shape*)TINYOBJ_MALLOC(sizeof(Tinyobj_shape) * (n + 1));

    for (i = 0; i < num_lines; i++) {
      if (commands[i].type == COMMAND_O || commands[i].type == COMMAND_G) {
        if (commands[i].type == COMMAND_O) {
          shape_name = commands[i].object_name;
          shape_name_len = commands[i].object_name_len;
        } else {
          shape_name = commands[i].group_name;
          shape_name_len = commands[i].group_name_len;
        }

        if (face_count == 0) {
          /* 'o' or 'g' appears before any 'f' */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
          prev_face_offset = face_count;
        } else {
          if (shape_idx == 0) {
            /* 'o' or 'g' after some 'v' lines. */
            (*shapes)[shape_idx].name = my_strndup(
                                                   prev_shape_name, prev_shape_name_len); /* may be NULL */
            (*shapes)[shape_idx].face_offset = prev_shape.face_offset;
            (*shapes)[shape_idx].length = face_count - prev_face_offset;
            shape_idx++;

            prev_face_offset = face_count;

          } else {
            if ((face_count - prev_face_offset) > 0) {
              (*shapes)[shape_idx].name =
                my_strndup(prev_shape_name, prev_shape_name_len);
              (*shapes)[shape_idx].face_offset = prev_face_offset;
              (*shapes)[shape_idx].length = face_count - prev_face_offset;
              shape_idx++;
              prev_face_offset = face_count;
            }
          }

          /* Record shape info for succeeding 'o' or 'g' command. */
          prev_shape_name = shape_name;
          prev_shape_name_len = shape_name_len;
          prev_shape_face_offset = face_count;
        }
      }
      if (commands[i].type == COMMAND_F) {
        face_count++;
      }
    }

    if ((face_count - prev_face_offset) > 0) {
      size_t length = face_count - prev_shape_face_offset;
      if (length > 0) {
        (*shapes)[shape_idx].name =
          my_strndup(prev_shape_name, prev_shape_name_len);
        (*shapes)[shape_idx].face_offset = prev_face_offset;
        (*shapes)[shape_idx].length = face_count - prev_face_offset;
        shape_idx++;
      }
    } else {
      /* Guess no 'v' line occurrence after 'o' or 'g', so discards current
       * shape information. */
    }

    (*num_shapes) = shape_idx;
  }

  if (commands) {
    TINYOBJ_FREE(commands);
  }

  destroy_hash_table(&material_table);

  (*materials_out) = materials;
  (*num_materials_out) = num_materials;

  return TINYOBJ_SUCCESS;
}

void tinyobj_attrib_init(Tinyobj_attrib *attrib) {
  attrib->vertices = NULL;
  attrib->num_vertices = 0;
  attrib->normals = NULL;
  attrib->num_normals = 0;
  attrib->texcoords = NULL;
  attrib->num_texcoords = 0;
  attrib->faces = NULL;
  attrib->num_faces = 0;
  attrib->face_num_verts = NULL;
  attrib->num_face_num_verts = 0;
  attrib->material_ids = NULL;
}

void tinyobj_attrib_free(Tinyobj_attrib *attrib) {
  if (attrib->vertices) TINYOBJ_FREE(attrib->vertices);
  if (attrib->normals) TINYOBJ_FREE(attrib->normals);
  if (attrib->texcoords) TINYOBJ_FREE(attrib->texcoords);
  if (attrib->faces) TINYOBJ_FREE(attrib->faces);
  if (attrib->face_num_verts) TINYOBJ_FREE(attrib->face_num_verts);
  if (attrib->material_ids) TINYOBJ_FREE(attrib->material_ids);
}

void tinyobj_shapes_free(Tinyobj_shape *shapes, size_t num_shapes) {
  size_t i;
  if (shapes == NULL) return;

  for (i = 0; i < num_shapes; i++) {
    if (shapes[i].name) TINYOBJ_FREE(shapes[i].name);
  }

  TINYOBJ_FREE(shapes);
}

void tinyobj_materials_free(Tinyobj_material *materials,
                            size_t num_materials) {
  size_t i;
  if (materials == NULL) return;

  for (i = 0; i < num_materials; i++) {
    if (materials[i].name) TINYOBJ_FREE(materials[i].name);
    if (materials[i].ambient_texname) TINYOBJ_FREE(materials[i].ambient_texname);
    if (materials[i].diffuse_texname) TINYOBJ_FREE(materials[i].diffuse_texname);
    if (materials[i].specular_texname) TINYOBJ_FREE(materials[i].specular_texname);
    if (materials[i].specular_highlight_texname)
      TINYOBJ_FREE(materials[i].specular_highlight_texname);
    if (materials[i].bump_texname) TINYOBJ_FREE(materials[i].bump_texname);
    if (materials[i].displacement_texname)
      TINYOBJ_FREE(materials[i].displacement_texname);
    if (materials[i].alpha_texname) TINYOBJ_FREE(materials[i].alpha_texname);
  }

  TINYOBJ_FREE(materials);
}
#endif /* TINYOBJ_LOADER_C_IMPLEMENTATION */
