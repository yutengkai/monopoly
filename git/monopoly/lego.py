import math

r = 100
z = 0.075

def board(l, w, a, b):
    



    luf = [a, l+b, 0]
    ldf = [a, b, 0]
    ruf = [w+a, l+b, 0]
    rdf = [w+a, b, 0]

    lub = luf[:]
    lub[2] = 0.05

    ldb = ldf[:]
    ldb[2] = 0.05

    rub = ruf[:]
    rub[2] = 0.05

    rdb = rdf[:]
    rdb[2] = 0.05
    print("\tglBegin(GL_QUADS);")
    print("\t// draw front face")
    print("\tglNormal3f(0, 0, 1);")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\t// draw back face")
    print("\tglNormal3f(0, 0, -1);")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\t// draw left face")
    print("\tglNormal3f(-1, 0, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw right face")
    print("\tglNormal3f(1, 0, 0);")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\t// draw top")
    print("\tglNormal3f(0, 1, 0);")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw bottom")
    print("\tglNormal3f(0, -1, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglEnd();")

def box(l1, w1, h1):
    l = l1/r
    w = w1/r
    h = h1/r

    board(h, w)
    print("glTranslatef(" + str(w) + ", 0, 0);")
    print("glRotatef(90, 0.0, 1.0, 0.0);")
    board(h, l)
    print("glTranslatef(" + str(l) + ", 0, 0);")
    print("glRotatef(90, 0.0, 1.0, 0.0);")
    board(h, w)
    print("glTranslatef(" + str(w) + ", 0, 0);")
    print("glRotatef(90, 0.0, 1.0, 0.0);")
    board(h, l)
    print("glTranslatef(0, " + str(h) + ", 0);")
    print("glRotatef(-90, 1.0, 0.0, 0.0);")
    print("glColor3f(1.0, 0.0, 0.0);")
    board(w, l)

def pad():
    print("glColor3f(1.0, 1.0, 0.0);")
    luf = [0, 0.085, 0]
    ldf = [0, 0, 0]
    ruf = [0.24, 0.085, 0]
    rdf = [0.24, 0, 0]

    lub = luf[:]
    lub[2] = -0.24

    ldb = ldf[:]
    ldb[2] = -0.24

    rub = ruf[:]
    rub[2] = -0.24

    rdb = rdf[:]
    rdb[2] = -0.24

    print("glBegin(GL_QUADS);")
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
    print("glEnd();")


def lego_one():
    box(39, 39, 48)
    print("glRotatef(90, 1.0, 0.0, 0.0);")
    print("glTranslatef(0.075, 0.0, -0.075);")
    pad()

def lego_two():
    box(39, 79, 16)
    print("glRotatef(90, 1.0, 0.0, 0.0);")
    print("glTranslatef(0.075, 0.0, -0.075);")
    pad()
    print("glTranslatef(0.0, 0.0, -0.4);")
    pad()
    
    
def mono_table(k):
    '''
    luf = [-13*k,  13*k, 0]
    ldf = [-13*k, -13*k, 0]
    ruf = [13*k, 13*k, 0]
    rdf = [13*k, -13*k, 0]

    lub = luf[:]
    lub[2] = -4*k

    ldb = ldf[:]
    ldb[2] = -4*k

    rub = ruf[:]
    rub[2] = -4*k

    rdb = rdf[:]
    rdb[2] = -4*k
    print("\tglColor3f(1.0, 1.0, 1.0);")
    print("\tglBegin(GL_QUADS);")
    print("\t// draw front face")
    print("\tglNormal3f(0, 0, 1);")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\t// draw back face")
    print("\tglNormal3f(0, 0, -1);")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\t// draw left face")
    print("\tglNormal3f(-1, 0, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw right face")
    print("\tglNormal3f(1, 0, 0);")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+
          ";")
    print("\t// draw top")
    print("\tglNormal3f(0, 1, 0);")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw bottom")
    print("\tglNormal3f(0, -1, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglEnd();")
    
    print("\tglColor3f(0.0, 0.0, 0.0);")
    board(0.02, 26*k, -13*k, -13*k)
    board(0.01, 18*k, -9*k, -9*k)
    for i in range(13):
        if i > 1 and i < 12:
            board(0.01, 4*k, -13*k, (2*i-13)*k)
    '''

def die(k):
    
    luf = [-2*k, 2*k, 2*k]
    ldf = [-2*k, -2*k, 2*k]
    ruf = [2*k, 2*k, 2*k]
    rdf = [2*k, -2*k, 2*k]

    lub = luf[:]
    lub[2] = -2*k

    ldb = ldf[:]
    ldb[2] = -2*k

    rub = ruf[:]
    rub[2] = -2*k

    rdb = rdf[:]
    rdb[2] = -2*k
    print("\tglColor3f(1.0, 1.0, 1.0);")
    print("\tglBegin(GL_QUADS);")
    print("\t// draw front face")
    print("\tglNormal3f(0, 0, 1);")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\t// draw back face")
    print("\tglNormal3f(0, 0, -1);")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\t// draw left face")
    print("\tglNormal3f(-1, 0, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw right face")
    print("\tglNormal3f(1, 0, 0);")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+
          ";")
    print("\t// draw top")
    print("\tglNormal3f(0, 1, 0);")
    print("\tglVertex3f" + str(tuple(luf))+ ";")
    print("\tglVertex3f" + str(tuple(ruf))+ ";")
    print("\tglVertex3f" + str(tuple(rub))+ ";")
    print("\tglVertex3f" + str(tuple(lub))+ ";")
    print("\t// draw bottom")
    print("\tglNormal3f(0, -1, 0);")
    print("\tglVertex3f" + str(tuple(ldb))+ ";")
    print("\tglVertex3f" + str(tuple(rdb))+ ";")
    print("\tglVertex3f" + str(tuple(rdf))+ ";")
    print("\tglVertex3f" + str(tuple(ldf))+ ";")
    print("\tglEnd();")

def one():
    die(0.08)


