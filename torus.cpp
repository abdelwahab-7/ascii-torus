#include <bits/stdc++.h>
#include <cmath>
#include <vector>
using namespace std;

struct Point {
  float x, y, z;
};

struct Vertex {
  Point p;
  Point normal;
  int label;
};
struct Tmatrix{
  float mat[3][3];
};

Point operator*(const Tmatrix& m, const Point& p){
  return {
    m.mat[0][0] * p.x + m.mat[0][1] * p.y + m.mat[0][2] * p.z,
    m.mat[1][0] * p.x + m.mat[1][1] * p.y + m.mat[1][2] * p.z,
    m.mat[2][0] * p.x + m.mat[2][1] * p.y + m.mat[2][2] * p.z,
  };
}

float operator*(const Point& u, const Point& w){
  return u.x * w.x + u.y * w.y + u.z * w.z;
}

array<float, 2> get_angles(float angle){
  return {cos(angle), sin(angle)};
}

Point normalize(Point& p){
  float len = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
  return {p.x / len, p.y / len, p.z / len};
}

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

const int R = 7;
const int r = 4;
const string shades = ".,-~:;=!*#$@";

int main(){
  vector<Vertex> vertices;
  for (float u = 0.0f; u <= 6.28f; u += 0.05f){
    for (float v = 0.0f; v <= 6.28f; v += 0.05f){
      Point p = {(R + r * cos(v)) * cos(u), (R + r * cos(v)) * sin(u), r * sin(v)};
      Point norm = {cos(v) * cos(u), cos(v) * sin(u), sin(v)};
      vertices.push_back({p, norm, 0});
    }
  }

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

      int screenX = (int)(rotated_p.x * 2) + (width / 2);
      int screenY = (int)(rotated_p.y * 1) + (height / 2);

      Point light = {0.0f, 1.0f, 1.0f};
      light = normalize(light);

      Tmatrix light_rot_y = Ry(angle * 2.0f);
      light = light_rot_y * light;

      float luminance = rotated_norm * light;

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
    }
    cout << "\033[2J\033[H";
    for (int i = 0; i < height; i++){
      for (int j = 0; j < width; j++){
        cout << screen[i][j];
      }
      cout << '\n';
    }
    angle += 0.05f;
    this_thread::sleep_for(chrono::milliseconds(10));
  }
  return 0;
}
