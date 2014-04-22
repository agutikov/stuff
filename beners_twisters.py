#!/usr/bin/python


from pprint import pprint
import sys
import random
import pygame
from collections import namedtuple
import math

# http://pygame.org/project-gradients-307-491.html
# ~/.local/lib64/python3.2/site-packages/
import gradients


'''

http://www.philsci.univ.kiev.ua/biblio/vrema-haos.html

'''

############################### Settings #######################################



gradient_radius = 3
gradient_center_opacity = 255
gradient_edge_opacity = 0

grid_size = 20
grid_width = grid_size*2
grid_height = grid_size*2

# integer multiply of diameter in cell
x_spacing = 1.5
y_spacing = 2

boiler_width = int(2*gradient_radius*grid_width*x_spacing)
boiler_height = int(2*gradient_radius*grid_height*y_spacing)

window_width = boiler_width
window_height = boiler_height

MODEL_SCALE=100.0


T_MAX =  573.15 * MODEL_SCALE
T_MIN =  273.15 * MODEL_SCALE


ATOM_NUMBER = grid_width*grid_height


print("ATOM_NUMBER =",ATOM_NUMBER)

# 3*k/m
KM=0.3

g_acc=5*9.81/MODEL_SCALE

DT=1

show_density=False
max_fps = 80

test_mode=False

########################### Model ##############################################

class Point:
	x=0
	y=0
	def __init__ (self, _x, _y):
		self.x = _x
		self.y = _y

class Atom:
	pos=Point(0,0)
	vel=Point(0,0)
	acc=Point(0,0)
	t=0
	def __init__ (self, _p, _v, _t, _a):
		self.pos=_p
		self.vel=_v
		self.t=_t
		self.acc=_a

def sign (x):
	return 1 if x >= 0 else -1

'''
t - temperature
'''
def t_to_vel(t, vel):
	x2 = vel.x*vel.x
	y2 = vel.y*vel.y
	s = x2 + y2
	qx = x2 / s if s>0 else 0
	qy = y2 / s if s>0 else 0
	return Point(sign(vel.x)*math.sqrt(t*KM*qx), sign(vel.y)*math.sqrt(t*KM*qy))

def vel_to_t(vel):
	return (vel.x*vel.x + vel.y*vel.y)/KM


model_width = boiler_width*MODEL_SCALE
model_height = boiler_height*MODEL_SCALE

model_radius = gradient_radius*MODEL_SCALE


########################### Model Init #########################################

pos_x = model_radius
pos_y = model_radius

def new_grid(w, h):
	g = []
	for iy in range(0, h):
		g.append([])
		for ix in range(0, w):
			g[-1].append([])
	return g

grid = new_grid(grid_width, grid_height)


def append_atom(grid, atom):
	ix = int(atom.pos.x/MODEL_SCALE/(2*gradient_radius*x_spacing))
	iy = int(atom.pos.y/MODEL_SCALE/(2*gradient_radius*y_spacing))
	grid[iy][ix].append(atom)

def create_test_atom(ix, iy, dx, dy):
	x = ix*2*gradient_radius*x_spacing + gradient_radius*x_spacing
	y = iy*2*gradient_radius*y_spacing + gradient_radius*y_spacing
	x *= MODEL_SCALE
	y *= MODEL_SCALE
	vel = Point(dx, dy)
	return Atom(Point(x, y), vel, vel_to_t(vel), Point(0, 0))

if test_mode:
	# g_acc = 0
	show_density = True
	max_fps = 3

	append_atom(grid, create_test_atom(1.5, 0.5, -100, 100))
	append_atom(grid, create_test_atom(0.5, 1.5, 100, -100))

else:
	for iy,row in enumerate(grid):
		for ix,cell in enumerate(row):
			x = ix*2*gradient_radius*x_spacing + gradient_radius*x_spacing
			y = iy*2*gradient_radius*y_spacing + gradient_radius*y_spacing
			x *= MODEL_SCALE
			y *= MODEL_SCALE

			t = float(random.randrange(int(T_MIN), int(T_MAX)))
			qx = float(random.randrange(-10, 10))
			qy = float(random.randrange(-10, 10))

			atom = Atom(Point(x, y), t_to_vel(t, Point(qx, qy)), t, Point(0, 0))

			cell.append(atom)

###################### Translating from Model into Graphics#####################


