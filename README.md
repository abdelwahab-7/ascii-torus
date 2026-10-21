# ASCII Torus

A real-time 3D ASCII animation written in C++17. It renders a spinning torus
(donut) surrounded by an independently spinning wireframe cube, entirely with
characters in an 80 × 40 terminal window.

```text
....--.-....-----.-.....------.......-.-..-..
.             -----------------             .
.         -----.,,,,------,,..-----         .
.       ---,--~~:::::;;;::::~~--,.---       .
,     --.,-~::;====!!!!!!!===;;::~-,,--     ,
,   --.,~::;;==!!!!!!***!!!!!!!=;;::--,--   ,
-  -.,~::;==!!!!!***######***!!!!!=;::~-.-  -
- -,-~:;==!!!!**###$$$$$$$###**!!!!==;:~-.- -
~ ,~:;;=!!!!***###$$$$$$$$$###***!!!==;::~,-~
~,~:;==!!!!*****######*#######****!!!!==;:-.~
:-:;;==!!!!****!!!=       =!*!****!!!!==;;~-:
::;==!!!!!!!=;;~--         --:===!!!!!!!=;;~:
;:;=!!!!!!!!=;:~-.-       -.-~:==!!!!!!!!=;:;
=:;=!!***!!!==;:-,---------,-:;==!!!!*!!!=;:=
=~;=!!***!!!!==;:~,,.---.,,~:;==!!!!***!!=;-=
! ;=!******!!!!=;;::~---~::;;==!!!******!=: !
!  ;!***##***!!!!==;;;;;;===!!!!****#***!;  !
!   =!*######**!!!!!!!!!!!!!!!**#####**!;   !
*     !*#########****!!!*****########*=     *
*       *###$$$$$$##########$$$$$###*       *
*         !*#$$$$$$$$$$$$$$$$$$$#*          *
*            !##$$$$$$$$$$$$$#!             *
*########$$$$$$$$$$$$$$$$$$$$$$$$$$$$#######*
```

## At a glance

| Item | Value |
| --- | --- |
| Language | C++17 |
| Source | `torus.cpp` (single file, no dependencies) |
| Output | 80 × 40 ASCII, ANSI escape codes |
| Geometry | 15,876 torus samples + 2,640 wireframe-cube samples |
| Rendering | Orthographic projection, z-buffer, Lambert shading |
| Animation | Continuous rotation, 10 ms delay per frame |

## Repository contents

```text
torus.cpp    The complete program
README.md    This file
.gitignore   Ignores compiled binaries and macOS debug files
```

Only `torus.cpp` is included from the original project folder. The other
files that may exist there (`main.cpp`, the compiled `main`/`torus`/`donut`
binaries and the `.dSYM` folders) are a separate earlier demo / build
artifacts and are not part of this repository.

## Build and run

```bash
c++ -std=c++17 -O2 torus.cpp -o torus
./torus
```

On most macOS/Linux systems `c++` is Clang or GCC; either works. Run it in a
terminal that is **at least 80 columns wide and 41 rows tall** (40 lines of
output plus the final newline). Press `Ctrl+C` to stop.

### If `bits/stdc++.h` is missing

The first line of the file is:

```cpp
#include <bits/stdc++.h>
```

That is a non-standard convenience header that ships with GCC. It is not part
of the C++ standard and does not exist on a clean macOS/Clang or MSVC install
(some macOS setups have a manually installed copy under
`/usr/local/include`). If your compiler cannot find it, replace that line
with the headers actually used:

```cpp
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
```

The program then compiles with any conforming C++17 compiler.

### Windows

Use WSL, MSYS2/MinGW, or another environment that provides a POSIX-style
terminal; a native MSVC build needs the header replacement above. Windows
Terminal (Windows 10+) understands the ANSI escape codes used for clearing
the screen.

## How it works

The program is a small software renderer. Every frame it

1. rotates every vertex of both objects,
2. projects it onto the 80 × 40 character grid,
3. uses a depth buffer to keep only the nearest point per cell,
4. converts the surface normal into a brightness character,
5. clears the terminal and prints the whole grid,
6. advances the animation angle and sleeps for 10 ms.

No graphics library, no window toolkit — just math and `std::cout`.

### Geometry

**Torus.** The donut is built from the parametric surface

```text
x = (R + r·cos v)·cos u
y = (R + r·cos v)·sin u
z = r·sin v
```

