#!/usr/bin/env python3


from pprint import pprint
import sys
import random
import pygame
from collections import namedtuple
import math

#TODO: numpy
#TODO: manual scale and move viewport, coordinate mesh legend, focus selection, ball size scale, rotation, collision, cmdline, args presets
#TODO: autoscale time
#TODO: multiple windows with different focus: (0,0), body_1, body_2, rotation
#TODO: telemetry overlay: time, coordinates, linear velocity and projections, tangential and radial relative velocity, distance, angular velocity, angle, acceleration and acc projections
#TODO: check conservation laws: energy, impule, momentum
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

def norm(a):
	s = math.sqrt(scal(a, a))
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

class Ball:
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


#R = 149597870700
R = 14959787070*4
V0 = 0
#V0 = 10000
#V1 = 29783
V1 = 35000
M0 = 2*10**30
#M0 = 6*10**24
M1 = 6*10**24
G = 6.67*10**-11

window_width_px = 800
window_height_px = 600
initial_distance_px = 100
initial_grid_size_px = 80
autoscale_grid = False
autoscale = False
follow = False

#TIME_ACCEL = 3600
TIME_ACCEL = 1000

max_fps = 10000



class Model:

	def __init__(self):
		self.time_accel = TIME_ACCEL

		self.px_size = R/initial_distance_px

		self.viewport_width = window_width_px*self.px_size
		self.viewport_height = window_height_px*self.px_size
		self.viewport_center = Point(0, 0)

		self.grid_size = initial_grid_size_px*self.px_size

		self.body_0 = Ball(M0, 20, Point(0,0), Point(-V0,-V0), Point(0,0))
		self.body_1 = Ball(M1, 10, Point(0, R), Point(V1,0), Point(0,0))


	def run(self, dt_ms):
		d2 = distance2(self.body_0.pos, self.body_1.pos)
		s = vsub(self.body_0.pos, self.body_1.pos)
		n = norm(s)

		self.body_0.push_pos_in_path(self.px_size)
		self.body_0.acc = vmul(invert(n), G*self.body_1.m/d2)
		self.body_0.vel = vsum(self.body_0.vel, vmul(self.body_0.acc, self.time_accel*dt_ms))
		self.body_0.pos = vsum(self.body_0.pos, vmul(self.body_0.vel, self.time_accel*dt_ms))

		self.body_1.push_pos_in_path(self.px_size)
		self.body_1.acc = vmul(n, G*self.body_0.m/d2)
		self.body_1.vel = vsum(self.body_1.vel, vmul(self.body_1.acc, self.time_accel*dt_ms))
		self.body_1.pos = vsum(self.body_1.pos, vmul(self.body_1.vel, self.time_accel*dt_ms))

		if follow:
			self.viewport_center = self.body_0.pos

		if autoscale:
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

		if autoscale and autoscale_grid:
			if self.grid_size/self.px_size < initial_grid_size_px/2:
				self.grid_size *= 2
			elif self.grid_size/self.px_size > initial_grid_size_px*2:
				self.grid_size /= 2

	def get_point_pos_in_viewport(self, p):
		return (int(  (p.x - self.viewport_center.x)/self.px_size + window_width_px/2 ),
		        int( -(p.y - self.viewport_center.y)/self.px_size + window_height_px/2))


model = Model()


pygame.init()
pygame.font.init()

fpsClock = pygame.time.Clock()
font = pygame.font.SysFont("LiberationMono-Regular", 30)

BLACK = pygame.Color(0,0,0, 255)
WHITE = pygame.Color(255, 255, 255, 255)
GREEN = pygame.Color(0, 255, 0, 255)
RED = pygame.Color(255, 0, 0, 255)
BLUE = pygame.Color(0, 0, 255, 255)

W = pygame.display.set_mode((window_width_px, window_height_px))
S = pygame.Surface((window_width_px, window_height_px), pygame.SRCALPHA)


overlay_render_period_ms = 500
overlay_render_ts = pygame.time.get_ticks()
overlay_text = ''

def render_ball(ball, color):
	pygame.draw.circle(S, RED, model.get_point_pos_in_viewport(ball.pos), ball.r, 0)
	if len(ball.path) > 1:
		pygame.draw.aalines(S, color, False, [model.get_point_pos_in_viewport(p) for p in ball.path], 1)


def render():
	global overlay_render_ts
	global overlay_text
	S.fill(BLACK)

	left_border_x = model.viewport_center.x - window_width_px/2*model.px_size
	line_x = math.ceil(left_border_x/model.grid_size)*model.grid_size
	while line_x < (left_border_x + window_width_px*model.px_size):
		line_x_px = (line_x - model.viewport_center.x)/model.px_size + window_width_px/2
		pygame.draw.line(S, WHITE, (line_x_px, 0), (line_x_px, window_height_px))
		line_x += model.grid_size


	top_border_y = model.viewport_center.y - window_height_px/2*model.px_size
	line_y = math.ceil(top_border_y/model.grid_size)*model.grid_size
	while line_y < (top_border_y + window_height_px*model.px_size):
		line_y_px = -(line_y - model.viewport_center.y)/model.px_size + window_height_px/2
		pygame.draw.line(S, WHITE, (0, line_y_px), (window_width_px, line_y_px))
		line_y += model.grid_size

	render_ball(model.body_0, RED)
	render_ball(model.body_1, BLUE)

	W.blit(S, (0, 0))

	if pygame.time.get_ticks() - overlay_render_ts > overlay_render_period_ms:
		overlay_render_ts = pygame.time.get_ticks()
		overlay_text = "%.1f fps" % fpsClock.get_fps()
	overlay_label = font.render(overlay_text, 1, GREEN)
	W.blit(overlay_label, (0, 0))

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



