#version 330 core

uniform int uCastShadows;

void main() {
  // Discard any fragment from an object that does not cast shadows.
  if (uCastShadows == 0)
    discard;
}
