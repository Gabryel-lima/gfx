# TODO do projeto gfx

Backlog vivo para ir fechando as partes que ainda faltam até o projeto ficar completo e funcional.

## Como usar
- Marque o que já estiver pronto com `x`.
- Se um item virar muito grande, quebre em subtarefas menores abaixo dele.
- Se algo deixar de fazer sentido, mova para a seção de notas no fim.

## P0 - Funcionalidade essencial
- [x] Implementar `gfx_mesh_load` em `gpu/mesh.c` com carregamento real de malha a partir de arquivo.
- [x] Implementar `gfx_mesh_free` em `gpu/mesh.c` com liberação correta de todos os recursos alocados.
- [x] Implementar `gfx_shader_create_from_source` em `gpu/shader.c` retornando um shader real, sem placeholder.
- [x] Implementar `gfx_shader_destroy` em `gpu/shader.c` com destruição segura do shader.
- [x] Corrigir qualquer fluxo de backend que ainda dependa de comportamento fictício ou de retorno artificial.
- [x] Garantir que o backend stub continue útil para smoke tests sem causar recursão ou comportamento confuso.

## P1 - Robustez do caminho CPU
- [x] Melhorar o tratamento de erro em `cpu/fb0_platform.c` para falhas de abertura do framebuffer.
- [x] Revisar `cpu/framebuffer.c` e `cpu/rasterizer.c` para cobrir melhor casos-limite e erros de entrada.
- [x] Reavaliar `gfx_fminf` e `gfx_fmaxf` em `src/gfx_math.c` para lidar corretamente com NaN e infinitos, se isso for necessário para o projeto.
- [x] Decidir se o suporte a quadriláteros em `cpu/rasterizer.c` deve ser implementado ou removido definitivamente.

## P1 - Parser OBJ/MTL
- [x] Fechar os `@todo` restantes em `include/tinyobj_loader.h`.
- [x] Melhorar o tratamento de parâmetros desconhecidos em MTL.
- [x] Revisar suporte a múltiplos grupos e objetos no loader.
- [x] Validar se a estratégia de alocação atual do parser continua adequada ou se vale migrar para estruturas dinâmicas.
- [x] Adicionar testes com OBJ/MTL reais para cobrir casos simples e casos com arestas.

## P1 - Testes e validação
- [x] Criar uma suíte de testes automatizados para os módulos centrais.
- [x] Adicionar integração com `ctest` no CMake.
- [x] Fazer o alvo `test` do `Makefile` apontar para uma rotina real de testes.
- [x] Cobrir pelo menos: math básica, parser OBJ/MTL, rasterização CPU e smoke test da fachada pública.

## P2 - Infraestrutura
- [ ] Adicionar CI para buildar o projeto em Linux.
- [ ] Validar builds em `Debug` e `Release`.
- [ ] Documentar dependências de runtime e permissões necessárias para o caminho CPU.
- [ ] Verificar se o carregamento dinâmico de `libGL.so.1` e `libX11.so` precisa de mensagens de erro melhores.

## P2 - Janela nativa e portabilidade
- [ ] Criar um backend de janela no Linux para renderizar em uma janela própria, com ciclo de apresentação e integração de eventos.
- [ ] Separar a camada de plataforma para isolar criação de janela, contexto gráfico e apresentação do frame.
- [ ] Validar o fluxo completo no Linux antes de iniciar a porta para Windows e macOS.
- [ ] Preparar a abstração de plataforma para que a API pública de alto nível não precise mudar quando vier a portabilidade.

## P2 - Documentação e consistência
- [ ] Revisar o `README.md` depois que os stubs GPU forem implementados.
- [ ] Atualizar a documentação quando a API pública mudar.
- [ ] Garantir que a documentação não prometa recursos que ainda não existem.
- [ ] Manter exemplos e headers públicos alinhados com o contrato real.

## Critério de pronto
- [ ] `cmake --build build` conclui sem erros.
- [ ] `ctest` ou o alvo equivalente executa a suíte sem falhas.
- [ ] `gfx_demo` roda como smoke test da fachada pública.
- [ ] `tinyobj_demo` carrega um OBJ e gera a prévia PPM sem intervenção extra.
- [ ] O caminho GPU deixa de ser esboço e passa a funcionar de ponta a ponta.

## Notas
- O foco inicial deve ser fechar os stubs e os testes; documentação vem logo depois para não cristalizar promessas falsas.
- Se aparecerem novos `TODO`, `FIXME` ou `HACK`, eles podem entrar aqui na categoria certa.
