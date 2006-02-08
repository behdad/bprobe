#!/usr/bin/python
"""
bprobe command-line tool.

Copyright (C) 2006  Behdad Esfahbod <behdad@gnome.org>

bprobe is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
See the GNU Library General Public License for more details.
"""

import sys, getopt, os, os.path, re


def __log (message):
	"""Log message if verbose."""
	if verbose:
		print "%s: %s" % (self, message)

def __log_v (message):
	"""Log message if verbose, without prompt."""
	if verbose:
		print message

def __fail (message):
	"""Fail with a message."""
	print >>sys.stderr, "%s: %s" % (self, message)
	sys.exit (exitcode)

def __get_vars (items):
	"""Yield a tuple containing second and third of every three items."""
	list = []
	for item in items:
		list.append (item)
		if len (list) == 3:
			yield (list[1], list[2])
			list = []

def __extract_metadata (dict, text):
	"""Extract metadata from preprocessed probe source."""

	items = re.split (r'\{\{([a-zA-Z0-9_]+)\{X\{(.*?)\}X\}\1\}\}', text)

	for var, val in __get_vars (items):

		val = os.path.expandvars (val)

		vals = dict.get (var, [])
		vals.append (val)
		dict[var] = vals
	
	return dict

cc = os.getenv ('CC') or 'gcc'
cpp = os.getenv ('CPP') or 'gcc -E'
pkg_config = os.getenv ('PKG_CONFIG') or 'pkg-config'

def get_metadata (probe, metadata = None, standalone = False):
	"""Return a dictionary containing the metadata of a probe source."""

	if not metadata:
		metadata = {}
	
	# Fail if bprobe headers are not found.
	#
	bprobe_h = os.path.join (include_dir, 'bprobe.h')
	bprobe_private_h = os.path.join (include_dir, 'bprobe-private.h')
	if not os.path.isfile (bprobe_h):
		fail ('bprobe.h not found')
	if not os.path.isfile (bprobe_private_h):
		fail ('bprobe-private.h not found')

	# Construct the preprocessor command.
	#
	cmd = cpp.split ()
	if not standalone:
		cmd.extend (['-DBPROBE'])
	cmd.extend (['-DBPROBE_GATHER_METADATA', '-nostdinc', '-I'+include_dir])
	cmd.extend (['-x', 'c', probe])

	# Run the preprocessor.
	#
	__log ("%s: preprocessing" % probe)
	__log_v (' '.join (cmd))
	(inn, out, err) = os.popen3 (cmd)
	text = out.read ()
	if not text:
		__fail ('%s: preprocessing failed using %s' % (probe, cpp))
	
	# Extract metadata from the preprocessor output.
	#
	return __extract_metadata (metadata, text)

def __md_item (metadata, key):
	"""Return a single string for a key in metadata."""
	return ' '.join (metadata.get (key, []))

def __pkg_config (list, what = '--cflags --libs'):
	"""Return a pkg-config command to lookup what in list of packages."""
	if list:
		return '`%s %s %s`' % (pkg_config, what, list)
	else:
		return ''

def compile (infile, outfile, standalone = False):
	"""Compile probe."""

	# Make sure outfile is not in the way.
	#
	if os.path.isfile (outfile):
		try:
			os.unlink (outfile)
		except OSError, e:
			__fail ("%s: %s" % (outfile, e.strerror))

	# Initialize fallback metadata.
	#
	metadata = {'_COMPILER': [cc], '_LINKER': [cc], '_LANGUAGE': ['c']}

	# Get metadata out of the probe source.
	#
	metadata = get_metadata (infile, metadata, standalone)

	# Construct the compile command.
	#
	compiler = metadata['_COMPILER'][-1]
	language = metadata['_LANGUAGE'][-1].lower ()

	cmd  = [compiler]

	cmd.append (__md_item (metadata, '_COMPILER_FLAGS'))
	cmd.append (__pkg_config (__md_item (metadata, '_PKG_CONFIG')))
	cmd.append (__pkg_config (__md_item (metadata, '_PKG_CONFIG_CFLAGS'), '--cflags'))
	cmd.append (__pkg_config (__md_item (metadata, '_PKG_CONFIG_LIBS'), '--libs'))

	if not standalone:
		cmd.extend (['-DBPROBE', '-shared'])
	cmd.extend (['-ldl', '-I'+include_dir, '-o', outfile, '-x', language, infile])

	cmd = [item for item in cmd if len(item)]
	cmd = ' '.join (cmd)

	# Run the compiler.
	#
	__log ("%s: compiling" % infile)
	__log_v (cmd)
	(inn, out, err) = os.popen3 (cmd, 'r')
	errs = err.read ()
	if errs:
		__fail (errs)
	
	if not os.path.isfile (outfile):
		__fail ("%s: compiler did not produce expected output file" % outfile)
	
	# Chmod it back from executable, if not standalone.
	#
	if not standalone:
		os.chmod (outfile, os.stat (infile).st_mode)



suffixes = ['.probe', '.bprobe']
sosuffix = '.so'
mainsuffix = '.main'

