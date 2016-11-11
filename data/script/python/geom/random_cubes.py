# Python interface (beta)
import matrixoptimizer as mo
import random

rnd = random.Random(23)
def vec_gauss(r, mean, sigma):
	return mo.Vec(r.gauss(mean, sigma), r.gauss(mean, sigma), r.gauss(mean, sigma))

# get the current geometry instance
g = mo.geometry()

g.clear()

for i in range(0,400):
	box = mo.Geometry()
	box.set_cur_attribute("a_param", vec_gauss(rnd, 0.5, 0.5))
	box.add_box(rnd.uniform(0.1, 4.))
	#box.rotate
	box.translate(vec_gauss(rnd, 0., 10.))
	g.add_geometry(box)
	