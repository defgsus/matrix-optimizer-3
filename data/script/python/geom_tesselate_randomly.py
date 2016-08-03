# Python interface (beta)

import matrixoptimizer as mo
import random;
rnd = random.Random(42)

g = mo.geometry()

g.scale(3)

for x in range(g.num_triangles()*3):
	g.tesselate_triangle(rnd.randint(0,g.num_triangles()-1))

g.convert_to_lines()