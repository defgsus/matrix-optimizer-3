# Python interface (beta)
import matrixoptimizer as mo
import math
import random


def make_cone(g, p1, r1, p2, r2):
	g.set_cur_attribute("a_cone_a", mo.Vec(p1.x, p1.y, p1.z, r1))
	g.set_cur_attribute("a_cone_b", mo.Vec(p2.x, p2.y, p2.z, r2))
	tilt = -(p2-p1).normalized()
	n = 4
	pts1 = []
	pts2 = []
	for i in range(n):
		t = i * 3.14159265 * 2. / n
		v = mo.Vec(math.sin(t), 0., math.cos(t))
		v.y = tilt.dot(v)
		v1 = p1 + v * r1
		gidx = g.add_vertex(v1)
		pts1.append(gidx)
		v1 = p2 + v * r2
		gidx = g.add_vertex(v1)
		pts2.append(gidx)
	for i in range(len(pts1)):
		i1 = (i + 1) % len(pts1)
		g.add_triangle(pts1[i], pts1[i1], pts2[i1])
		g.add_triangle(pts1[i], pts2[i1], pts2[i])
	
		

class Tree:
	def __init__(self, seed = 23, pos = mo.Vec(0,0,0), id = 0):
		self.rnd = random.Random(seed)
		#help(self.rnd)
		self.geom = mo.Geometry()
		self.geom.set_shared(False)
		self.points = dict()
		self.points[0] = pos
		self.rads = dict()
		self.rads[0] = 0.1
		self.geom.set_cur_attribute("a_id", id)
		self.geom.set_cur_color(.5,.5,.5,1.)
		self.next_level = 0
		self.grow_prob = .9
		self.split_prob = .4
		self.rad_decay = .9

	def grow(self, prev, rad2):
		#print("grow " + str(prev))
		v1 = self.points[prev]
		v2 = v1 + 0. 
		v2.y = v2.y + self.rnd.uniform(.2,4)
		v2.x = v2.x + self.rnd.uniform(-1,1)
		v2.z = v2.z + self.rnd.uniform(-1,1)
		self.next_level = self.next_level + 1
		level = self.next_level
		self.points[level] = v2
		self.rads[level] = rad2
		make_cone(self.geom, v1, self.rads[prev], v2, rad2)
		
		if level > 7 and self.rnd.random() < self.grow_prob:
			return
		if rad2 > 0.02:
			self.grow(level, rad2*self.rad_decay)
			if self.rnd.random() < self.split_prob:
				#print(" split " + str(level))
				self.grow(level, rad2*.7)
		

g = mo.geometry()
g.set_shared(False)

#conny = Conny(); conny.test(); g.add_geometry(conny.geom)

def make_trees(g):
	rnd = random.Random(777)
	rad = 30
	for i in range(150):
		p = mo.Vec(rnd.uniform(-rad,rad), 0, rnd.uniform(-rad,rad))
		tree = Tree(seed = rnd.randint(1000,10000000), pos=p, id = i); 
		tree.split_prob = rnd.uniform(0.1, .9)
		tree.rad_decay = rnd.uniform(0.5, .98)
		tree.grow(0, rnd.uniform(0.05, .2)); 
		g.add_geometry(tree.geom)

make_trees(g)

print(g)
#help(random)