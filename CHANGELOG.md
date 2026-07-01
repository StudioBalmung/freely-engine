FREELY ENGINE 0.4.2 - CHANGELOG
================================

Version bump from 0.2.0 to 0.4.2, with build-compatibility and one
rendering-quality fix applied. These fixes were found and applied while
cross-compiling a real game (Darkness Rising) against this engine with
mingw-w64 to a native Windows x86_64 executable using the engine's
Window / Input / Renderer2D / Texture / Camera / AudioEngine runtime.


FIXES
-----

1. Core/Window.cpp
   Fixed a glad1-vs-glad2 API mismatch: the code called
   `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)`, which is the
   glad1 signature. Updated to the glad2 equivalent:
   `gladLoadGL((GLADloadfunc)glfwGetProcAddress)`.
   (The engine's CMakeLists.txt expects a vendored Thirdparty/glad;
   if you regenerate that folder with the glad2 web generator or the
   `glad2` Python package, use API `gl:core=4.5`, no extensions, to
   match what the engine's renderer calls.)

2. Renderer2D/Renderer2D.h, Renderer2D/Renderer2D.cpp
   Fixed a compile error where the internal free-function helper
   `PushQuad` (used by DrawSprite/DrawSubSprite/DrawRect) called the
   private static method `Renderer2D::NextBatch()`. Added a
   namespace-scope forward declaration of `PushQuad` before the class,
   added it as a `friend` of `Renderer2D`, and removed the conflicting
   `static` (internal linkage) qualifier from its definition in the
   .cpp file, since a friend declaration requires external linkage.

3. Renderer/Shader.h
   Added a missing `#include <memory>`. The header uses
   `std::shared_ptr` but did not include the header that declares it;
   this previously only worked by accident when another header already
   pulled in <memory> first.

4. Renderer/Texture.cpp
   Changed the path-based Texture2D constructor's default filtering
   from GL_LINEAR_MIPMAP_LINEAR / GL_LINEAR with mipmap generation to
   GL_NEAREST with no mipmaps, and wrap mode from GL_REPEAT to
   GL_CLAMP_TO_EDGE. The previous defaults caused visible blur and
   color bleeding between adjacent frames of the same sprite-sheet
   texture atlas for pixel-art style 2D games. This does not affect
   the separate TextureSpec-based constructor used internally by
   Font's SDF atlas, which already had correct settings.


------------

Note: 
This release resolves that by setting every internal version reference to 0.4.2, matching the intended
release name going forward.