def model_pos_to_graphics (p):
	# потому что отсчёт абсциссы сверху-вниз
	y = model_height - p.y
	x = p.x
	# -gradient_radius потому что координата рисования - координата верхнего левого угла
	x = x/MODEL_SCALE - gradient_radius
	y = y/MODEL_SCALE - gradient_radius
	return (int(x), int(y))


def model_t_to_graphics (t):
	T=T_MAX-T_MIN
	t_min = T_MIN - T
	t_max = T_MAX + T
	if t < t_min:
		t = t_min
	if t > t_max:
		t = t_max

	c = (t-t_min)/(t_max-t_min)*255

	if c < 0:
		print("AHTUNG! COLOR < 0 : ", c)
		return 0
	if c > 255:
		print("AHTUNG! COLOR > 255 : ", c)
		return 255

	return int(c)

########################## Prepare Rendering ###################################


margin_h = (window_width - boiler_width)/2
margin_v = (window_height - boiler_height)/2

pygame.init()
pygame.font.init()

fpsClock = pygame.time.Clock()
font = pygame.font.SysFont("LiberationMono-Regular", 30)

BLACK = pygame.Color(0,0,0, 255)
WHITE = pygame.Color(255, 255, 255, 255)
GREEN = pygame.Color(0, 255, 0, 255)
RED = pygame.Color(255, 0, 0, 255)
BLUE = pygame.Color(0, 0, 255, 255)


W = pygame.display.set_mode((window_width, window_height))

atom_colors = []

for r in range(0, 255+1):
	atom_colors.append(
		gradients.radial(
			gradient_radius,
			pygame.Color(r,0,255-r,gradient_center_opacity),
			pygame.Color(r,0,255-r,gradient_edge_opacity)
		)
	)

S = pygame.Surface((boiler_width, boiler_height), pygame.SRCALPHA)

############################ Modelling and Rendering ###########################


# http://habrahabr.ru/post/105882/

model_d2 = 4*model_radius*model_radius

def distance2 (a, b):
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)

def scal (a, b):
	return a.x*b.x + a.y*b.y

def norm (a):
	s = math.sqrt(scal(a, a))
	if s != 0:
		return Point(a.x/s, a.y/s)
	else:
		return Point(0, 0)

def proj (a, b):
	s = scal(a, b)/scal(b, b)
	return Point(b.x*s, b.y*s)

def vsum (a, b):
	return Point(a.x + b.x, a.y + b.y)

def vdif (a, b):
	return Point(a.x - b.x, a.y - b.y)

def vmul (a, k):
	return Point(a.x*k, a.y*k)

def invert (a):
	return Point(-a.x, -a.y)

def orto (a):
	return Point(-a.y, a.x)

'''
TODO: convert kinetic energy into potential - add acceleration eqvivalent to potential energy
and substract impulse eqvivalent to kinetic energy
'''

def impact (atom1, atom2, dt):
	d2 = distance2(atom1.pos, atom2.pos)
	if d2 <= model_d2:

#		print("before:", atom1.vel.x, atom1.vel.y, atom2.vel.x, atom2.vel.y)
#		print("before:", atom1.acc.x, atom1.acc.y, atom2.acc.x, atom2.acc.y)

		# vector from center of atom1 to center of atom2 goes through impact place
		s12 = vdif(atom2.pos, atom1.pos)
		s21 = invert(s12)

#		print(s12.x, s12.y)

		# normal vectors to impact line - atoms will exchange velocity projections on this vector
		n12 = norm(s12)
		n21 = invert(n12)

#		print(n12.x, n12.y, n21.x, n21.y)

		# velocity of atom1 in projection on vector from atom1 to atom2 - then goes to atom2
		v1n12 = proj(atom1.vel, s12)
		# velocity of atom2 in projection on vector from atom2 to atom1 - then goes to atom1
		v2n21 = proj(atom2.vel, s21)
		# left velocity - saved
		v1p = vdif(atom1.vel, v1n12)
		v2p = vdif(atom2.vel, v2n21)

		# exchange impulse
		atom1.vel = vsum(v1p, v2n21)
		atom2.vel = vsum(v2p, v1n12)

		d = math.sqrt(d2)
		r = model_radius - d/2
		r += 1.0

		atom1.pos = vsum(atom1.pos, vmul(n21, r))
		atom2.pos = vsum(atom2.pos, vmul(n12, r))

