# gfx — API gráfica em C puro

Biblioteca de renderização 2D/3D escrita em C99. O núcleo (`include/` + `core/`) não depende de nada além da libc padrão — sem X11, sem OpenGL, sem `dlopen` — para poder ser embutido em outro projeto com o mínimo de fricção possível, inclusive um kernel bare-metal no futuro. O backend de janela Linux/X11/OpenGL é um extra opcional e isolado, para quem quer abrir uma janela real num desktop Linux.

![C99](https://img.shields.io/badge/C99-00599C?style=for-the-badge) ![Linux](https://img.shields.io/badge/Linux-2E2E2E?style=for-the-badge) ![Core%20sem%20SO](https://img.shields.io/badge/Core_sem_SO-2E7D32?style=for-the-badge)

## Estrutura do projeto

```
gfx/
├── include/                 ← headers públicos
│   ├── gfx.h                    fachada: GfxContext/GfxBackend, dispatch, Mesh
│   ├── gfx_math.h               Vec2/Vec3/Vec4/Mat4 e helpers escalares
│   ├── gfx_raster.h             Framebuffer portátil + rasterizador por software
│   ├── gfx_platform_linux_fb.h  extra: abrir /dev/fb0 no Linux
│   ├── gfx_platform_gl_window.h extra: janela Linux/X11/OpenGL
│   └── tinyobj_loader.h         parser OBJ/MTL (header-only)
├── core/                    ← núcleo portátil: só libc padrão (fopen/malloc no máximo)
├── platform/linux/          ← extras específicos do Linux (fb0, X11, GL via dlopen)
├── examples/                ← demos
├── tests/                   ← testes
└── CMakeLists.txt
```

`core/` é o que se copia para embutir o gfx em outro projeto. `include/gfx_math.h`, `include/gfx_raster.h` e as três fontes correspondentes (`core/gfx_math.c`, `core/gfx_raster.c`, `core/gfx_framebuffer.c`) não usam nada além de `<stdint.h>`/`<stddef.h>` — é o subconjunto pensado para rodar sem sistema operacional, atrás de um framebuffer linear entregue por um bootloader, por exemplo. `core/gfx_mesh.c` e o parser OBJ (`core/tinyobj_loader.c`, `core/tinyobj_preview.c`) também são portáveis entre sistemas hospedados, mas usam `fopen`/`malloc`, então dependem de uma libc real (não fazem sentido bare-metal sem um shim de alocador/arquivo).

## Embutindo em outro projeto

```bash
cp -r include/ core/ /caminho/do/seu/projeto/vendor/gfx/
```

Compile os `.c` de `core/` com qualquer compilador C99, adicione `include/` ao `-I`, e pronto — nenhuma outra dependência, nenhuma flag especial. Se você só precisa do núcleo mínimo (matemática + rasterizador + framebuffer, sem carregar OBJ), pode compilar apenas `gfx_math.c`, `gfx_raster.c` e `gfx_framebuffer.c`; é exatamente esse subconjunto que o CMake valida com `-DGFX_CORE_FREESTANDING_CHECK=ON` (veja abaixo).

## API pública

### `gfx.h` — fachada e dispatcher
- `GfxContext`, `GfxBackend` (com `begin_frame`, `end_frame`, `draw_mesh`, `set_camera` e `cleanup`)
- `gfx_context_init`, `gfx_begin_frame`, `gfx_end_frame`, `gfx_draw_mesh`, `gfx_set_camera`, `gfx_cleanup`
- `gfx_get_stub_backend` — backend de depuração que só imprime cada chamada
- `Mesh` (opaco) e seus acessores: `gfx_mesh_load`, `gfx_mesh_free`, `gfx_mesh_vertex_count`, `gfx_mesh_triangle_count`, `gfx_mesh_positions`, `gfx_mesh_triangle_colors`

### `gfx_math.h` — tipos compartilhados
- `Vec2`, `Vec3`, `Vec4`, `Mat4`, `mat4_identity`
- `gfx_min`, `gfx_max`, `gfx_fminf`, `gfx_fmaxf`
- `vec3_min`, `vec3_max`, `vec3_clamp`
- `gfx_edge2d`, `vec3_to_rgba`

### `gfx_raster.h` — framebuffer e rasterizador por software (o núcleo portátil)
- `Framebuffer` (`pixels`/`width`/`height`/`pitch`, sem conceito de sistema operacional)
- `gfx_fb_clear`, `gfx_fb_set_pixel`
- `gfx_rasterize_triangle` — cores interpoladas com correção perspectiva (o `w`
  do vértice entra na conta; `w = 1` nos três dá o caso afim), profundidade
  linear em espaço de tela e regra top-left nas arestas compartilhadas. Não há
  clipping: cortar contra o frustum é de quem chama.

### `gfx_platform_linux_fb.h` — extra: framebuffer Linux
- `GfxLinuxFb` (`Framebuffer` + `fd`), `gfx_fb_open("/dev/fb0")`, `gfx_fb_close`

### `gfx_platform_gl_window.h` — extra: janela Linux/X11/OpenGL
- `PlatformWindow` (opaco), `gfx_platform_window_create`, `gfx_platform_window_destroy`
- `gfx_platform_window_pump_events`, `gfx_platform_window_should_close`
- `gfx_platform_window_set_clear_color`, `gfx_platform_window_context`
- `GfxLight`, `gfx_platform_window_default_light`, `gfx_platform_window_set_light` —
  luz direcional (sem atenuação) para o sombreamento Phong por fragmento
- `gfx_platform_window_read_pixel` — lê o back buffer entre `gfx_begin_frame` e
  `gfx_end_frame`; existe para tornar a saída verificável sem olhar a tela
- Implementado via `dlopen`/`dlsym` — só linka `libdl` (e `libm`), nunca `libGL`/`libX11` diretamente.

### `tinyobj_loader.h` — parser OBJ/MTL
- `tinyobj_load_obj`, `tinyobj_load_mtl`, `tinyobj_attrib_init/free`, `tinyobj_shapes_free`, `tinyobj_materials_free`
- `tinyobj_attrib_get_vertex`, `tinyobj_attrib_compute_bounds`, `tinyobj_material_color`, `tinyobj_project_vertex`
- `tinyobj_save_preview_ppm` (`core/tinyobj_preview.c`)

## Uso básico

```c
#include "gfx.h"

int main(void) {
    GfxContext ctx;
    gfx_context_init(&ctx, gfx_get_stub_backend(), NULL);

    gfx_begin_frame(&ctx);
    gfx_set_camera(&ctx, (Vec3){0, 1, 3}, (Vec3){0, 0, 0}, 60.0f);
    gfx_draw_mesh(&ctx, NULL, mat4_identity());
    gfx_end_frame(&ctx);
    gfx_cleanup(&ctx);
    return 0;
}
```

## Compilação

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Alvos: `gfx_demo` e `tinyobj_demo` linkam só `gfx_core` (sem `dl`, sem X11/GL). `gfx_tests` cobre o núcleo e a camada Linux (fb0, loaders X11/GL). `gfx_window_demo` só é construído se `GFX_ENABLE_GL_BACKEND=ON` (padrão em Linux) — desligue com `-DGFX_ENABLE_GL_BACKEND=OFF` se não quiser o backend de janela.

Para validar que o subconjunto bare-metal (`gfx_math.c` + `gfx_raster.c` + `gfx_framebuffer.c`) continua sem dependência de SO:

```bash
cmake -B build-freestanding -DGFX_CORE_FREESTANDING_CHECK=ON
cmake --build build-freestanding --target gfx_core_freestanding_check
```

Isso compila (sem linkar) esses três arquivos com `-ffreestanding`, para pegar cedo qualquer dependência hospedada que volte a aparecer sem querer.

## Executando os exemplos

- `./build/gfx_demo` executa o smoke test da fachada pública usando `gfx_get_stub_backend()`.
- `./build/tinyobj_demo [modelo.obj] [saida.ppm]` carrega OBJ/MTL e grava uma prévia em PPM usando só o núcleo portátil.
- `./build/gfx_window_demo` abre uma janela nativa Linux com contexto GLX e desenha um cubo girando com iluminação Phong; em ambiente headless, rode-o sob `xvfb-run`. Ele também se autoverifica: amostra uma grade de pixels sobre o cubo e sai com código 1 se a variação de luminância indicar que a iluminação não chegou ao fragmento. Como todas as faces do `examples/models/cube.obj` usam o mesmo material, qualquer diferença de cor entre elas só pode vir do sombreamento.

## Dependências em tempo de execução

- Núcleo (`gfx_demo`, `tinyobj_demo`): nenhuma além da libc.
- `gfx_platform_linux_fb.h` (`gfx_fb_open`/`gfx_fb_close`): Linux, `/dev/fb0`, normalmente exige root ou grupo `video`.
- `gfx_platform_gl_window.h` / `gfx_window_demo`: Linux, `libdl` no link, `libGL.so.1` e `libX11.so` carregadas via `dlopen` em tempo de execução, sessão X11 (`DISPLAY`) ou `xvfb-run`.

## Roadmap

### ✅ Já presente
- Núcleo portátil: math, framebuffer, rasterizador por software com z-buffer,
  interpolação com correção perspectiva e regra top-left
- Parser `.obj`/`.mtl` próprio, com normais por vértice (geradas a partir da
  face quando o arquivo não traz `vn`)
- Extra: framebuffer Linux (`/dev/fb0`)
- Extra: janela Linux/X11/OpenGL via `dlopen`, com iluminação Phong por fragmento
- Embutido no kernel do [AlmaOS](https://github.com/Gabryel-lima/AlmaOS): o
  núcleo portátil roda lá sobre um framebuffer VGA/VBE real (`third_party/gfx`
  + `src/kernel/gfx_bridge.c` naquele repositório)
- Demos e testes

### 🔭 Próximos passos
- Texturas: os UVs já saem do carregador (`gfx_mesh_texcoords`), mas ainda
  faltam um decodificador de imagem, o `map_Kd` do MTL e o caminho de textura
  nos dois backends
- Sombras (shadow mapping)
- Avaliar um backend Wayland e suporte Windows/WGL

## 📜 Licença

Veja [LICENSE](LICENSE) — MIT License.
