#!/usr/bin/env python3


from pprint import pprint
import sys
import random
import pygame
from collections import namedtuple
import math

#TODO: check conservation laws: energy, impule
#TODO: numpy
#TODO: astronomical units: distance: ae, le, pc; time: ey(Earth year); mass: me(Earth), ms(sun)
#TODO: args presets
#TODO: telemetry prints
#TODO: telemetry overlay: time, coordinates, linear velocity and projections, tangential and radial relative velocity, distance, angular velocity, angle, acceleration and acc projections
#TODO: rotation, momentum conservation
#TODO: collision detection
#TODO: connected bodies, disconnection
#TODO: visualisation-dependant time acceleration vs. max time step + max distance step (velocity*time_step)
#TODO: other ways of modelling mechanical systems, like numerical equeations solving...
#TODO: focus selection
#TODO: multiple windows with different focus: (0,0), body_1, body_2, rotation
#TODO: cmdline
#TODO: manual scale and move viewport
#TODO: coordinate mesh legend
#TODO: multiple bodies and 3d

class Point:
	x=0
	y=0
	def __init__ (self, _x, _y):
		self.x = _x
		self.y = _y

def distance2(a, b):
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)

def distance(a, b):
	return math.sqrt(distance2(a, b))

def scal(a, b):
	return a.x*b.x + a.y*b.y

def abs(a):
	return math.sqrt(scal(a, a))

def norm(a):
	s = abs(a)
	if s != 0:
		return Point(a.x/s, a.y/s)
	else:
		return Point(0, 0)

def proj(a, b):
	s = scal(a, b)/scal(b, b)
	return Point(b.x*s, b.y*s)

def vsum(a, b):
	return Point(a.x + b.x, a.y + b.y)

def vsub(a, b):
	return Point(a.x - b.x, a.y - b.y)

def vmul(a, k):
	return Point(a.x*k, a.y*k)

def invert(a):
	return Point(-a.x, -a.y)

def orto(a):
	return Point(-a.y, a.x)


max_path_len = 1000

class Body:
	def __init__ (self, _m, _r, _p, _v, _a):
		self.m = _m
		self.r = _r
		self.pos=_p
		self.vel=_v
		self.acc=_a
		self.path=[_p]

	def push_pos_in_path(self, distance_threshold):
		d = distance(self.pos, self.path[-1])
		if d >= distance_threshold:
			self.path.append(self.pos)
			if len(self.path) > max_path_len:
				self.path = self.path[1:]


D = 149597870700
#D = 14959787070*2
R0 = 20000000000
R1 = 10000000000
V0 = 0
#V0 = 10000
V1 = 29783
#V1 = 35000
M0 = 2*10**30
#M0 = 6*10**24
M1 = 6*10**24
#M1 = 1*10**30
G = 6.67*10**-11

window_width_px = 800
window_height_px = 600
initial_distance_px = 100
initial_grid_size_px = 80
initial_px_size = D/initial_distance_px

autoscale_grid = False
autoscale_viewport = False
follow_body_0 = True

# max and min speed of bodies in visualisation, pixels per second
max_linear_velocity_px = 200
min_linear_velocity_px = 10
autoscale_time = True
# time acceleration in model, seconds per second, initial = 30 days/s
initial_time_acceleration = 3600*24*30

max_fps = 1000

