// g++ main.cpp -o main.exe -lfreeglut -lopengl32 -lglu32
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>
#include <GL/freeglut.h>
#include <GL/glu.h>
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
using namespace std;
#define PI 3.1415926535 // 1 pi = 180 degrees

float roomSize = 20.0f, halfRoom = roomSize / 2.0f,
    camX = 0.0f, camY = 5.0f, camZ = 0.0f,
    camY_rotate = -90.0f, camX_rotate = 0.0f;
float spiderX = 0.0f, spiderZ = 0.0f, spiderY = 1.0f, spiderY_rotate = 0.0f;
float fov = 45.0f, animationTimer = 0.0f;
bool isDragging = false, isMoving = false,
    lightEnabled[4] = {true, true, true, true};
int lastX = -1, lastY = -1;
GLuint textures[3]; // 1: wall, 0: floor, 2: spider
GLUquadric *quadric;
GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};

void init(), reshape(int w, int h), createMenu(), menuCallback(int value);
void display(), setupLights(), drawSpider(), update(int value), drawRoom();
void drawQuad(GLfloat n[3], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3], GLfloat v3[3]);
GLuint loadBMP(const char *imagepath);
void specialKeys(int key, int x, int y), keyboard(unsigned char key, int x, int y),
     mouse(int button, int state, int x, int y), mouseMotion(int x, int y),
     mouseWheel(int wheel, int direction, int x, int y);

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Robot 8 Chan Tim Nguon Sang");
    init();
    glutDisplayFunc(display); glutReshapeFunc(reshape); 
    glutSpecialFunc(specialKeys); glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard); glutMotionFunc(mouseMotion); 
    glutMouseWheelFunc(mouseWheel);
    createMenu();
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}

void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST); glEnable(GL_NORMALIZE); glEnable(GL_TEXTURE_2D);
    textures[0] = loadBMP("wall.bmp"); textures[1] = loadBMP("floor.bmp"); textures[2] = loadBMP("spider.bmp");
    quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH); gluQuadricTexture(quadric, GL_TRUE);
    glEnable(GL_LIGHTING); glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    // look direction vectors
    float lookX = cos(camX_rotate * PI / 180.0) * cos(camY_rotate * PI / 180.0);
    float lookZ = cos(camX_rotate * PI / 180.0) * sin(camY_rotate * PI / 180.0);
    float lookY = sin(camX_rotate * PI / 180.0);

    gluLookAt(camX, camY, camZ,
        camX + lookX, camY + lookY, camZ + lookZ, // position the camera is looking at
        0.0f, 1.0f, 0.0f); // y = 1 -> up direction
    setupLights(); drawRoom(); drawSpider();
    glutSwapBuffers();
}

void setupLights() {
    float lights_offset = halfRoom - 0.1f;
    GLfloat lightPositions[4][4] = {
        {-lights_offset, roomSize - 0.1f, -lights_offset, 1.0f},
        {lights_offset, roomSize - 0.1f, -lights_offset, 1.0f},
        {lights_offset, roomSize - 0.1f, lights_offset, 1.0f},
        {-lights_offset, roomSize - 0.1f, lights_offset, 1.0f}};
    GLfloat lightColors[4][4] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {1.0f, 1.0f, 1.0f, 1.0f}, // White
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 1.0f, 1.0f}}; // Blue

    for (int i = 0; i < 4; ++i) {
        if (lightEnabled[i]) {
            glEnable(GL_LIGHT0 + i);
            glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPositions[i]);
            glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lightColors[i]);
            glLightfv(GL_LIGHT0 + i, GL_SPECULAR, lightColors[i]);
            glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambient);
            glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.0f);
            glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.005f); // lower : fades slower, light travels further
        } else glDisable(GL_LIGHT0 + i);
    }
}