* `u ∈ [0, 2π)` walks around the large ring (in the xy-plane).
* `v ∈ [0, 2π)` walks around the tube.
* `R = 7` is the major radius (center of the torus to the center of the tube).
* `r = 4` is the minor radius (radius of the tube itself).
* The outer radius is `R + r = 11`; the hole radius is `R − r = 3`.

Its outward unit normal is

```text
n = (cos v·cos u, cos v·sin u, sin v)
```

which is exactly the formula used in the code.

**Cube frame.** A cube of half-size `S = 11`, drawn only as its 12 edges. Each
edge is sampled every 0.1 units, producing roughly 220 points per edge. The
normal stored for each edge point is `normalize(position)` — the direction
from the origin, not a true cube-face normal. That is not physically correct,
but because the cube is only a wireframe it gives a pleasant smooth lighting
variation along the edges.

### Rotation

Points are rotated with 3 × 3 matrices:

```text
Rx(a) = [1  0   0 ]      Ry(a) = [ c  0  s ]      Rz(a) = [ c -s  0 ]
        [0  c  -s ]              [ 0  1  0 ]              [ s  c  0 ]
        [0  s   c ]              [-s  0  c ]              [ 0  0  1 ]
```

The torus and the cube use different angles per axis, so they tumble at
different speeds and in different directions:

```text
torus:  Rx(0.2·a) · Ry(0.5·a) · Rz(0.7·a)
cube:   Rx(−0.4·a) · Ry(0.8·a) · Rz(−0.2·a)
```

Because the expression is `rx * (ry * (rz * p))`, the rotations are applied
to a point in the order **Rz first, then Ry, then Rx**.

Rotations preserve length and angles, so the same matrix can be applied to
both the position and the normal (no inverse-transpose is needed, as it would
be for scaling).

### Projection

The code drops `z` and maps the remaining coordinates directly:

```cpp
screenX = int(x * 2) + width / 2;
screenY = int(y * 1) + height / 2;
```

This is an orthographic (parallel) projection — there is no perspective
foreshortening, so distant and near parts of the object are the same size.
The horizontal scale factor of 2 compensates for terminal cells being about
twice as tall as they are wide, keeping the torus round instead of squashed.

The cube's corners can reach a radius of `11·√3 ≈ 19`, so its projected
coordinates can reach `int(19·2) + 40 = 78` horizontally and `19 + 20 = 39`
vertically. That just fits in 80 × 40, which is why the bounds check in the
code rarely (if ever) rejects anything at this size.

### Depth buffer

`zbuffer[y][x]` stores the largest `z` seen so far for that character cell.
A new point replaces the existing one only when its `z` is larger:

```cpp
if (rotated_p.z > zbuffer[screenY][screenX]) { ... }
```

Larger `z` is treated as closer, which corresponds to a camera on the +z axis
looking toward −z. Because both objects share one depth buffer, the cube and
the torus correctly occlude each other.

### Lighting and characters

A single directional light is used. It starts as `(0, 1, 1)`, is normalized,
and is then rotated around the y-axis by `2·a`, so the light orbits the scene
and highlights sweep across the surface.

For every vertex:

```text
luminance = dot(normal, light)
```

This is a Lambertian (diffuse) term and lies in `[−1, 1]`. Positive values
face the light and are mapped to the 12-character brightness ramp

```text
".,-~:;=!*#$@"
```

from darkest (`.`) to brightest (`@`):

```cpp
int luminance_index = (int)(luminance * 11.0f);
screen[y][x] = shades[luminance_index];
```

Negative values (surfaces facing away from the light) are drawn as `-`
instead of a space, so the silhouette of the object stays visible even in
shadow. There is no ambient light, specular highlight, or shadow casting.

### Output

Each frame starts with the ANSI sequence

```text
ESC[2J   clear the entire screen
ESC[H    move the cursor to the top-left corner
```

then prints all 40 rows. `ESC` is written as `\033` in the code. The frame is
followed by 10 ms of sleep, giving a nominal ~100 frames per second (the real
rate is limited by the terminal and CPU).

## Code walkthrough

Line numbers refer to `torus.cpp` as committed.

### Includes — lines 1–4

```cpp
#include <bits/stdc++.h>
#include <cmath>
#include <vector>
using namespace std;
```

* `bits/stdc++.h` pulls in the whole GNU C++ standard library (including
  `<iostream>`, `<array>`, `<string>`, `<chrono>` and `<thread>` used below).
  It is convenient for a single-file program but not portable.
