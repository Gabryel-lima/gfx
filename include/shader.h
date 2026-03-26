#pragma once

/**
 * @file include/shader.h
 * @brief Interface mínima para criação e destruição de objetos Shader.
 */

/** Opacidade para implementações de shader */
typedef struct Shader Shader; // Tipo opaco que representa um programa shader

/**
 * Cria um `Shader` a partir de código fonte de vértice e fragmento.
 * @param vert_src Código-fonte do shader de vértice (NULL-terminado)
 * @param frag_src Código-fonte do shader de fragmento (NULL-terminado)
 * @return Ponteiro para Shader recém-criado ou NULL em caso de erro.
 */
Shader* shader_create_from_source(const char *vert_src, const char *frag_src);

/** Destroi um `Shader` previamente criado 
 * @param s Ponteiro para o Shader a ser destruído
 * @return void
 * @note Após chamar esta função, o ponteiro `s` não deve ser usado novamente
*/
void shader_destroy(Shader *s);
