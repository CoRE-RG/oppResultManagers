END TRANSACTION;

ATTACH DATABASE 'General-1.sqlite3' AS toMerge;

DELETE FROM toMerge.run WHERE toMerge.run.runid IN (SELECT runid FROM run);

INSERT INTO run (runid)
SELECT runid FROM toMerge.run WHERE NOT EXISTS (SELECT 1 FROM run WHERE runid=toMerge.run.runid);


INSERT INTO runattr (runid,nameid,value)
SELECT dest_run.id AS runid, dest_name.id AS nameid, toMerge.runattr.value FROM toMerge.runattr
JOIN toMerge.run AS src_run ON toMerge.runattr.runid = src_run.id
JOIN toMerge.name AS src_name ON toMerge.runattr.nameid = src_name.id
JOIN run AS dest_run ON src_run.runid = dest_run.runid
JOIN name AS dest_name ON src_name.name = dest_name.name;

INSERT INTO module (name) SELECT name FROM toMerge.module WHERE NOT EXISTS (SELECT 1 FROM module WHERE name=toMerge.module.name);

INSERT INTO name (name) SELECT name FROM toMerge.name WHERE NOT EXISTS (SELECT 1 FROM name WHERE name=toMerge.name.name);

INSERT INTO scalar (runid,moduleid,nameid,value) 
SELECT dest_run.id AS runid, dest_module.id AS moduleid,dest_name.id AS nameid,value FROM toMerge.scalar 
JOIN toMerge.run ON toMerge.scalar.runid = toMerge.run.id
JOIN toMerge.module ON toMerge.scalar.moduleid = toMerge.module.id
JOIN toMerge.name ON toMerge.scalar.nameid = toMerge.name.id
JOIN run AS dest_run ON toMerge.run.runid = dest_run.runid
JOIN module AS dest_module ON toMerge.module.name = dest_module.name
JOIN name AS dest_name ON toMerge.name.name = dest_name.name;

INSERT INTO scalarattr (scalarid,nameid,value)
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
JOIN scalar AS dest_scalar ON dest_run.id = dest_scalar.runid AND dest_module.id = dest_scalar.moduleid AND dest_scalarname.id = dest_scalar.nameid;

INSERT INTO vector (runid,moduleid,nameid) 
SELECT dest_run.id AS runid, dest_module.id AS moduleid,dest_name.id AS nameid FROM toMerge.vector 
JOIN toMerge.run ON toMerge.vector.runid = toMerge.run.id
JOIN toMerge.module ON toMerge.vector.moduleid = toMerge.module.id
JOIN toMerge.name ON toMerge.vector.nameid = toMerge.name.id
JOIN run AS dest_run ON toMerge.run.runid = dest_run.runid
JOIN module AS dest_module ON toMerge.module.name = dest_module.name
JOIN name AS dest_name ON toMerge.name.name = dest_name.name;

INSERT INTO vectorattr (vectorid,nameid,value)
SELECT dest_vector.id AS vectorid, dest_name.id AS nameid, value FROM toMerge.vectorattr
JOIN toMerge.vector AS src_vector ON toMerge.vectorattr.vectorid = src_vector.id
JOIN toMerge.run AS src_run ON src_vector.runid = src_run.id
JOIN toMerge.module AS src_module ON src_vector.moduleid = src_module.id
JOIN toMerge.name AS src_name ON toMerge.vectorattr.nameid = src_name.id
JOIN toMerge.name AS src_vectorname ON src_vector.nameid = src_vectorname.id
JOIN run AS dest_run ON src_run.runid = dest_run.runid
JOIN module AS dest_module ON src_module.name = dest_module.name
JOIN name AS dest_name ON src_name.name = dest_name.name
JOIN name AS dest_vectorname ON src_vectorname.name = dest_vectorname.name
JOIN vector AS dest_vector ON dest_run.id = dest_vector.runid AND dest_module.id = dest_vector.moduleid AND dest_vectorname.id = dest_vector.nameid;

INSERT INTO vectordata (vectorid,time,value)
SELECT dest_vector.id AS vectorid, time, value FROM toMerge.vectordata
JOIN toMerge.vector AS src_vector ON toMerge.vectordata.vectorid = src_vector.id
JOIN toMerge.run AS src_run ON src_vector.runid = src_run.id
JOIN toMerge.module AS src_module ON src_vector.moduleid = src_module.id
JOIN toMerge.name AS src_vectorname ON src_vector.nameid = src_vectorname.id
JOIN run AS dest_run ON src_run.runid = dest_run.runid
JOIN module AS dest_module ON src_module.name = dest_module.name
JOIN name AS dest_vectorname ON src_vectorname.name = dest_vectorname.name
JOIN vector AS dest_vector ON dest_run.id = dest_vector.runid AND dest_module.id = dest_vector.moduleid AND dest_vectorname.id = dest_vector.nameid;

DETACH DATABASE 'toMerge';