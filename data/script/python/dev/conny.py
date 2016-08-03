# Python interface (beta)
import matrixoptimizer as mo
import math
import random

class Conny:
	def __init__(self):
		self.geom = mo.Geometry()
		self.geom.set_cur_color(.5,.5,.5,1.)
		self.points = dict()
	
	def add_ring(self, idx, rad, pos, tilt):
		n = 9
		pts = []
		for i in range(n):
			t = i * 3.14159265 * 2. / n
			v = mo.Vec(math.sin(t), 0., math.cos(t))
			v.y = tilt.dot(v)
			v = pos + v * rad
			gidx = self.geom.add_vertex(v)
			pts.append(gidx)
		self.points[idx] = pts

	def connect_rings(self, idx1, idx2):
		pts1 = self.points[idx1]
		pts2 = self.points[idx2]
		for i in range(len(pts1)):
			i1 = (i + 1) % len(pts1)
			self.geom.add_triangle(pts1[i], pts1[i1], pts2[i1])
			self.geom.add_triangle(pts1[i], pts2[i1], pts2[i])

	def test(self):
		self.add_ring(0, .3, mo.Vec(0,0,0),  mo.Vec(0,0,0))
		self.add_ring(1, .2, mo.Vec(0,1,0),  mo.Vec(0,0,0))
		self.add_ring(2, .07, mo.Vec(-1,2,0), mo.Vec(.2,0,0))
		self.add_ring(3, .06, mo.Vec(1,3,0),  mo.Vec(-.4,0,0))
		self.connect_rings(0, 1)
		self.connect_rings(1, 2)
		self.connect_rings(1, 3)
		

class Tree:
	def __init__(self, seed = 23, pos = mo.Vec(0,0,0)):
		self.rnd = random.Random(seed)
		#help(self.rnd)
		self.conny = Conny()
		self.points = dict()
		self.points[0] = pos
		self.conny.add_ring(0, .2, self.points[0], mo.Vec(0,0,0))
		self.next_level = 0
		self.grow_prob = .9
		self.split_prob = .4
		self.rad_decay = .9

	def grow(self, prev, rad):
		#print("grow " + str(prev))
		v1 = self.points[prev]
		v2 = v1 + 0. 
		v2.y = v2.y + self.rnd.uniform(.2,4)
		v2.x = v2.x + self.rnd.uniform(-1,1)
		v2.z = v2.z + self.rnd.uniform(-1,1)
		tilt = -(v2-v1).normalized()
		self.next_level = self.next_level + 1
		level = self.next_level
		self.points[level] = v2
		self.conny.add_ring(level, rad, v2, tilt)
		self.conny.connect_rings(prev, level)
		#print("connect " + str(prev) + " " + str(level))
		#print("  " + str(v1) + " " + str(v2))
		if level > 7 and self.rnd.random() < self.grow_prob:
			return
		if rad > 0.02:
			self.grow(level, rad*self.rad_decay)
			if self.rnd.random() < self.split_prob:
				#print(" split " + str(level))
				self.grow(level, rad*.7)
		

g = mo.geometry()
g.set_shared(True)

#conny = Conny(); conny.test(); g.add_geometry(conny.geom)

def make_trees(g):
	rnd = random.Random()
	rad = 30
	for i in range(150):
		p = mo.Vec(rnd.uniform(-rad,rad), 0, rnd.uniform(-rad,rad))
		tree = Tree(seed = rnd.randint(1000,10000000), pos=p); 
		tree.split_prob = rnd.uniform(0.1, .9)
		tree.rad_decay = rnd.uniform(0.5, .98)
		tree.grow(0, rnd.uniform(0.05, .2)); 
		g.add_geometry(tree.conny.geom)

make_trees(g)

print(g)
help(random)