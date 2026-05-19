#version 330 core

out vec4 FragColor;

uniform vec4 color;  // kolor linii/siatki

void main()
{
    FragColor = color;
}