def get_compiled (probe, forced = False, standalone = False):
	"""Returns the file-name of a compiled probe, given an input file name."""

	if standalone:
		suffix = mainsuffix
	else:
		suffix = sosuffix

	# If a file exists, does not end with any bprobe suffix, and ends
	# with "suffix" or contains "suffix." in it, it's a compiled probe
	# (or another shared library, whatever), just return it...
	#
	if os.path.isfile (probe) and \
	   (probe.endswith (suffix) or probe.find (suffix+'.') != -1) and \
	   not [s for s in suffixes if probe.endswith (s)]:

		return probe

	# Otherwise if it does not exist but adding a suffix turns it into
	# an existing file, add the suffix.
	#
	if not os.path.isfile (probe):
		for suffix in suffixes:
			if os.path.isfile (probe+suffix):
				probe += suffix
				break

	# Finally, if it still does not exist, err
	#
	if not os.path.isfile (probe):
		__fail ("%s: probe not found" % probe)
	
	outfile = probe + suffix

	bprobe_py = __file__
	bprobe_h = os.path.join (include_dir, 'bprobe.h')
	bprobe_private_h = os.path.join (include_dir, 'bprobe-private.h')

	# If not forced, and the outfile file for the probe exists, and
	# the outfile file is not older than the probe, either the bprobe
	# headers are not found or outfile is not older than any of the
	# header files and the compiler, the outfile file is up to date,
	# do not recompile.
	#
	if not forced and \
	   os.path.isfile (outfile) and \
	   os.path.getmtime (outfile) >= os.path.getmtime (probe) and \
	   (not os.path.isfile (bprobe_h) or not os.path.isfile (bprobe_private_h) or \
	    (os.path.getmtime (outfile) >= os.path.getmtime (bprobe_py) and \
	     os.path.getmtime (outfile) >= os.path.getmtime (bprobe_h) and \
	     os.path.getmtime (outfile) >= os.path.getmtime (bprobe_private_h))):

		__log ("%s: up to date, not recompiling" % outfile)
		return outfile
	
	# If still here, compile it.
	#
	compile (probe, outfile, standalone)

	return outfile



def run (probes, cmd_args, forced = False, standalone = False, shared = True):
	"""Compile probes if needed or forced, and run the command with the probes set, if any.  If no shared compilation is set, no command should be passed."""

	if standalone:
		# Compile standalones first.
		#
		[get_compiled (probe, forced, True) for probe in probes]

	if shared:

		# Get .so files for probes.
		#
		probes = [get_compiled (probe, forced, False) for probe in probes]

		# If command set, run it.
		if cmd_args:

			# Append $LD_PRELOAD.
			#
			ld_preload = os.getenv ('LD_PRELOAD')
			probe_preload = ' '.join (probes)
			if ld_preload:
				probe_preload += ' ' + ld_preload
			if probe_preload:
				os.putenv ('LD_PRELOAD', probe_preload)

			cmd = cmd_args[0]
			try:
				os.execvp (cmd, cmd_args)
			except OSError, e:
				__fail ("%s: %s" % (cmd, e.strerror))

	elif cmd_args:
		__fail ('cannot run command with standalone probes')


exitcode = 126
self = 'bprobe'
include_dir = None
verbose = False

def main (args):

	"""Usage: bprobe [OPTION]... [PROBE]... -- [CMD [ARGS]...]
Run command with one or more probes set.

  -h, --help        Show this usage info
  -c, --compile     Compile probes (default action)
  -m, --main        Make standalone applications from probes.  This changes
                    the default action. Use --compile to compile probes still.
  -f, --force       Force recompilation even if outputs are up to date
  -v, --verbose     Be a bit more talkative

Report bugs to <behdad@gnome.org>"""

	# Initialized include dir, etc.
	#
	global dir, include_dir
	dirname = os.path.dirname (args[0])
	if not include_dir:
		include_dir = dirname
	del args[0]

	forced = False
	standalone = False
	shared = False

	# Parse options.
	#
	if not args:
		print >>sys.stderr, main.__doc__
		sys.exit (exitcode)
	try:
		opts, args = getopt.getopt (args, 'hcmvVf', ['help', 'compile', 'main', 'verbose', 'force'])
	except getopt.GetoptError, e:
		__fail (e)
	for opt, value in opts:
		if opt in ['-h', '--help']:
			print main.__doc__
			sys.exit ()
		elif opt in ['-c', '--compile']:
			shared = True
		elif opt in ['-m', '--main']:
			standalone = True
		elif opt in ['-v', '-V', '--verbose']:
			global verbose
			verbose = True
		elif opt in ['-f', '--force']:
			forced = True

	# Default action.
	#
	if not standalone:
		shared = True

	# Split remaining arguments into probes and cmd/args.
	#
	if '--' in args:
		i = args.index ('--')
		probes = args[:i]
		cmd_args = args[i+1:]
	else:
		probes = args
		cmd_args = None
	
	# Run!
	#
	run (probes, cmd_args, forced=forced, standalone=standalone, shared=shared)


if __name__ == '__main__':
	main(sys.argv)