* `<cmath>` and `<vector>` are already included by the first line; they are
  redundant but harmless.
* `using namespace std;` lets the code write `vector`, `string`, `cout`, etc.
  without `std::`. Acceptable in a small standalone program.

### `Point` — lines 6–8

```cpp
struct Point {
  float x, y, z;
};
```

A plain 3D vector. It is used for both positions (points in space) and
directions (surface normals and the light vector). `float` is sufficient for
a character-grid renderer.

### `Vertex` — lines 10–14

```cpp
struct Vertex {
  Point p;
  Point normal;
  int label;
};
```

One sample of a surface:

* `p` — position in 3D.
* `normal` — outward direction used for lighting.
* `label` — which object it belongs to: `0` = torus, `1` = cube frame. The
  render loop reads this to choose the correct rotation matrices.

### `Tmatrix` — lines 15–17

```cpp
struct Tmatrix {
  float mat[3][3];
};
```

A bare 3 × 3 matrix. Only rotations are needed here, and rotations are
3 × 3; there is no translation or perspective, so no 4 × 4 matrix is
required.

### Matrix × point — lines 19–25

```cpp
Point operator*(const Tmatrix& m, const Point& p){
  return {
    m.mat[0][0] * p.x + m.mat[0][1] * p.y + m.mat[0][2] * p.z,
    m.mat[1][0] * p.x + m.mat[1][1] * p.y + m.mat[1][2] * p.z,
    m.mat[2][0] * p.x + m.mat[2][1] * p.y + m.mat[2][2] * p.z,
  };
}
```

Overloads `*` so that `matrix * point` performs the standard matrix-vector
product, returning the rotated point. Each output component is one row of the
matrix dotted with the input vector.

### Dot product — lines 27–29

```cpp
float operator*(const Point& u, const Point& w){
  return u.x * w.x + u.y * w.y + u.z * w.z;
}
```

A second overload of `*`, this time `point * point → float`. It computes the
dot product, which is used as the diffuse lighting term. Overloading the same
operator for two different operations keeps call sites short
(`rotated_norm * light`) but can be confusing to read.

### `get_angles` — lines 31–33

```cpp
array<float, 2> get_angles(float angle){
  return {cos(angle), sin(angle)};
}
```

Returns the cosine and sine of an angle as a two-element `std::array`. The
callers unpack it with a C++17 structured binding:

```cpp
auto [c, s] = get_angles(angle);
```

This avoids calling `cos` and `sin` separately in each rotation-matrix
function.

### `normalize` — lines 35–38

```cpp
Point normalize(Point& p){
  float len = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
  return {p.x / len, p.y / len, p.z / len};
}
```

Divides a vector by its Euclidean length so the result has length 1. This is
needed for the light vector and for the cube-edge normals.

Two details:

* The parameter is a non-const reference even though the function never
  modifies it; `const Point&` would be more correct. It also means the
  function cannot be called on a temporary.
* There is no guard against `len == 0`; a zero vector would produce a
  division by zero (NaN). The current inputs never trigger this.

### Rotation matrices — lines 40–53

```cpp
Tmatrix Rx(float angle){
  auto [c, s] = get_angles(angle);
  return {{{1, 0, 0}, {0, c, -s}, {0, s, c}}};
}

Tmatrix Ry(float angle){
  auto [c, s] = get_angles(angle);
  return {{{c, 0, s}, {0, 1, 0}, {-s, 0, c}}};
}

Tmatrix Rz(float angle){
  auto [c, s] = get_angles(angle);
  return {{{c, -s, 0}, {s, c , 0}, {0, 0, 1}}};
}
```

The three standard right-handed rotation matrices:

* `Rx` rotates around the x-axis and leaves x unchanged.
* `Ry` rotates around the y-axis and leaves y unchanged.
* `Rz` rotates around the z-axis and leaves z unchanged.

The brace initializer `{{{...}, {...}, {...}}}` initializes the nested
`mat[3][3]` array. The extra spaces in `{s, c , 0}` are cosmetic.

### Constants — lines 55–57

```cpp
const int R = 7;
const int r = 4;
const string shades = ".,-~:;=!*#$@";
```

* `R` — major radius of the torus (7).
* `r` — minor/tube radius of the torus (4).
* `shades` — the 12-character brightness ramp, darkest to brightest. The
  characters were made famous by the classic `donut.c` demo.

