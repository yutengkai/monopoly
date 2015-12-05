

def d(points, z):
    length = len(points)
    print("\tglBegin(GL_QUADS);")
    print("\t// draw front face")
    print("\tglNormal3f(0, 0, 1);")
    for i in points:
        new_p = i + [z]
        print("\tglVertex3f" + str(tuple(new_p))+ ";")
    #print("\tglVertex3f" + str(tuple(points[0]+[z]))+ ";")
    print("\t// draw back face")
    print("\tglNormal3f(0, 0, -1);")
    re_p = points[:]
    re_p.reverse()
    for i in re_p:
        new_p = i + [0]
        print("\tglVertex3f" + str(tuple(new_p))+ ";")
    #print("\tglVertex3f" + str(tuple(re_p[0] + [0]))+ ";")
    new_points = points + [points[0]]
    for i in range(length):
        p1 = new_points[i] + [0]
        p2 = new_points[i+1] + [0]
        p3 = new_points[i+1] + [z]
        p4 = new_points[i] + [z]
        normal = [p1[1] - p2[1], p2[0] - p1[0], 0]
        print("\t// draw face " + str(i))
        print("\tglNormal3f" + str(tuple(normal))+ ";")
        print("\tglVertex3f" + str(tuple(p1))+ ";")
        print("\tglVertex3f" + str(tuple(p2))+ ";")
        print("\tglVertex3f" + str(tuple(p3))+ ";")
        print("\tglVertex3f" + str(tuple(p4))+ ";")
        #print("\tglVertex3f" + str(tuple(p1))+ ";")
    print("\tglEnd();")
    
def big(w, l):
    print("\tglColor3f(0.90, 0.91, 0.98);")
    d([[-w,0], [-w, l], [w, l], [w, 0]], 0.32)
    print("\tglColor3f(0.80, 0.498039, 0.196078);")
    print("\tglTranslatef(0.0, 0.0, 0.32);")
    d([[-w/4,0], [-w/4,l/4], [w/4, l/4], [w/4, 0]], 0.02)
    print("\tglTranslatef(0.0, " + str(0.4*l) + ", 0.0);")
    print("\tglTranslatef(" + str(-w*0.6) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    #
    print("\tglTranslatef(0.0, " + str(0.2*l) + ", 0.0);")
    print("\tglTranslatef(" + str(-w*1.2) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    #
    print("\tglTranslatef(0.0, " + str(0.2*l) + ", 0.0);")
    print("\tglTranslatef(" + str(-w*1.2) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    print("\tglTranslatef(" + str(w*0.4) + ", 0.0, 0.0);")
    d([[-w/8,0], [-w/8,l*0.15], [w/8, l*0.15], [w/8, 0]], 0.02)
    

def small(w, l):
    print("\tglPushMatrix();")
    d([[-w,0], [-w, l], [w, l], [w, 0]], 0.32)
    print("\tglColor3f(0.80, 0.498039, 0.196078);")
    print("\tglPushMatrix();")
    print("\tglTranslatef(0.0, " + str(l) + ", 0.0);")
    d([[-w,0], [0, 0.4 * l], [0, 0.4 * l], [w, 0]], 0.32)
    print("\tglPopMatrix();")
    print("\tglTranslatef(0.0, 0.0, 0.32);")
    d([[-w/4,0], [-w/4,l/2], [w/4, l/2], [w/4, 0]], 0.02)
    print("\tglPopMatrix();")


    
    
def one():
    d([[-0.15, 0.15], [0.15, 0.15], [0.15, -0.15], [-0.15, -0.15]], 0.01)

def two():
    print("\tglTranslatef(0.2, 0.2, 0.0);")
    d([[-0.07, 0.07], [0.07, 0.07], [0.07, -0.07], [-0.07, -0.07]], 0.01)
    print("\tglTranslatef(-0.4, -0.4, 0.0);")
    d([[-0.07, 0.07], [0.07, 0.07], [0.07, -0.07], [-0.07, -0.07]], 0.01)

def question(k):
    print("\tglColor3f(0.80, 0.498039, 0.196078);")
    print("\tglPushMatrix();")
    d([[-k, 2*k], [k,2*k], [k,0], [-k, 0]],k)
    print("\tglTranslatef(0.0, " + str(k*3) + ", 0.0);")
    d([[-k, 3*k], [k,3*k], [k,0], [-k, 0]],k)
    print("\tglTranslatef(" + str(k*3) + ", " + str(k) + ", 0.0);")
    d([[-2*k, 2*k], [2*k,2*k], [2*k,0], [-2*k, 0]],k)
    print("\tglTranslatef(" + str(k) + ", " + str(k*2) + ", 0.0);")
    d([[-k, 6*k], [k,6*k], [k,0], [-k, 0]],k)
    print("\tglTranslatef(" + str(-k*4) + ", " + str(4*k) + ", 0.0);")
    d([[-3*k, 2*k], [3*k,2*k], [3*k,0], [-3*k, 0]],k)
    print("\tglTranslatef(" + str(-k*2) + ", " + str(-2*k) + ", 0.0);")
    d([[-k, 2*k], [k,2*k], [k,0], [-k, 0]],k)
    print("\tglPopMatrix();")

def three(k):
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(-k*3) + ", " + str(-k*3) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*6) + ", " + str(k*6) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)

def four(k):
    print("\tglTranslatef(" + str(-k*4) + ", " + str(-k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*8) + ", " + str(0) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(-k*8) + ", " + str(k*8) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*8) + ", " + str(0) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)