def render_elapsed(elapsed_s):
	y = elapsed_s // (365*24*3600)
	d = (elapsed_s % (365*24*3600)) // (24*3600)
	h = (elapsed_s % (24*3600)) // 3600
	m = (elapsed_s % 3600) // 60
	s = elapsed_s - (elapsed_s // 60) * 60
	return (("%dy " % y) if y > 0 else "") + (("%dd " % d) if d > 0 else "") + ("%02d:%02d:%02.3f" % (h, m, s))

class Model:
	def __init__(self):
		self.time_acc = initial_time_acceleration
		self.elapsed = 0

		self.update_count = 0
		self.update_elapsed = 0

		self.px_size = initial_px_size

		self.viewport_width = window_width_px*self.px_size
		self.viewport_height = window_height_px*self.px_size
		self.viewport_center = Point(0, 0)

		self.grid_size = initial_grid_size_px*self.px_size

		self.body_0 = Body(M0, R0, Point(0,0), Point(-V0,-V0), Point(0,0))
		self.body_1 = Body(M1, R1, Point(0, D), Point(V1,0), Point(0,0))

	def reset_avg_metrics(self):
		self.update_count = 0
		self.update_elapsed = 0

	def run(self, dt_ms):
		d2 = distance2(self.body_0.pos, self.body_1.pos)
		s = vsub(self.body_0.pos, self.body_1.pos)
		n = norm(s)

		model_time_diff = self.time_acc*dt_ms/1000
		self.elapsed += model_time_diff
		self.update_elapsed += model_time_diff
		self.update_count += 1

		self.body_0.push_pos_in_path(self.px_size)
		self.body_0.acc = vmul(invert(n), G*self.body_1.m/d2)
		self.body_0.vel = vsum(self.body_0.vel, vmul(self.body_0.acc, model_time_diff))
		self.body_0.pos = vsum(self.body_0.pos, vmul(self.body_0.vel, model_time_diff))

		self.body_1.push_pos_in_path(self.px_size)
		self.body_1.acc = vmul(n, G*self.body_0.m/d2)
		self.body_1.vel = vsum(self.body_1.vel, vmul(self.body_1.acc, model_time_diff))
		self.body_1.pos = vsum(self.body_1.pos, vmul(self.body_1.vel, model_time_diff))

		if follow_body_0:
			self.viewport_center = self.body_0.pos

		if autoscale_time:
			v0_px = abs(self.body_0.vel) / self.px_size * self.time_acc
			v1_px = abs(self.body_1.vel) / self.px_size * self.time_acc
			#print(v1_px, v0_px)
			max_v_px = max(v0_px, v1_px)
			if max_v_px > max_linear_velocity_px:
				self.time_acc /= 2
			elif max_v_px < min_linear_velocity_px:
				self.time_acc *= 2


		if autoscale_viewport:
			def madx(p0, p1, p2):
				return max(abs(p1.x - p0.x), abs(p2.x - p0.x))
			def mady(p0, p1, p2):
				return max(abs(p1.y - p0.y), abs(p2.y - p0.y))

			self.viewport_width = max(self.viewport_width, 3*madx(self.viewport_center, self.body_0.pos, self.body_1.pos))
			self.viewport_height = max(self.viewport_height, 3*mady(self.viewport_center, self.body_0.pos, self.body_1.pos))

			self.px_size = max(self.viewport_width/window_width_px, self.viewport_height/window_height_px)

			self.viewport_width = 6*min(self.viewport_width/6, madx(self.viewport_center, self.body_0.pos, self.body_1.pos))
			self.viewport_height = 6*min(self.viewport_height/6, mady(self.viewport_center, self.body_0.pos, self.body_1.pos))

			self.px_size = max(self.viewport_width/window_width_px, self.viewport_height/window_height_px)

			self.viewport_width = window_width_px*self.px_size
			self.viewport_height = window_height_px*self.px_size

		if autoscale_grid:
			if self.grid_size/self.px_size < initial_grid_size_px/2:
				self.grid_size *= 2
			elif self.grid_size/self.px_size > initial_grid_size_px*2:
				self.grid_size /= 2

	def get_point_pos_in_viewport(self, p):
		return (int(  (p.x - self.viewport_center.x)/self.px_size + window_width_px/2 ),
		        int( -(p.y - self.viewport_center.y)/self.px_size + window_height_px/2))

	def get_telemetry(self):
		return [
			"time: elapsed=%s; speed=(%dd %dh %ds)/1s; dt=%.3fs/tick" % (render_elapsed(self.elapsed),
			                         self.time_acc // (3600*24), 
			                         (self.time_acc % (3600*24)) // 3600,
									 self.time_acc % 3600,
									 self.update_elapsed / self.update_count)
			]


model = Model()

pygame.init()
pygame.font.init()

fpsClock = pygame.time.Clock()
font_size = 24
font = pygame.font.SysFont("LiberationMono-Regular", font_size)

BLACK = pygame.Color(0,0,0, 255)
WHITE = pygame.Color(255, 255, 255, 255)
GRAY = pygame.Color(100, 100, 100, 255)
GREEN = pygame.Color(0, 255, 0, 255)
RED = pygame.Color(255, 0, 0, 255)
BLUE = pygame.Color(0, 0, 255, 255)

W = pygame.display.set_mode((window_width_px, window_height_px))
S = pygame.Surface((window_width_px, window_height_px), pygame.SRCALPHA)



def render_body(surface, body, color):
	pygame.draw.circle(surface, color, model.get_point_pos_in_viewport(body.pos), int(body.r / model.px_size), 0)
	if len(body.path) > 1:
		pygame.draw.lines(surface, WHITE, False, [model.get_point_pos_in_viewport(p) for p in body.path], 1)

def render_grid(surface):
	left_border_x = model.viewport_center.x - window_width_px/2*model.px_size
	line_x = math.ceil(left_border_x/model.grid_size)*model.grid_size
	while line_x < (left_border_x + window_width_px*model.px_size):
		line_x_px = (line_x - model.viewport_center.x)/model.px_size + window_width_px/2
		pygame.draw.line(surface, GRAY, (line_x_px, 0), (line_x_px, window_height_px))
		line_x += model.grid_size

	top_border_y = model.viewport_center.y - window_height_px/2*model.px_size
	line_y = math.ceil(top_border_y/model.grid_size)*model.grid_size
	while line_y < (top_border_y + window_height_px*model.px_size):
		line_y_px = -(line_y - model.viewport_center.y)/model.px_size + window_height_px/2
		pygame.draw.line(surface, GRAY, (0, line_y_px), (window_width_px, line_y_px))
		line_y += model.grid_size


overlay_render_period_ms = 1000
overlay_render_ts = pygame.time.get_ticks()
overlay_text = ''

def render_overlay():
	global overlay_render_ts
	global overlay_text

	if pygame.time.get_ticks() - overlay_render_ts > overlay_render_period_ms:
		overlay_render_ts = pygame.time.get_ticks()
		overlay_text = model.get_telemetry()
		overlay_text.append("%.1f fps" % fpsClock.get_fps())
		model.reset_avg_metrics()

	for index, line in enumerate(overlay_text):
		overlay_label = font.render(line, 1, GREEN)
		W.blit(overlay_label, (0, int(index*font_size/4*3)))


def render():
	S.fill(BLACK)

	render_grid(S)

	render_body(S, model.body_0, RED)
	render_body(S, model.body_1, BLUE)

	W.blit(S, (0, 0))

	render_overlay()

	pygame.display.update()


prev_time = pygame.time.get_ticks()

while True:
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			sys.exit(0)
#		else:
			# print(event)

	time = pygame.time.get_ticks()
	model.run(time - prev_time)
	prev_time = time

	render()

	fpsClock.tick(max_fps)



