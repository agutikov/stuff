#!/usr/bin/python


from pprint import pprint
import sys
import random
import pygame
from collections import namedtuple
import math



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
V = 29783
M0 = 2*10**30
M1 = 6*10**24
G = 6.67*10**-11

window_width = 400
window_height = 300

class Model:

	def __init__(self):
		self.time_accel=360000/1000

		self.model_scale=R/100
		self.model_viewport_width = window_width*self.model_scale
		self.model_viewport_height = window_height*self.model_scale
		self.model_viewport_start = Point(0, 0)

		self.star = Ball(M0, 20, Point(0,0), Point(0,0), Point(0,0))
		self.planet = Ball(M1, 10, Point(0, R), Point(V,0), Point(0,0))


	def run(self, dt_ms):
		d2 = distance2(self.star.pos, self.planet.pos)
		s = vdif(self.star.pos, self.planet.pos)
		n = norm(s)

		star_new_pos = vsum(self.star.pos, vmul(self.star.vel, self.time_accel*dt_ms))
		star_new_vel = vsum(self.star.vel, vmul(self.star.acc, self.time_accel*dt_ms))
		star_new_acc = vmul(invert(n), G*self.planet.m/d2)

		planet_new_pos = vsum(self.planet.pos , vmul(self.planet.vel, self.time_accel*dt_ms))
		planet_new_vel = vsum(self.planet.vel , vmul(self.planet.acc, self.time_accel*dt_ms))
		planet_new_acc = vmul(n, G*self.star.m/d2)

		self.star.pos = star_new_pos
		self.star.vel = star_new_vel
		self.star.acc = star_new_acc
		self.planet.pos = planet_new_pos
		self.planet.vel = planet_new_vel
		self.planet.acc = planet_new_acc

		model_viewport_width = max(self.model_viewport_width, 3*max(abs(self.planet.pos.x), abs(self.star.pos.x)))
		model_viewport_height = max(self.model_viewport_height, 3*max(abs(self.planet.pos.y), abs(self.star.pos.y)))

		self.model_scale = max(self.model_viewport_width/window_width, self.model_viewport_height/window_height)


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

W = pygame.display.set_mode((window_width, window_height))
S = pygame.Surface((window_width, window_height), pygame.SRCALPHA)


def render():

	S.fill(BLACK)

	pygame.draw.circle(S, WHITE,
		(int( (model.star.pos.x - model.model_viewport_start.x)/model.model_scale + window_width/2 ),
		 int( -(model.star.pos.y - model.model_viewport_start.y)/model.model_scale + window_height/2 )),
		model.star.r, 0)

	pygame.draw.circle(S, BLUE,
		(int( (model.planet.pos.x - model.model_viewport_start.x)/model.model_scale + window_width/2 ),
		 int( -(model.planet.pos.y - model.model_viewport_start.y)/model.model_scale + window_height/2 )),
		model.planet.r, 0)

	W.blit(S, (0, 0))

	str = "%.1f fps" % fpsClock.get_fps()
	label = font.render(str, 1, GREEN)
	W.blit(label, (0, 0))

	pygame.display.update()


prev_time = pygame.time.get_ticks()

max_fps = 100

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