#		print("after:", atom1.vel.x, atom1.vel.y, atom2.vel.x, atom2.vel.y)
#		print("after:", atom1.acc.x, atom1.acc.y, atom2.acc.x, atom2.acc.y)

		'''
		Ek1 = scal(v1n12, v1n12)/2
		Ek2 = scal(v2n21, v2n21)/2

#		print(v1n12.x, v1n12.y, v2n21.x, v2n21.y)

		r2 = model_d2 - d2
		R = r2*r2

		Ep1 = 0
		Ep2 = 0

		a1 =
		a2 =

		acc1 = vmul(n21, a1)
		acc2 = vmul(n12, a2)

		atom1.acc = vsum(atom1.acc, acc1)
		atom2.acc = vsum(atom2.acc, acc2)
		'''





def run_model (grid, dt):
	result_grid = new_grid(grid_width, grid_height)
	for iy,row in enumerate(grid):
		for ix,cell in enumerate(row):
			for atom in cell:

				# Dynamics
				atom.acc = Point(0, -g_acc)

				for pix in [ix-1, ix, ix+1]:
					for piy in [iy-1, iy, iy+1]:
						if pix >= 0 and piy >= 0 and pix < len(row) and piy < len(grid):
							for a in grid[piy][pix]:
								if not a is atom:
									impact(atom, a, dt)
				# Kinematics
				atom.vel.x += atom.acc.x*dt
				atom.vel.y += atom.acc.y*dt
				atom.t = vel_to_t(atom.vel)

				atom.pos.x += atom.vel.x*dt
				atom.pos.y += atom.vel.y*dt

				# borders (also kinematics)
				if atom.pos.x < model_radius:
					atom.pos.x = model_radius+1
					atom.vel.x = -atom.vel.x
				if atom.pos.x > model_width-model_radius:
					atom.pos.x = model_width-model_radius-1
					atom.vel.x = -atom.vel.x

				if atom.pos.y < model_radius:
					atom.pos.y = model_radius+1
					atom.vel.y = -atom.vel.y
					atom.t = T_MAX
					atom.vel = t_to_vel(atom.t, atom.vel)
				if atom.pos.y > model_height-model_radius:
					atom.pos.y = model_height-model_radius-1
					atom.vel.y = -atom.vel.y
					atom.t = T_MIN
					atom.vel = t_to_vel(atom.t, atom.vel)

				# new cell
				ix = int(atom.pos.x/MODEL_SCALE/(2*gradient_radius*x_spacing))
				iy = int(atom.pos.y/MODEL_SCALE/(2*gradient_radius*y_spacing))

				result_grid[iy][ix].append(atom)
	return result_grid



def render (grid):

	if show_density:
		max_atoms_in_cell=x_spacing*y_spacing*2
		for iy,row in enumerate(grid):
			for ix,cell in enumerate(row):
				c = int(len(cell)*255/max_atoms_in_cell)
				c = 255-c if c <= 255 else 0
				pygame.draw.rect(S, (c,c,c),
					((ix*2*gradient_radius*x_spacing, (grid_height-iy-1)*2*gradient_radius*y_spacing),
					(2*gradient_radius*x_spacing, 2*gradient_radius*y_spacing)))
	else:
		S.fill(WHITE)

	for row in grid:
		for cell in row:
			for atom in cell:
				S.blit(atom_colors[model_t_to_graphics(atom.t)], model_pos_to_graphics(atom.pos))

	W.blit(S, (margin_h, margin_v))

	str = "%.1f fps, %.2f/%.2f" % (last_fps, last_model_time, last_render_time)
	label = font.render(str, 1, GREEN)
	W.blit(label, (0, 0))

	pygame.display.update()



iter_count = 0
model_time = 0
render_time = 0

last_model_time = 0
last_render_time = 0
last_fps = 0

while True:
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			sys.exit(0)
#		else:
			# print(event)

	start = pygame.time.get_ticks()
	grid = run_model(grid, DT)
	model_time += pygame.time.get_ticks() - start

	start = pygame.time.get_ticks()
	render(grid)
	render_time += pygame.time.get_ticks() - start

	iter_count += 1
	if iter_count == max_fps:
		last_model_time = model_time/iter_count
		last_render_time = render_time/iter_count
		model_time = 0
		render_time = 0
		iter_count = 0
		last_fps = fpsClock.get_fps()

	fpsClock.tick(max_fps)