void update(int value) {
    float target_x = 0.0f, target_z = 0.0f, targetY_rotate = 0.0f;
    bool light_found = false;
    // _____ Ox
    // |    -> red corner
    // Oz

    if (lightEnabled[0]) { 
        target_x = -halfRoom; target_z = -halfRoom;
        targetY_rotate = 135.0f;
    } // red
    else if (lightEnabled[1]) { 
        target_x = halfRoom; target_z = -halfRoom; 
        targetY_rotate = 45.0f; 
    } // white
    else if (lightEnabled[2]) { 
        target_x = halfRoom; target_z = halfRoom; 
        targetY_rotate = -45.0f; 
    } // green
    else if (lightEnabled[3]) { 
        target_x = -halfRoom; target_z = halfRoom; 
        targetY_rotate = -135.0f; 
    } // blue
    if (target_x != 0.0f || target_z != 0.0f) light_found = true;

    if (light_found) {
        float dx = target_x - spiderX, dz = target_z - spiderZ;
        float distance = sqrt(dx * dx + dz * dz);

        if (distance > 4.0f) { // offset from corner
            isMoving = true;
            float yRotate_diff = targetY_rotate - spiderY_rotate;
            // convert to shortest rotation
            if (yRotate_diff > 180.0f) yRotate_diff -= 360.0f;
            if (yRotate_diff < -180.0f) yRotate_diff += 360.0f;
            spiderY_rotate += yRotate_diff * 0.05f;
            spiderX += dx * 0.005f; spiderZ += dz * 0.005f;

            float boundary = halfRoom - 2.75f;
            if (spiderX < -boundary) spiderX = -boundary;
            if (spiderX > boundary) spiderX = boundary;
            if (spiderZ < -boundary) spiderZ = -boundary;
            if (spiderZ > boundary) spiderZ = boundary;

            animationTimer += 0.1f;
        }  else isMoving = false; // reached target
    } else isMoving = false;
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void drawLimb(float length) {
    glPushMatrix();
    glTranslatef(length / 2.0f, 0.0f, 0.0f);
    glScalef(length, 0.08f, 0.08f); // stretch x: length, yz: thickness
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawSpider() {
    glPushMatrix(); 
    // update new position and direction it's facing
    glTranslatef(spiderX, spiderY, spiderZ);
    glRotatef(spiderY_rotate, 0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f); glEnable(GL_TEXTURE_2D); 
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glPushMatrix(); // body
    gluSphere(quadric, 1.0, 8, 8);
    glPopMatrix();

    glPushMatrix(); // head
    glTranslatef(1.2f, 0.0f, 0.0f); gluSphere(quadric, 0.5, 20, 20);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // glPushMatrix();              // nose
    // glColor3f(1.0f, 0.0f, 0.0f); // nose color
    // glTranslatef(1.7f, 0.0f, 0.0f);
    // glutSolidSphere(0.1, 10, 10);
    // glPopMatrix();

    glPushMatrix(); // eyes
    glColor3f(1.0f, 0.0f, 0.0f); // eye color
    glTranslatef(1.4f, 0.0f, 0.2f); glutSolidSphere(0.25, 10, 10);
    // glPopMatrix(); // end of left eye
    // glPushMatrix(); // second eye
    // glTranslatef(1.4f, 0.0f, -0.2f); glutSolidSphere(0.25, 8, 8);
    glTranslatef(0.0f, 0.0f, -0.4f); glutSolidSphere(0.25, 10, 10);
    glPopMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);

    float upper_length = 1.3f, lower_length = 2.5f;
    float idle_upper = 65.0f, // how high the leg go up from the body
          idle_lower = -120.0f; // knee joint (further from 0 -> bent more)
    float upper_angle, lower_angle;

    for (int i = 0; i < 10; ++i) {
        glPushMatrix();
        // i/2: group odd and even into pairs on x axis
        float side = (i % 2 == 0) ? 1.0f : -1.0f; // side +/even, -/odd
        float x = -0.6f + (i / 2) * 0.4f;
        // -0.6: starting x for first pair, 0.4: distance between each pair
        float z = sqrt(1.0f - (x * x)) * 0.8f; // Z position on the side of the body

        if (isMoving) {
            float phase = (i / 2) * (PI / 2.0f); // each pair (4 total) has different starting point in rotation cycle (pi/2 -> 4pi/2)
            if (side < 0) phase += PI; // odd legs start from opposite side
            // sin, cos: -1 to +1 -> sin * amp = -amp to +amp
            // 15, 20 (amplitude): how far the legs swing
            upper_angle = idle_upper + sin(animationTimer * 5.0f + phase) * 15.0f;
            lower_angle = idle_lower + cos(animationTimer * 5.0f + phase) * 20.0f;
        } else {
            upper_angle = idle_upper; lower_angle = idle_lower;
        }

        if (i == 8 || i == 9) { // whiskers
            // glTranslatef(1.4f, 0.0f, 0.2f);
            // glRotatef(side * -90.0f + x * side * 50.0f, 0.0f, 1.0f, 0.0f);
            // // 50: how fan out the legs are

            // glRotatef(upper_angle, 0.0f, 0.0f, 1.0f);
            // drawLimb(upper_length);
            // glTranslatef(upper_length, 0.0f, 0.0f); // form the knee
            // glRotatef(lower_angle, 0.0f, 0.0f, 1.0f);
            // drawLimb(lower_length);
        } else {
            glTranslatef(x, 0.0f, side * z); // position upper leg    
            glRotatef(side * -90.0f + x * side * 50.0f, 0.0f, 1.0f, 0.0f);
            // 50: how fan out the legs are

            glRotatef(upper_angle, 0.0f, 0.0f, 1.0f); drawLimb(upper_length);
            glTranslatef(upper_length, 0.0f, 0.0f); // form the knee
            glRotatef(lower_angle, 0.0f, 0.0f, 1.0f); drawLimb(lower_length);
        }
        glPopMatrix(); // End of this leg
    }
    glPopMatrix();
}

void drawQuad(GLfloat n[3], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3], GLfloat v3[3]) {
    glNormal3fv(n); glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3fv(v0);
    glTexCoord2f(3.0f, 0.0f); glVertex3fv(v1);
    glTexCoord2f(3.0f, 3.0f); glVertex3fv(v2);
    glTexCoord2f(0.0f, 3.0f); glVertex3fv(v3);
    glEnd();
}

