z = 0.5

luf = [-0.5,  0.05, z]
ldf = [-0.5, 0, z]
ruf = [0, 0.1, z]
rdf = [0, 0, z]

lub = luf[:]
lub[2] = -lub[2]

ldb = ldf[:]
ldb[2] = -ldb[2]

rub = ruf[:]
rub[2] = -rub[2]

rdb = rdf[:]
rdb[2] = -rdb[2]

print("// draw front face")
print("glNormal3f(0, 0, 1);")
print("glVertex3f" + str(tuple(ldf))+ ";")
print("glVertex3f" + str(tuple(rdf))+ ";")
print("glVertex3f" + str(tuple(ruf))+ ";")
print("glVertex3f" + str(tuple(luf))+ ";")
print("// draw back face")
print("glNormal3f(0, 0, -1);")
print("glVertex3f" + str(tuple(rdb))+ ";")
print("glVertex3f" + str(tuple(ldb))+ ";")
print("glVertex3f" + str(tuple(lub))+ ";")
print("glVertex3f" + str(tuple(rub))+ ";")
print("// draw left face")
print("glNormal3f(-1, 0, 0);")
print("glVertex3f" + str(tuple(ldb))+ ";")
print("glVertex3f" + str(tuple(ldf))+ ";")
print("glVertex3f" + str(tuple(luf))+ ";")
print("glVertex3f" + str(tuple(lub))+ ";")
print("// draw right face")
print("glNormal3f(1, 0, 0);")
print("glVertex3f" + str(tuple(rdf))+ ";")
print("glVertex3f" + str(tuple(rdb))+ ";")
print("glVertex3f" + str(tuple(rub))+ ";")
print("glVertex3f" + str(tuple(ruf))+ ";")
print("// draw top")
print("glNormal3f(0, 1, 0);")
print("glVertex3f" + str(tuple(luf))+ ";")
print("glVertex3f" + str(tuple(ruf))+ ";")
print("glVertex3f" + str(tuple(rub))+ ";")
print("glVertex3f" + str(tuple(lub))+ ";")
print("// draw bottom")
print("glNormal3f(0, -1, 0);")
print("glVertex3f" + str(tuple(ldb))+ ";")
print("glVertex3f" + str(tuple(rdb))+ ";")
print("glVertex3f" + str(tuple(rdf))+ ";")
print("glVertex3f" + str(tuple(ldf))+ ";")
