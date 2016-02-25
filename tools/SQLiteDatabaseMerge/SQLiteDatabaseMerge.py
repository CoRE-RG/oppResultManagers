#!/usr/bin/env python
# Copyright (c) 2016, CoRE Research Group, Hamburg University of Applied Sciences
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
#
#
# Script to merge multiple sqlite3 databases that were written using a cSQLiteOutputManager
#
# This is useful when running multiple simulations in parallel, as these cannot share the
# same database file due to concurrency and locking
#
# Author: Till Steinbach
#
#

import sqlite3
import argparse
import glob
import sys
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Merges two or more source databases into destination database. Databases must be in cSQLiteResultManager format.')
    parser.add_argument('databasefiles', nargs='*', metavar='databasefile', help='sqlite3 files that contain the databases to merge. Must be in cSQLiteResultManager format. Default is *.sqlite3')
    parser.add_argument('-o', '--output', nargs=1, metavar='output', help='Filename of output database. Default is merge.sqlite3')
    args = parser.parse_args()

    if not args.databasefiles:
        args.databasefiles = glob.glob('*.sqlite3')
    if not args.output:
        args.output = 'merge.sqlite3'
    if len(args.databasefiles) == 0:
	print 'nothing to do here'
    elif len(args.databasefiles) == 1:
		sys.stdout.write('moving database '+args.databasefiles[0]+ ' over to '+args.output+'\n')
		os.rename(args.databasefiles[0], args.output)
    else:
		print 'moving database '+args.databasefiles[0]+ ' over to '+args.output
		os.rename(args.databasefiles[0], args.output)
		conn = sqlite3.connect(args.output)
		c = conn.cursor()

		# Performance and integrity settings
		c.execute("PRAGMA synchronous = OFF;")
		c.execute("PRAGMA journal_mode = MEMORY;")
		c.execute("PRAGMA cache_size = -16768;")
		# TODO turn this off when everything works! Performance!
		c.execute("PRAGMA foreign_keys = ON;")
	
		for file in args.databasefiles[1:]:
			sys.stdout.write('Attaching database '+file+'...')
			# Attach database to merge into source database
			c.execute("ATTACH DATABASE '"+file+"' AS toMerge;")
			sys.stdout.write('done\n')

			sys.stdout.write('\t deleting duplicate runs (runs that exist in multiple databases) based on the unique runid...')
			# DELETE DUPLICATES, due to cascading this will delete a lot of data and thus saves time if there are really duplicates
			c.execute("DELETE FROM toMerge.run WHERE toMerge.run.runid IN (SELECT runid FROM run);")
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting remaining runs...')
			# Insert all missing runs
			c.execute("INSERT INTO run (runid) SELECT runid FROM toMerge.run WHERE NOT EXISTS (SELECT 1 FROM run WHERE runid=toMerge.run.runid);")
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting runattr...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO runattr (runid,nameid,value)
			SELECT dest_run.id AS runid, dest_name.id AS nameid, toMerge.runattr.value FROM toMerge.runattr
			JOIN toMerge.run AS src_run ON toMerge.runattr.runid = src_run.id
			JOIN toMerge.name AS src_name ON toMerge.runattr.nameid = src_name.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN name AS dest_name ON src_name.name = dest_name.name;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting missing modules...')
			# Insert all missing modules
			c.execute("INSERT INTO module (name) SELECT name FROM toMerge.module WHERE NOT EXISTS (SELECT 1 FROM module WHERE name=toMerge.module.name);")
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting missing names...')
			# Insert all missing names
			c.execute("INSERT INTO name (name) SELECT name FROM toMerge.name WHERE NOT EXISTS (SELECT 1 FROM name WHERE name=toMerge.name.name);")
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting scalars...')
			# Insert all scalars while looking uo the correct names and modules
			c.execute('''INSERT INTO scalar (runid,moduleid,nameid,value) 
			SELECT dest_run.id AS runid, dest_module.id AS moduleid,dest_name.id AS nameid,value FROM toMerge.scalar 
			JOIN toMerge.run ON toMerge.scalar.runid = toMerge.run.id
			JOIN toMerge.module ON toMerge.scalar.moduleid = toMerge.module.id
			JOIN toMerge.name ON toMerge.scalar.nameid = toMerge.name.id
			JOIN run AS dest_run ON toMerge.run.runid = dest_run.runid
			JOIN module AS dest_module ON toMerge.module.name = dest_module.name
			JOIN name AS dest_name ON toMerge.name.name = dest_name.name;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting scalarattr...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO scalarattr (scalarid,nameid,value)
			SELECT dest_scalar.id AS scalarid, dest_name.id AS nameid, toMerge.scalarattr.value FROM toMerge.scalarattr
			JOIN toMerge.scalar AS src_scalar ON toMerge.scalarattr.scalarid = src_scalar.id
			JOIN toMerge.run AS src_run ON src_scalar.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_scalar.moduleid = src_scalar.id
			JOIN toMerge.name AS src_name ON toMerge.scalarattr.nameid = src_name.id
			JOIN toMerge.name AS src_scalarname ON src_scalar.nameid = src_scalarname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_name ON src_name.name = dest_name.name
			JOIN name AS dest_scalarname ON src_scalarname.name = dest_scalarname.name
			JOIN vector AS dest_scalar ON dest_run.id = dest_scalar.runid AND dest_module.id = dest_scalar.moduleid AND dest_scalarname.id = dest_scalar.nameid;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting statistic...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO statistic (runid, moduleid, nameid)
			SELECT dest_run.id AS runid, dest_module.id AS moduleid,dest_name.id AS nameid FROM toMerge.statistic 
			JOIN toMerge.run AS src_run ON toMerge.statistic.runid = src_run.id
			JOIN toMerge.module AS src_module ON toMerge.statistic.moduleid = src_module.id
			JOIN toMerge.name AS src_name ON toMerge.statistic.nameid = src_name.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_name ON src_name.name = dest_name.name;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting statisticattr...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO statisticattr (statisticid,nameid,value)
			SELECT dest_statistic.id AS statisticid, dest_name.id AS nameid, toMerge.statisticattr.value FROM toMerge.statisticattr
			JOIN toMerge.statistic AS src_statistic ON toMerge.statisticattr.statisticid = src_statistic.id
			JOIN toMerge.run AS src_run ON src_statistic.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_statistic.moduleid = src_module.id
			JOIN toMerge.name AS src_name ON toMerge.statisticattr.nameid = src_name.id
			JOIN toMerge.name AS src_statisticname ON src_statistic.nameid = src_statisticname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_name ON src_name.name = dest_name.name
			JOIN name AS dest_statisticname ON src_statisticname.name = dest_statisticname.name
			JOIN statistic AS dest_statistic ON dest_run.id = dest_statistic.runid AND dest_module.id = dest_statistic.moduleid AND dest_statisticname.id = dest_statistic.nameid;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting field...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO field (statisticid,nameid,value)
			SELECT dest_statistic.id AS statisticid, dest_name.id AS nameid, toMerge.field.value FROM toMerge.field
			JOIN toMerge.statistic AS src_statistic ON toMerge.field.statisticid = src_statistic.id
			JOIN toMerge.run AS src_run ON src_statistic.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_statistic.moduleid = src_module.id
			JOIN toMerge.name AS src_name ON toMerge.field.nameid = src_name.id
			JOIN toMerge.name AS src_statisticname ON src_statistic.nameid = src_statisticname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_name ON src_name.name = dest_name.name
			JOIN name AS dest_statisticname ON src_statisticname.name = dest_statisticname.name
			JOIN statistic AS dest_statistic ON dest_run.id = dest_statistic.runid AND dest_module.id = dest_statistic.moduleid AND dest_statisticname.id = dest_statistic.nameid;''')
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting bin...')
			# Insert all scalarattrs while looking uo the correct names and modules
			c.execute('''INSERT INTO bin (statisticid,binlowerbound,value)
			SELECT dest_statistic.id AS statisticid, toMerge.bin.binlowerbound AS binlowerbound, toMerge.bin.value FROM toMerge.bin
			JOIN toMerge.statistic AS src_statistic ON toMerge.bin.statisticid = src_statistic.id
			JOIN toMerge.run AS src_run ON src_statistic.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_statistic.moduleid = src_module.id
			JOIN toMerge.name AS src_statisticname ON src_statistic.nameid = src_statisticname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_statisticname ON src_statisticname.name = dest_statisticname.name
			JOIN statistic AS dest_statistic ON dest_run.id = dest_statistic.runid AND dest_module.id = dest_statistic.moduleid AND dest_statisticname.id = dest_statistic.nameid;''')
			sys.stdout.write('done\n')

			sys.stdout.write('\t inserting vectors...')
			# Insert all vectors while looking uo the correct names and modules
			c.execute('''INSERT INTO vector (runid,moduleid,nameid) 
			SELECT dest_run.id AS runid, dest_module.id AS moduleid,dest_name.id AS nameid FROM toMerge.vector 
			JOIN toMerge.run ON toMerge.vector.runid = toMerge.run.id
			JOIN toMerge.module ON toMerge.vector.moduleid = toMerge.module.id
			JOIN toMerge.name ON toMerge.vector.nameid = toMerge.name.id
			JOIN run AS dest_run ON toMerge.run.runid = dest_run.runid
			JOIN module AS dest_module ON toMerge.module.name = dest_module.name
			JOIN name AS dest_name ON toMerge.name.name = dest_name.name;''')
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting vectorattr...')
			# Insert all vectorattr while looking uo the correct vectors
			c.execute('''INSERT INTO scalarattr (scalarid,nameid,value)
			SELECT dest_scalar.id AS scalarid, dest_name.id AS nameid, toMerge.scalarattr.value FROM toMerge.scalarattr
			JOIN toMerge.scalar AS src_scalar ON toMerge.scalarattr.scalarid = src_scalar.id
			JOIN toMerge.run AS src_run ON src_scalar.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_scalar.moduleid = src_module.id
			JOIN toMerge.name AS src_name ON toMerge.scalarattr.nameid = src_name.id
			JOIN toMerge.name AS src_scalarname ON src_scalar.nameid = src_scalarname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_name ON src_name.name = dest_name.name
			JOIN name AS dest_scalarname ON src_scalarname.name = dest_scalarname.name
			JOIN scalar AS dest_scalar ON dest_run.id = dest_scalar.runid AND dest_module.id = dest_scalar.moduleid AND dest_scalarname.id = dest_scalar.nameid;''')
			sys.stdout.write('done\n')
			
			sys.stdout.write('\t inserting vectordata...')
			# Insert all vectordata while looking uo the correct vectors
			c.execute('''INSERT INTO vectordata (vectorid,time,value)
			SELECT dest_vector.id AS vectorid, time, value FROM toMerge.vectordata
			JOIN toMerge.vector AS src_vector ON toMerge.vectordata.vectorid = src_vector.id
			JOIN toMerge.run AS src_run ON src_vector.runid = src_run.id
			JOIN toMerge.module AS src_module ON src_vector.moduleid = src_module.id
			JOIN toMerge.name AS src_vectorname ON src_vector.nameid = src_vectorname.id
			JOIN run AS dest_run ON src_run.runid = dest_run.runid
			JOIN module AS dest_module ON src_module.name = dest_module.name
			JOIN name AS dest_vectorname ON src_vectorname.name = dest_vectorname.name
			JOIN vector AS dest_vector ON dest_run.id = dest_vector.runid AND dest_module.id = dest_vector.moduleid AND dest_vectorname.id = dest_vector.nameid;''')
			sys.stdout.write('done\n')
			
			sys.stdout.write('Detaching database '+file+'...')
			c.execute("DETACH DATABASE 'toMerge';")
			sys.stdout.write('done\n')
			
			sys.stdout.write('Deleting database '+file+'...')
			os.remove(file)
			sys.stdout.write('done\n')
		conn.close()
    sys.stdout.write('Finished merging databases\n')
    sys.exit(0)
