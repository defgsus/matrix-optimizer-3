#import matrixoptimizer as mo
import sys
import numpy as np
import gi as some

def print_object_tree(o, indent = ""):
	k = 0
	for x in o:
		print(indent + str(k) + ". " + x.name())
		print_object_tree(x, indent + "   ")
		k += 1

def print_object(o):
	print(str(o) + " [id:" + o.id_path() + "]")
	print_object_tree(o)

print("----")

o = mo.scene()

#list = [mo.Object("Group"), mo.Object("SequenceFloat")]
#o.add(list, 0)
#o.add(mo.Object("Group"), 0)

print_object(o)



"""
class Holla:
	def __init__(self):
		self.a = 23;

h = Holla()
h.a = 42
h.b = 12
print( h.__dict__ )

o.k = 12
"""

#newo = mo.Object("Group")
#newo.add(mo.Object("SequenceFloat"))
#newo.children(0).add(mo.Object("Camera"))
#o.add(newo)

"""
o = mo.scene()
# o.set_name("Hello")
print_object(o)
for x in o:
	for y in x:
		#y.set_name( y.name() + "_" )
		for z in y:
			print( z )
"""

"""
g = mo.Geometry()

print(g)

g.add_vertex(1, 2, 3)

print(g)

g2 = mo.Geometry(g)

g2.add_vertex(2, 3, 4)

print(g)
print(g2)
"""