def five(k):
    print("\tglColor3f(0.0, 0.0, 1.0);")
    print("\tglPushMatrix();")
    three(k)
    print("\tglPopMatrix();")
    print("\tglPushMatrix();")
    print("\tglRotatef(90, 0.0, 0.0, 1.0);")
    three(k)
    print("\tglPopMatrix();")

def six(k):
    print("\tglTranslatef(" + str(-k*6) + ", " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*12) + ", " + str(0) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(-k*12) + ", " + str(-k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*12) + ", " + str(0) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(-k*12) + ", " + str(-k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)
    print("\tglTranslatef(" + str(k*12) + ", " + str(0) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.01)

def arrow(k):
    d([[-k, k], [k,2*k], [k,0], [-k, k]],0.01)
    d([[k, k*1.5], [3*k,k*1.5], [3*k,k/2], [k, k/2]],0.01)

def train(k):
    print("\tglPushMatrix();")
    print("\tglColor3f(0.0, 0.0, 0.0);")
    d([[-3*k, 0],[-4*k, 2*k],[4*k, 2*k],[3*k, 0]], 0.32)
    print("\tglTranslatef(0, " + str(k*2) + ", 0.0);")
    print("\tglColor3f(1.0, 0.0, 0.0);")
    d([[-4*k, 0],[-4*k, 9*k],[4*k, 9*k],[4*k, 0]], 0.32)
    print("\tglTranslatef(0, " + str(k) + ", 0.32);")
    print("\tglColor3f(1.0, 1.0, 0.0);")
    d([[-3*k, 0],[-3*k, 2*k],[-1*k, 2*k],[-1*k, 0]], 0.01)
    d([[1*k, 0],[1*k, 2*k],[3*k, 2*k],[3*k, 0]], 0.01)
    print("\tglTranslatef(0, " + str(k*3) + ", 0.0);")
    print("\tglColor3f(0.0, 0.0, 1.0);")
    d([[-3*k, 0],[-3*k, 3*k],[3*k, 3*k],[3*k, 0]], 0.01)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    print("\tglColor3f(1.0, 1.0, 0.0);")
    d([[-1*k, 0],[-1*k, k],[1*k, 1*k],[1*k, 0]], 0.01)
    print("\tglTranslatef(0, " + str(k) + ", -0.32);")
    print("\tglColor3f(0.0, 0.0, 0.0);")
    d([[-4*k, 0],[-3*k, k],[3*k, k],[4*k, 0]], 0.32)
    print("\tglPopMatrix();")