void drawRoom() {
    glEnable(GL_TEXTURE_2D); glColor3f(1.0f, 1.0f, 1.0f);
    GLfloat b_front_left[3] = {-halfRoom, 0.0f, halfRoom},
            b_front_right[3] = {halfRoom, 0.0f, halfRoom},
            b_back_left[3] = {-halfRoom, 0.0f, -halfRoom},
            b_back_right[3] = {halfRoom, 0.0f, -halfRoom},
            t_front_left[3] = {-halfRoom, roomSize, halfRoom},
            t_front_right[3] = {halfRoom, roomSize, halfRoom},
            t_back_left[3] = {-halfRoom, roomSize, -halfRoom},
            t_back_right[3] = {halfRoom, roomSize, -halfRoom},
            up[3] = {0.0f, 1.0f, 0.0f}, down[3] = {0.0f, -1.0f, 0.0f},
            front[3] = {0.0f, 0.0f, -1.0f}, back[3] = {0.0f, 0.0f, 1.0f},
            left[3] = {1.0f, 0.0f, 0.0f}, right[3] = {-1.0f, 0.0f, 0.0f};

    glBindTexture(GL_TEXTURE_2D, textures[1]); // floor
    drawQuad(up, b_back_left, b_back_right, b_front_right, b_front_left);
    glBindTexture(GL_TEXTURE_2D, textures[0]); // ceiling
    drawQuad(down, t_back_left, t_back_right, t_front_right, t_front_left);
    glBindTexture(GL_TEXTURE_2D, textures[0]); // walls
    drawQuad(back, b_back_left, b_back_right, t_back_right, t_back_left); // Back
    drawQuad(front, b_front_right, b_front_left, t_front_left, t_front_right); // Front
    drawQuad(left, b_front_left, b_back_left, t_back_left, t_front_left); // Left
    drawQuad(right, b_back_right, b_front_right, t_front_right, t_back_right); // Right
    glDisable(GL_TEXTURE_2D);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (double)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void specialKeys(int key, int x, int y) { // 0.5 : speed
    float radian = camY_rotate * PI / 180.0;
    if (key == GLUT_KEY_UP) {
        camX += cos(radian) * 0.5f; camZ += sin(radian) * 0.5f;
    } else if (key == GLUT_KEY_DOWN) {
        camX -= cos(radian) * 0.5f; camZ -= sin(radian) * 0.5f;
    } else if (key == GLUT_KEY_LEFT) {
        camX += cos(radian - PI/2) * 0.5f; camZ += sin(radian - PI/2) * 0.5f;
    } else if (key == GLUT_KEY_RIGHT) {
        camX += cos(radian + PI/2) * 0.5f; camZ += sin(radian + PI/2) * 0.5f;
    }
    if (camX > 9.0f) camX = 9.0f; else if (camX < -9.0f) camX = -9.0f;
    if (camZ > 9.0f) camZ = 9.0f; else if (camZ < -9.0f) camZ = -9.0f;   
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'w') camY += 0.5f; else if (key == 's') camY -= 0.5f;
    if (camY > 19.0f) camY = 19.0f; else if (camY < 2.0f) camY = 2.0f;
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isDragging = true; lastX = x; lastY = y;
    } else isDragging = false;
}

