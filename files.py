#!/usr/bin/python

import re
import sys
import io
import os
from pprint import pprint
import subprocess
import glob

def grep_includes (path, recursive=False):
	args = ["grep", path, "-e", "#include"]
	if recursive:
		args.append("-r")

	grepout_bin = b''

	try:
		grepout_bin = subprocess.check_output(args)
	except subprocess.CalledProcessError as err:
#		print(err.output)
#		print(err.returncode)
#		print(err.cmd)
		print()

	if not grepout_bin:
		return []

	grepout = grepout_bin.decode("utf-8").split(os.linesep)

	regex = re.compile('#include.*["<].*\.h[">]')

	trueincs = []
	incfiles = set([])

	for s in grepout:
		inc = regex.findall(s)
		if inc:
			trueincs.append(inc[0])
			f = inc[0].replace("#include", "").strip(" \"<>./")
			incfiles.add(f)
	#	else:
	#		print("\"%s\" not match" % s)

#	print()
#	print(path)
#	print()
#	pprint(incfiles)
#	print()

	return list(incfiles)


'''
	incfiles - list of filenames with path
'''
def basenames (incfiles):
	path = []

	for f in incfiles:
		path.append(f.split("/"))

	return list(map(lambda x: x[-1], path))



'''
	incfiles - list of stripped filenames without path
'''
def filter_project_deps (prj_path, subrelpath, incfiles):
	depincs = set([])

	for f in incfiles:
		find = subprocess.check_output(["find", prj_path, "-name", f])
		s = find.decode("utf-8").strip(os.linesep)
		if s and not subrelpath in s and os.path.isfile(s):
			depincs.add(s)

	return list(depincs)

def merge_lists (l1, l2):
	m = set(l1)
	for e in l2:
		m.add(e)
	return list(m)


def find_more_deps (prj_path, subrelpath, deps):
	count = 0
	while True:
		# pprint(sorted(deps))

		# grep includes from deps
		incs = set([])
		for d in deps:
			names = basenames(grep_includes(d))
			for n in names:
				incs.add(n)

		# filter only includes, that belongs to project, but not to subproject
		new_deps = filter_project_deps(prj_path, subrelpath, list(incs))

		deps = merge_lists(deps, new_deps)

		if len(deps) == count:
			break
		else:
			count = len(deps)

	return deps


def subproject_deps (prj_path, subrelpath):
	# grep includes from subproject
	includes = basenames(grep_includes(subrelpath, recursive=True))

	# filter only includes, that belongs to project, but not to subproject
	deps = filter_project_deps(prj_path, subrelpath, includes)

	# find deps for deps:
	deps = find_more_deps(prj_path, subrelpath, deps)

	return deps
'''
TODO: show full picture:

	walk trough subproject
	find all code files by filename extension
	save in tree structure
	grep for includes
	filter external, internal for project and internal for subproject
	save in tree structure
	repeat until not found new dependencies

	tree_element:
		type: subproject file | project file | external
		incs: list of included files


'''

PROJECT_PATH = sys.argv[1]
SUBPROJECT_PATH = sys.argv[2]

subprj_rel_path = SUBPROJECT_PATH.replace(PROJECT_PATH, "")

deps = subproject_deps(PROJECT_PATH, SUBPROJECT_PATH)

pprint(sorted(deps))





