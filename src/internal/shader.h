#pragma once

/**
 * @brief Suporte esboçado para criação e destruição de objetos Shader.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/shader.h
*/

/** Tipo opaco que representa um programa shader. */
typedef struct Shader Shader;

/**
 * Cria um `Shader` a partir de código-fonte de vértice e fragmento.
 * @param vert_src Código-fonte do shader de vértice (NULL-terminado)
 * @param frag_src Código-fonte do shader de fragmento (NULL-terminado)
 * @return Ponteiro para Shader recém-criado ou NULL em caso de erro.
 * @note A implementação atual é um placeholder.
*/
Shader* gfx_shader_create_from_source(const char *vert_src, const char *frag_src);

/** Destroi um `Shader` previamente criado.
 * @param s Ponteiro para o Shader a ser destruído
 * @return void
 * @note A implementação atual é um placeholder.
*/
void gfx_shader_destroy(Shader *s);