### Building the torus — lines 59–67

```cpp
vector<Vertex> vertices;
for (float u = 0.0f; u <= 6.28f; u += 0.05f){
  for (float v = 0.0f; v <= 6.28f; v += 0.05f){
    Point p = {(R + r * cos(v)) * cos(u), (R + r * cos(v)) * sin(u), r * sin(v)};
    Point norm = {cos(v) * cos(u), cos(v) * sin(u), sin(v)};
    vertices.push_back({p, norm, 0});
  }
}
```

* Nested loops over `u` and `v` with a step of 0.05 radians produce 126 values
  of each parameter (the last one is 6.25001, not 6.28319). That is
  126 × 126 = **15,876 torus points**.
* `p` evaluates the parametric torus equations shown earlier.
* `norm` evaluates the corresponding unit normal.
* `label` is `0` for every torus point.
* Because the loop stops at `6.28` instead of exactly `2π ≈ 6.283185`, the
  surface has a tiny unwrapped seam at the back. It is much smaller than one
  grid step and is not noticeable in the animation.
* The vector has no `reserve`, so it reallocates a few times while filling;
  this happens once at startup and does not affect the animation.

### Building the cube frame — lines 69–84

```cpp
const float S = 11.0f;
for (float t = -S; t <= S; t += 0.1f){
  auto add_edge = [&](float x, float y, float z){
    Point p = {x, y, z};
    Point norm = normalize(p);
    vertices.push_back({p, norm, 1});
  };
  add_edge(t, -S, -S); add_edge(t , S, -S);
  add_edge(t, -S, S); add_edge(t , S, S);

  add_edge(-S, t, -S); add_edge(S , t, -S);
  add_edge(-S, t, S); add_edge(S , t, S);

  add_edge(-S, -S, t); add_edge(S , -S, t);
  add_edge(-S, S, t); add_edge(S , S, t);
}
```

* `S = 11` is the half-size of the cube, so it spans `[-11, 11]` on each axis.
* `t` walks from `−S` to `S` in steps of 0.1, producing 220 samples per edge
  (floating-point accumulation stops at `t = 10.9`).
* `add_edge` is a lambda that captures `vertices` by reference, builds a
  point, normalizes its position to use as a normal, marks it with `label 1`,
  and appends it.
* The twelve calls create the twelve edges of the cube, four per axis:
  * four edges parallel to x: `(t, ±S, ±S)`,
  * four edges parallel to y: `(±S, t, ±S)`,
  * four edges parallel to z: `(±S, ±S, t)`.
* 12 edges × 220 samples = **2,640 cube points**, for a total of
  **18,516 vertices** in the scene.

The cube's half-size equals the torus's outer radius, so the wireframe box
just touches the torus's outer equator; the shared z-buffer sorts out which
character is in front wherever they overlap.

### Frame state — lines 86–99

```cpp
float angle = 0.0f;
const int height = 40;
const int width = 80;

while (true){
  char screen[height][width];
  float zbuffer[height][width];

  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      screen[i][j] = ' ';
      zbuffer[i][j] = -1000.0f;
    }
  }
```

* `angle` is the global animation phase, advanced every frame.
* The frame is fixed at 40 rows × 80 columns.
* `screen` is the character framebuffer; `zbuffer` holds the closest depth
  seen per cell. They are local to the loop body, so a fresh pair is created
  each frame (about 16 KB of stack).
* Both are cleared: the screen to spaces, the depth buffer to `−1000` (a
  depth far behind anything in the scene).

### Rotating the two objects — lines 101–118

```cpp
Tmatrix rx = Rx(angle * 0.2f);
Tmatrix ry = Ry(angle * 0.5f);
Tmatrix rz = Rz(angle * 0.7f);

Tmatrix rx_s = Rx(angle * -0.4f);
Tmatrix ry_s = Ry(angle * 0.8f);
Tmatrix rz_s = Rz(angle * -0.2f);

Point rotated_p;
Point rotated_norm;
for (auto& [p, norm, label] : vertices){
  if (label == 0){
    rotated_p = rx * (ry * (rz * p));
    rotated_norm = rx * (ry * (rz * norm));
  } else {
    rotated_p = rx_s * (ry_s * (rz_s * p));
    rotated_norm = rx_s * (ry_s * (rz_s * norm));
  }
```

* Six matrices are built once per frame: three for the torus and three for
  the cube (`_s` presumably stands for the original "square"/cube demo).
