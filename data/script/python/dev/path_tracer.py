# Press F1 for help
import matrixoptimizer as mo


# camera speed
meter_per_sec = 4.
 
# draws the path in the timelines as lines into a geometry
def add_path(geom, tl, tlup, tllook):
	for i in range(100):
		geom.add_line(tl.value(i), tl.value(i+1));
		geom.add_point(tl.value(i));
  
	for i in range(100):
		geom.add_line(tl.value(i), tl.value(i) + tlup.value(i));
		#geom.add_line(tl.value(i), tllook.value(i) + tlup.value(i));
	
def debug_path(tl, tlup, tllook):
	obj = mo.scene().find_by_name("PathDebug");
	g = mo.Geometry()
	add_path(g, tl, tlup, tllook)
	obj.set_geometry(g)

def store_timeline_in(obj, tl):
	print("store timeline " + str(tl) + " in " + obj.name())
	obj.set_timeline(tl) 

def store_timeline(name, tl):
	obj = mo.scene().find_by_name(name + "x")
	store_timeline_in(obj, tl.get_timeline(0))	
	obj = mo.scene().find_by_name(name + "y")
	store_timeline_in(obj, tl.get_timeline(1))	
	obj = mo.scene().find_by_name(name + "z")
	store_timeline_in(obj, tl.get_timeline(2))	
 
def create_path():
	print("\nCreating path through geometry");
	
	# get the geometry we want to trace
	obj = mo.scene().find_by_name("Cubes");
	geom = obj.get_geometry()
	print( str(obj) + " : " + str(geom) )	
	obj = mo.scene().find_by_name("RoomModel");
	geom.add_geometry( obj.get_geometry() )
	print( str(obj) + " : " + str(geom) )	

	tl = mo.Timeline(3)
	tlup = mo.Timeline(3)
	tldir = mo.Timeline(3)

	# start

	pos = mo.Vec(1,3,-5) 
	dir = mo.Vec(0.1,0.4,1).normalized() 
	len = 4.;
	
	tl.add(-1, pos)#, mo.Timeline.SPLINE6)
	tl.add(0, pos)
	#tl.add(5, pos+dir)
	#tl.add(10, pos+dir*3.)
	#tl.add(100, pos+dir*100.)

	# search

	for i in range(2000):
		if (time > 120) break

		hitt = geom.intersection(pos, dir);	
	
		// make a few tries to find a non-hit position
		for hits in range(50):
			#dir = normalize(dir + 0.9*(rnd.vec3(-1,1)-dir));
			hitt = geom.intersection(pos, dir);	
			
		for (; hits<50 && hit && distance(pos, hitpos) <= len; ++hits)
		{
			// aim somewhere else
			hit = obj.intersects(pos,dir, hitpos);	
		}

		if (hits>=50) 
		{
			print("got stuck");
			break;
		}

		// move forward
		float actlen = 1. / (1.+0.1*hits);
		pos = pos + actlen * dir;

		// count time and add timeline point
		time += actlen / meter_per_sec;
		if (time >= tl.end() + meter_per_sec * 0.2)
		{
			tl.add(time, pos);
//			if (time >= tlup.end() + meter_per_sec * 2)
//				tlup.add(time, normalize(pos));
		}
		// go towards goal pos
		float lp = length(pos);
		vec3 goal = ((pos + dir*len)/lp) * (16 - lp);
		dir += 0.042318 * (normalize(goal-pos) - dir);

		// randomize direction
		dir = dir + 0.1 * (rnd.vec3(-1,1) - dir);
		dir = normalize(dir);
		
		if (i % 50 == 0)
			print("time = " + time);
	}
	
	t = geom.intersection(pos, dir)
	print(t)


	# finish

	#store_timeline("path", tl)
 
	debug_path(tl, tlup, tldir)

create_path()