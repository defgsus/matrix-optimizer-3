# Python interface (beta)

import matrixoptimizer as mo
import random;

g = mo.geometry()

gl = g.copy()
gl.convert_to_lines()
gl.scale(0.99)

g.set_cur_color(1,1,1,1)
g.add_geometry_only(gl, line=True)