* The multiplication order is `Rz` applied first, then `Ry`, then `Rx`.
* The loop uses a C++17 structured binding with `auto&`, so `p`, `norm` and
  `label` refer directly to the members of each vertex.
* `label == 0` selects the torus matrices; every other value selects the
  cube matrices. The same rotations are applied to the position and to the
  normal.
* `rotated_p` and `rotated_norm` are declared outside the loop and reused
  for each vertex; only their final values are used per iteration.

### Projection to the screen — lines 120–121

```cpp
int screenX = (int)(rotated_p.x * 2) + (width / 2);
int screenY = (int)(rotated_p.y * 1) + (height / 2);
```

* `x` is scaled by 2, `y` by 1, to compensate for the terminal cell aspect
  ratio (cells are taller than they are wide).
* Adding `width / 2` and `height / 2` centers the origin.
* `(int)` truncates toward zero rather than rounding or flooring. For
  negative coordinates this shifts the projection by up to one cell and makes
  it very slightly asymmetric around the center; at this resolution it is not
  noticeable.
* This is a parallel projection: `z` is used only for depth, not for size.

### The moving light — lines 123–129

```cpp
Point light = {0.0f, 1.0f, 1.0f};
light = normalize(light);

Tmatrix light_rot_y = Ry(angle * 2.0f);
light = light_rot_y * light;

float luminance = rotated_norm * light;
```

* The light starts above and in front of the scene: `(0, 1, 1)`, normalized
  to length 1.
* Rotating it around the y-axis by `2·angle` makes it orbit, so highlights
  travel across the torus and the cube edges.
* `luminance` is the dot product of the unit surface normal and the unit
  light direction. It is positive where the surface faces the light and
  negative where it faces away.
* All three light computations are **inside the per-vertex loop**, so the
  same normalized, rotated light vector is recomputed 18,516 times per frame.
  This is correct but wasteful; it could be hoisted above the loop. That is
  roughly 18.5k unnecessary `sqrt` calls per frame.

### Depth test, shading and plotting — lines 131–141

```cpp
if (screenX >= 0 && screenX < width && screenY >= 0 && screenY < height){
  if (rotated_p.z > zbuffer[screenY][screenX]){
    zbuffer[screenY][screenX] = rotated_p.z;
    if (luminance > 0){
      int luminance_index = (int)(luminance * 11.0f);
      screen[screenY][screenX] = shades[luminance_index];
    } else {
      screen[screenY][screenX] = '-';
    }
  }
}
```

* The bounds check discards points that project outside the window.
* The depth test keeps the point with the largest `z` (nearest to the
  camera) for each cell. The depth buffer is updated even for shaded-off
  points, so a nearer dark point still hides a farther bright one — this is
  what makes occlusion correct.
* For lit points, `luminance ∈ (0, 1]` is multiplied by 11 and truncated to an
  index into `shades`:
  * `luminance ≈ 0` → index 0 → `.`
  * `luminance ≈ 0.5` → index 5 → `;`
  * `luminance = 1` → index 11 → `@`
* For unlit points, `-` is used as a visible "dark surface" character. `-`
  also happens to be `shades[2]`, but here it is a separate fallback, not a
  ramp entry.
* There is **no clamp** on `luminance_index`. It is safe here because the
  normal and light are unit vectors, so the dot product cannot exceed 1 by
  more than floating-point noise. It would become an out-of-bounds read if
  the normals were ever scaled or the dot product were changed to a
  non-normalized sum. Adding `std::clamp(index, 0, 11)` is the robust fix.

### Drawing and advancing — lines 143–152

```cpp
cout << "\033[2J\033[H";
for (int i = 0; i < height; i++){
  for (int j = 0; j < width; j++){
    cout << screen[i][j];
  }
  cout << '\n';
}
angle += 0.05f;
this_thread::sleep_for(chrono::milliseconds(10));
```

* `\033[2J` clears the terminal and `\033[H` homes the cursor.
* The nested loop prints 80 characters per row and a newline after each row.
* Time is advanced by 0.05 radians per frame, then the program sleeps 10 ms.
  The sleep is a fixed delay, not a frame-rate limiter: the real frame time
  is `10 ms + rendering time`, so the speed varies with the machine and
  terminal.
* `angle` grows without bound. `float` has about 7 significant decimal
  digits, so after a few hours of running the increments become coarse and
  the motion can visibly stutter. Wrapping it with
  `angle = fmod(angle, 2π)` would avoid that.
