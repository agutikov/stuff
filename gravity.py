#!/usr/bin/env python


from pprint import pprint
import sys
import random
import pygame
from collections import namedtuple
import math

#TODO: path, manual scale, 3d, coordinate mesh legend, focus

class Point:
	x=0
	y=0
	def __init__ (self, _x, _y):
		self.x = _x
		self.y = _y

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




class Ball:
	def __init__ (self, _m, _r, _p, _v, _a):
		self.m = _m
		self.r = _r
		self.pos=_p
		self.vel=_v
		self.acc=_a


R = 149597870700
#R = 14959787070
V0 = 0
#V0 = 10000
#V1 = 29783
V1 = 35000
M0 = 2*10**30
#M0 = 6*10**24
M1 = 6*10**24
G = 6.67*10**-11

window_width_px = 400
window_height_px = 300
start_distance_px = 100
start_grid_size_px = 80

TIME_ACCEL = 3600

max_fps = 100


class Model:

	def __init__(self):
		self.time_accel=TIME_ACCEL

		self.model_scale=R/start_distance_px

		self.model_viewport_width = window_width_px*self.model_scale
		self.model_viewport_height = window_height_px*self.model_scale
		self.model_viewport_start = Point(0, 0)

		self.grid_size = start_grid_size_px*self.model_scale

		self.star = Ball(M0, 20, Point(0,0), Point(-V0,-V0), Point(0,0))
		self.planet = Ball(M1, 10, Point(0, R), Point(V1,0), Point(0,0))


	def run(self, dt_ms):
		d2 = distance2(self.star.pos, self.planet.pos)
		s = vdif(self.star.pos, self.planet.pos)
		n = norm(s)

		self.star.acc = vmul(invert(n), G*self.planet.m/d2)
		self.star.vel = vsum(self.star.vel, vmul(self.star.acc, self.time_accel*dt_ms))
		self.star.pos = vsum(self.star.pos, vmul(self.star.vel, self.time_accel*dt_ms))

		self.planet.acc = vmul(n, G*self.star.m/d2)
		self.planet.vel = vsum(self.planet.vel , vmul(self.planet.acc, self.time_accel*dt_ms))
		self.planet.pos = vsum(self.planet.pos , vmul(self.planet.vel, self.time_accel*dt_ms))


		self.model_viewport_start = self.star.pos

		self.model_viewport_width = max(self.model_viewport_width,
						3*max(abs(self.planet.pos.x - self.model_viewport_start.x), abs(self.star.pos.x - self.model_viewport_start.x)))
		self.model_viewport_height = max(self.model_viewport_height,
						3*max(abs(self.planet.pos.y - self.model_viewport_start.y), abs(self.star.pos.y - self.model_viewport_start.y)))

		self.model_scale = max(self.model_viewport_width/window_width_px, self.model_viewport_height/window_height_px)

		self.model_viewport_width = 6*min(self.model_viewport_width/6,
						max(abs(self.planet.pos.x - self.model_viewport_start.x), abs(self.star.pos.x - self.model_viewport_start.x)))
		self.model_viewport_height = 6*min(self.model_viewport_height/6,
						max(abs(self.planet.pos.y - self.model_viewport_start.y), abs(self.star.pos.y - self.model_viewport_start.y)))

		self.model_scale = max(self.model_viewport_width/window_width_px, self.model_viewport_height/window_height_px)

		self.model_viewport_width = window_width_px*self.model_scale
		self.model_viewport_height = window_height_px*self.model_scale


		if self.grid_size/self.model_scale < start_grid_size_px/2:
			self.grid_size *= 2
		elif self.grid_size/self.model_scale > start_grid_size_px*2:
			self.grid_size /= 2


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


def render():
	S.fill(BLACK)


	left_border_x = model.model_viewport_start.x - window_width_px/2*model.model_scale
	line_x = math.ceil(left_border_x/model.grid_size)*model.grid_size
	while line_x < (left_border_x + window_width_px*model.model_scale):
		line_x_px = (line_x - model.model_viewport_start.x)/model.model_scale + window_width_px/2
		pygame.draw.line(S, WHITE, (line_x_px, 0), (line_x_px, window_height_px))
		line_x += model.grid_size


	top_border_y = model.model_viewport_start.y - window_height_px/2*model.model_scale
	line_y = math.ceil(top_border_y/model.grid_size)*model.grid_size
	while line_y < (top_border_y + window_height_px*model.model_scale):
		line_y_px = -(line_y - model.model_viewport_start.y)/model.model_scale + window_height_px/2
		pygame.draw.line(S, WHITE, (0, line_y_px), (window_width_px, line_y_px))
		line_y += model.grid_size


	pygame.draw.circle(S, RED,
		(int( (model.star.pos.x - model.model_viewport_start.x)/model.model_scale + window_width_px/2 ),
		 int( -(model.star.pos.y - model.model_viewport_start.y)/model.model_scale + window_height_px/2 )),
		model.star.r, 0)

	pygame.draw.circle(S, BLUE,
		(int( (model.planet.pos.x - model.model_viewport_start.x)/model.model_scale + window_width_px/2 ),
		 int( -(model.planet.pos.y - model.model_viewport_start.y)/model.model_scale + window_height_px/2 )),
		model.planet.r, 0)

	W.blit(S, (0, 0))

	str = "%.1f fps" % fpsClock.get_fps()
	label = font.render(str, 1, GREEN)
	W.blit(label, (0, 0))

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



