#include "../src/internal/shader.h"

Shader* shader_create_from_source(const char *vert_src, const char *frag_src) {
    // Implementação fictícia para fins de exemplo
    // Em um cenário real, isso envolveria chamadas a APIs gráficas como OpenGL ou Vulkan
    (void)vert_src; // Evita warnings de variável não usada
    (void)frag_src; // Evita warnings de variável não usada
    return (Shader*)0xDEADBEEF; // Retorna um ponteiro fictício para o shader
}

void shader_destroy(Shader *s) {
    (void)s; // TODO: Implementar destruição de shader
}