* `while (true)` never terminates; `return 0` at line 153 is unreachable and
  a compiler may warn about it. Exit with `Ctrl+C`.

## Constants you can tweak

| Constant | Location | Value | Effect |
| --- | --- | --- | --- |
| `R` | line 55 | `7` | Major torus radius (size of the ring) |
| `r` | line 56 | `4` | Tube radius (thickness of the donut) |
| `shades` | line 57 | 12 chars | Brightness ramp, dark → bright |
| `S` | line 69 | `11.0f` | Half-size of the wireframe cube |
| `u`/`v` step | lines 61–62 | `0.05f` | Torus mesh density (smaller = more points) |
| `t` step | line 70 | `0.1f` | Cube-edge sampling density |
| `height`/`width` | lines 87–88 | `40`/`80` | Output resolution |
| Torus spin | lines 101–103 | `0.2/0.5/0.7` | Rotation speed multipliers per axis |
| Cube spin | lines 105–107 | `-0.4/0.8/-0.2` | Independent rotation multipliers |
| `angle` increment | line 150 | `0.05f` | Animation speed |
| Sleep | line 151 | `10 ms` | Frame delay |
| Light | line 123 | `(0,1,1)` | Initial light direction |

Note that the torus and cube rotation speeds are **not** independent of the
frame rate: they are fixed increments per rendered frame, not per second.

## Notes and limitations

* **No anti-aliasing.** Each sample is one character; thin cube edges can
  look dotted and silhouettes are blocky.
* **Orthographic only.** There is no perspective, so the scene looks flat
  compared with a camera-based projection.
* **No clipping except screen bounds.** The scene is sized to fit in 80 × 40
  at the current constants; enlarging the objects, the scale factors, or
  rotating them differently can push points off-screen (they are then
  dropped, not clipped).
* **The screen is fully cleared every frame.** That can flicker on some
  terminals. Since every cell is overwritten anyway, printing only the
  cursor-home sequence (`\033[H`) would be enough and would flicker less.
  Hiding the cursor (`\033[?25l`) is another nice touch.
* **The terminal is left cleared** on exit, and the cursor is not restored;
  the program has no cleanup path because it never leaves the loop.
* **Fixed size.** If the terminal is narrower than 80 columns or shorter
  than 41 rows, the output wraps or scrolls and the animation breaks.
* **`bits/stdc++.h` is non-portable**; see the build section for the
  portable replacement.
* **Unreachable `return 0`** at the end of `main`.
* **Unused/duplicate includes** (`<cmath>`, `<vector>` are already pulled in
  by `bits/stdc++.h`).
* **No command-line options.** All parameters are compile-time constants.
* **The cube "normals" are radial**, not true face normals, so the lighting
  on the cube is artistic rather than physical.
* **The torus loop stops at 6.28**, not 2π, leaving a hairline seam.

## Possible improvements

1. Replace `6.28f` with a proper `2π` and compute the sample count from the
   desired resolution.
2. Hoist the light calculation out of the vertex loop; build the rotation
   matrices for the light once per frame.
3. Clamp the shade index: `clamp(int(luminance * 11.0f), 0, 11)`.
4. Change `normalize` to take `const Point&` and guard against zero length.
5. Use an `enum class` (or `bool`) instead of the magic `int label`.
6. Buffer the whole frame in a `std::string` and write it with a single
   `cout.write(...)` instead of 3,200 individual insertions plus 40 newlines.
7. Hide the cursor and use the alternate screen buffer
   (`\033[?1049h` / `\033[?1049l`) so the user's terminal is restored on
   exit; add a signal handler or RAII guard for `Ctrl+C`.
8. Use `std::chrono::steady_clock` and `sleep_until` for a stable frame rate,
   or scale movement by elapsed time so animation speed is independent of
   rendering speed.
9. Add perspective projection with a camera distance for a stronger 3D look.
10. Add ambient light and distance-based depth shading to make the dark side
    less flat.
11. Wrap the animation angle with `fmod` to avoid `float` precision drift.
12. Reserve `vertices` capacity before the generation loops.

## Credits

The character ramp and the general "z-buffered ASCII 3D" idea are inspired by
Andy Sloane's classic [`donut.c`](https://www.a1k0n.net/2011/07/20/donut-math.html).
This implementation uses its own C++ structures, a parametric torus mesh, a
separate wireframe cube, and a moving light.
