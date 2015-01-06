#version 330

// --- input from vertex shader ---
in vec4 v_color;

// output to rasterizer
out vec4 color;

void main()
{
    // final color
    color = vec4(clamp(v_color, 0.0, 1.0));
}
