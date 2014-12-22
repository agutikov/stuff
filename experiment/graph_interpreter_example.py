#!/usr/bin/python



"""

engine

environment
	node index


embedded io library
embedded processing library



signal - callback call
signal output - call
signal input - callback


bus - data transfer into callback call


--------------------------------------------------------------------------------

engine == graph interpreter
	graph representation
	interpreter - serialize steps on graph
	executor - execute queue of events
	io generate events and put them into queue

node hierarchy (python classes)
	node should register itself in the environment with it's interface and bus data types

basic environment 
	node index: name <-> node (interface, io bindings)
	data types index: name <-> node interface
	
io bindings	
	basic IO events handling + generating embedded node's signals

runtime environment




"""

import sys
import io
import os
from pprint import pprint
import select
import queue


class basic_environment:
	
	# name -> node map
	nodes = {}
	
	register_node (node):
		nodes[node.name] = node
	



class node:
	
	name = ""
	
	interface = {}
	
	
	
class graph:
	
	
	
	

class io_bindings:
	
	# move to stdin and stdout nodes
	_in = Queue()
	_out = Queue()
	
	_rlist = [sys.stdin]
	_wlist = [sys.stdout]
	_xlist = []
	_select_timeout = 0.0
	
	
	
	def io_loop ():
		readable, writable, exceptional =  select.select(_rlist, _wlist, _xlist, _select_timeout)
		
		if readable:
			for line in sys.stdin:
				_in.append(line)
				
		if writable:
			for line in _out:
				sys.stdout.write(line)
				
	
	




