def jail(k):
    print("\tglPushMatrix();")
    print("\tglColor3f(0.0, 0.0, 0.0);")
    print("\tglTranslatef(" + str(-k*12) + ", " + str(-k*12) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglTranslatef(0, " + str(k*4) + ", 0.0);")
    d([[-k, k], [k,k], [k,-k], [-k, -k]],0.6)
    print("\tglPopMatrix();")

def jail_four(k):
    print("\tglPushMatrix();")
    jail(k)
    print("\tglRotatef(90, 0.0, 0.0, 1.0);")
    jail(k)
    print("\tglRotatef(90, 0.0, 0.0, 1.0);")
    jail(k)
    print("\tglRotatef(90, 0.0, 0.0, 1.0);")
    jail(k)
    print("\tglPopMatrix();")

def draw(l, k):
    print("\tglPushMatrix();")
    for i in range(len(l)):
        print("\tglPushMatrix();")
        for j in range(len(l[0])):
            color = l[i][j]
            if color == "b":
                print("\tglColor3f(0.0, 0.0, 0.0);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            elif color == "w":
                print("\tglColor3f(1.0, 1.0, 1.0);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            elif color == "r":
                print("\tglColor3f(1.0, 0.5
                      , 0.0);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            elif color == "p":
                print("\tglColor3f(0.737255, 0.560784, 0.560784);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            elif color == "y":
                print("\tglColor3f(1.0, 1.0, 0.0);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            elif color == "o":
                print("\tglColor3f(0.647059, 0.164706, 0.164706);")
                d([[-k, k], [k,k], [k,-k], [-k, -k]],0.32)
            print("\tglTranslatef(" + str(k*2) + ", " + str(0) + ", 0.0);")
        print("\tglPopMatrix();")
        print("\tglTranslatef(0, " + str(k*2) + ", 0.0);")
    print("\tglPopMatrix();")

s = [""]*16
s[0] = "brrbbbbbb0000000"
s[1] = "brrrbbbbbbbbrr00"
s[2] = "0brbbbbbbbbbrr00"
s[3] = "bpppbrbbbbbbrrrb"
s[4] = "bpppbrbbbbbbrrrb"
s[5] = "bpppbrrbwbbwbrrb"
s[6] = "0bbbrrrbrrrbbbb0"
s[7] = "00brrrbrrrbrb000"
s[8] = "0000bbpppppprb00"
s[9] = "000bbpppbbbbbb00"
s[10] ="00bppbbppbppprb0"
s[11] ="00bppbbpbpppprb0"
s[12] ="000bbbppbpbbbrb0"
s[13] ="000brrrrrrrrbppb"
s[14] ="0000brrrrrbbpppb"
s[15] ="00000bbbbb00bbb0"

p = []
p.append("00000000bb00000")
p.append("0bb0bbbbyyb0000")
p.append("byybyyyybbyb000")
p.append("0bbyyyyyybbb000")
p.append("0bybbyyrrrbrb00")
p.append("0byyyyyyyybrrb0")
p.append("0bybbyyrrrbrb00")
p.append("byyyyryyybbyyb0")
p.append("byybyyyyybybbb0")
p.append("0byyyyyybyyb000")
p.append("0byyyyyybbyyb00")
p.append("00byyyyyybyyyb0")
p.append("000bbbyybbbyyyb")
p.append("000000bbbbbbbbb")
p.append("000000000bb0000")

box = []
box.append("b"*16)
box.append("r"+"o"*14+"b")
box.append("robooooobbooobob")
box.append("roooooorrbooooob")
box.append("roooooorroooooob")
box.append("rooooooobbooooob")
box.append("roooooorrbooooob")
box.append("roooooorrbbbooob")
box.append("roooobborrrbooob")
box.append("rooorrboorrbooob")
box.append("rooorrboorrbooob")
box.append("rooorrbbbrroooob")
box.append("roooorrrrrooooob")
box.append("roboooooooooobob")
box.append("r"+"o"*14+"b")
box.append("b"+"r"*14+"b")