void mouseMotion(int x, int y) {
    if (isDragging) { // 0.2f : sensitivity
        camY_rotate += (x - lastX) * 0.2f; camX_rotate -= (y - lastY) * 0.2f;
        if (camX_rotate > 89.0f) camX_rotate = 89.0f;
        if (camX_rotate < -89.0f) camX_rotate = -89.0f;
        lastX = x; lastY = y;
        glutPostRedisplay();
    }
}

void mouseWheel(int wheel, int direction, int x, int y) {
    if (direction > 0) fov -= 1.0f; // Zoom in
    else fov += 1.0f; // Zoom out
    if (fov < 15.0f) fov = 15.0f; if (fov > 90.0f) fov = 90.0f;
    reshape(glutGet(GLUT_WINDOW_WIDTH), 
    glutGet(GLUT_WINDOW_HEIGHT));
    glutPostRedisplay();
}

enum MENU_ITEMS { LIGHT0, LIGHT1, LIGHT2, LIGHT3, LIGHTS };

string title(string text, int index) {
    if (index == -1) 
        return "All " + string(lightEnabled[0] || lightEnabled[1] || lightEnabled[2] || lightEnabled[3] ? "On" : "Off");
    return text + " " + string(lightEnabled[index] ? "On" : "Off");
}

void createMenu() {
    if (glutGetMenu()) glutDestroyMenu(glutGetMenu());
    int light_menu = glutCreateMenu(menuCallback);
    glutAddMenuEntry(title("Red", 0).c_str(), LIGHT0); 
    glutAddMenuEntry(title("White", 1).c_str(), LIGHT1);
    glutAddMenuEntry(title("Green", 2).c_str(), LIGHT2); 
    glutAddMenuEntry(title("Blue", 3).c_str(), LIGHT3);
    glutAddMenuEntry("---", -1); // Separator
    glutAddMenuEntry(title("All", -1).c_str(), LIGHTS);
    glutCreateMenu(menuCallback);
    glutAddSubMenu("Lights", light_menu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void menuCallback(int value) {
    if (value >= LIGHT0 && value <= LIGHT3) {
        lightEnabled[value - LIGHT0] = !lightEnabled[value - LIGHT0];
        createMenu();
    } else if (value == LIGHTS) {
        bool state = lightEnabled[0] || lightEnabled[1] || lightEnabled[2] || lightEnabled[3]; // true: one or more lights are on, false: all lights are off
        for (int i = 0; i < 4; ++i) {
            if (state) lightEnabled[i] = false;
            else lightEnabled[i] = true;
        }
        createMenu();
    }
    glutPostRedisplay();
}

GLuint loadBMP(const char *imagepath) {
    unsigned char header[54], *data;
    FILE *file = fopen(imagepath, "rb");

    if (!file) return 0;
    if (fread(header, 1, 54, file) != 54 ||
        header[0] != 'B' || header[1] != 'M' || // Must be BMP
        *(int *)&(header[0x1E]) != 0 || // Must be uncompressed
        *(int *)&(header[0x1C]) != 24)  // Must be 24-bit
    { fclose(file); return 0; }
        
    unsigned int dataPos = *(int *)&(header[0x0A]), 
        imageSize = *(int *)&(header[0x22]),
        w = *(int *)&(header[0x12]), h = *(int *)&(header[0x16]);

    if (imageSize == 0) imageSize = w * h * 3;
    if (dataPos == 0) dataPos = 54;
    data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file); fclose(file);

    for (unsigned int i = 0; i < imageSize; i += 3) {
        unsigned char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    } // Swap BGR to RGB

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[] data;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return textureID;
}