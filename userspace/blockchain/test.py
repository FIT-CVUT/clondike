import pickledb
db = pickledb.load('example.db', False) 
db.set('key', 'value') 
db.get('key') 
db.dump()
