/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "keyframe.h"
#include "timer.h"
#include "vector.h"


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

const float SPINNER_SPEED = 0.1;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -7.5;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Render settings
enum { WIREFRAME, SOLID, OUTLINED, METALLIC, MATTE };	// README: the different render styles
int renderStyle = WIREFRAME;			// README: the selected render style

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate

// Keyframe settings
// there is a file storing keyframes for animating a dice roll
// there is a file storing keyframes for each individual dice value
// there is a file storing keyframes that will be used to move game pieces
const char filenameKF[] = "keyframe-files/dice-rolling.txt";
const char filenameDV[] = "keyframe-files/dice-values.txt";
const char filenameMP[] = "keyframe-files/move-pieces.txt";

// lists of keyframes
Keyframe* keyframes;
Keyframe* diceValues;
Keyframe* movePieces;

// indices of max VALID keyframe (in keyframe list)
int maxValidKeyframe1   = 0;
int maxValidKeyframe2   = 0;
int maxValidKeyframe3   = 0;

const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
const float ROOT_TRANSLATE_X_MIN = -5.0;
const float ROOT_TRANSLATE_X_MAX =  5.0;
const float ROOT_TRANSLATE_Y_MIN = -5.0;
const float ROOT_TRANSLATE_Y_MAX =  5.0;
const float ROOT_TRANSLATE_Z_MIN = -5.0;
const float ROOT_TRANSLATE_Z_MAX =  5.0;
const float ROOT_ROTATE_X_MIN    = -180.0;
const float ROOT_ROTATE_X_MAX    =  180.0;
const float ROOT_ROTATE_Y_MIN    = -180.0;
const float ROOT_ROTATE_Y_MAX    =  180.0;
const float ROOT_ROTATE_Z_MIN    = -180.0;
const float ROOT_ROTATE_Z_MAX    =  180.0;
const float DICE_MIN   			 =  0.0;
const float DICE_MAX   			 =  360.0;
const float SHOULDER_ROLL_MIN    = -45.0;
const float SHOULDER_ROLL_MAX    =  45.0;
const float HIP_PITCH_MIN        = -45.0;
const float HIP_PITCH_MAX        =  45.0;
const float HIP_YAW_MIN          = -45.0;
const float HIP_YAW_MAX          =  45.0;
const float HIP_ROLL_MIN         = -45.0;
const float HIP_ROLL_MAX         =  45.0;
const float BEAK_MIN             =  0.0;
const float BEAK_MAX             =  0.4;
const float ELBOW_MIN            =  0.0;
const float ELBOW_MAX            = 75.0;
const float KNEE_MIN             =  0.0;
const float KNEE_MAX             = 75.0;
const float LIGHT_ANGLE_MIN		 = -360;
const float LIGHT_ANGLE_MAX		 = 360;



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);

// search
void mono_table();
void table_bar_h_l();
void dice();
void condo();
void house();
void houses();
void jail();
void jail_four();
void question_sign();
void roll_dice();
void go_sign();
void train();
void mario();
void pikachu();
void chest();
void rollDiceButton();
void loadKeyframesFromFileButton();
int dice_rolling;
int is_rolling = 1;
int current_player = 0;

// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

    loadKeyframesFromFileButton();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	diceValues = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		diceValues[i].setID(i);

	movePieces = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		movePieces[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}

void moveX(int player, float current_x, float movement) {
	joint_ui_data->setDOF(player, current_x + movement);
	movePieces[current_player].setDOF(player, current_x + movement);
	keyframes[0].setDOF(player, current_x + movement);
	keyframes[1].setDOF(player, current_x + movement);
}

void moveY(int player, float current_y, float movement) {
	joint_ui_data->setDOF(player, current_y + movement);
	movePieces[current_player].setDOF(player, current_y + movement);
	keyframes[0].setDOF(player, current_y + movement);
	keyframes[1].setDOF(player, current_y + movement);
}

void movePiece(int dice_roll) {
	float num_spaces = (dice_roll + 1) * 0.64f;

	int playerx = current_player == 0 ? Keyframe::PLAYER1_X : Keyframe::PLAYER2_X;
	int playery = current_player == 0 ? Keyframe::PLAYER1_Y : Keyframe::PLAYER2_Y;

	float current_x = movePieces[current_player].getDOF(playerx);
	float current_y = movePieces[current_player].getDOF(playery);
	float x_movement = 0.0;
	float y_movement = 0.0;

	printf("player %d \n", current_player + 1);
	printf("origin X: %f \n", current_x);
	printf("origin Y: %f \n", current_y);
	
	for (float i=0.0; i<num_spaces; i=i+0.64 ) {
		if ((current_x + x_movement) <= 0.0f && (current_x + x_movement) > -6.4f) {
			if (current_y > 0.0f) {
				//if ((current_x + x_movement) <= -6.4f) {
					// on corner square. add an additional x and y movement to cover its size
					//x_movement = x_movement + 0.64;
					//y_movement = y_movement - 0.64;
				//} else {
					//x_movement = x_movement + 0.64;
				//}
			} else {
				if ((current_x + x_movement) < -5.12f) {
					// on last square in the row.
					// move two spots left and one spot up
					x_movement = x_movement - 1.28;
					y_movement = y_movement + 0.64;
				} else {
					x_movement = x_movement - 0.64;
				}
			}
		} else if ((current_y + y_movement) > 0.0f && (current_y + y_movement) < 7.04f) {
			if (current_x < 0.0f) {
				if ((current_y + y_movement) > 5.76f) {
					// on last square in the row.
					// move two spots up and one spot right
					y_movement = y_movement + 1.28;
					x_movement = x_movement + 0.64;
				} else {
					y_movement = y_movement + 0.64;
				}
			} else {
				/*if ((current_y + y_movement) >= 7.04f) {
					// on corner square
					x_movement = x_movement + 0.64;
					y_movement = y_movement + 0.64;
				} else {
					y_movement = y_movement + 0.64;
				}*/
			}
		}
	}
	
	printf("total x mov: %f \n", x_movement);
	printf("total y mov: %f \n", y_movement);


	moveX(playerx, current_x, x_movement);
	moveY(playery, current_y, y_movement);

	current_player = current_player == 0 ? 1 : 0;
	
	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	//sprintf(msg, "You moved %d spaces!", dice_roll + 1);
	//status->set_text(msg);
}

// there is a keyframes file containing dice rolls that is loaded
// when the program is initialized
void stopRoll() {
	// generates number between 0 and 5
	int dice_roll = rand() % 6;

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	joint_ui_data->setDOF(Keyframe::DICE_X, diceValues[dice_roll].getDOF(Keyframe::DICE_X));
	joint_ui_data->setDOF(Keyframe::DICE_Y, diceValues[dice_roll].getDOF(Keyframe::DICE_Y));

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	sprintf(msg, "You rolled a %d!", dice_roll + 1);
	status->set_text(msg);

	movePiece(dice_roll);
}

// roll dice button handler.  Called when the "animate" button is pressed.
void rollDiceButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 ) {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Rolling... Click Stop when ready");
	status->set_text(msg);
  } else {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	stopRoll();
  }
}

/*
// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}


// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
	///////////////////////////////////////////////////////////
	// TODO:
	//   Modify this function to save the UI joint values into
	//   the appropriate keyframe entry in the keyframe list
	//   when the user clicks on the 'Update Keyframe' button.
	//   Refer to the 'loadKeyframeButton' function for help.
	///////////////////////////////////////////////////////////

	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'maxValidKeyframe' index variable
	// (it will be needed when doing the interpolation)
	maxValidKeyframe = keyframeID;

	// Update the appropriate entry in the 'keyframes' array
	// with the 'joint_ui_data' data
	keyframes[keyframeID] = *joint_ui_data;

	// Let the user know the values have been updated
	sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
	status->set_text(msg);
}
*/

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton()
{
	// Open file for reading
	FILE* file1 = fopen(filenameKF, "r");
	FILE* file2 = fopen(filenameDV, "r");
	FILE* file3 = fopen(filenameMP, "r");
	if( file1 == NULL || file2 == NULL || file3 == NULL )
	{
		sprintf(msg, "Status: Failed to open a file");
		status->set_text(msg);
		return;
	}

	// --------------- Dice Rolling Keyframe File ------------

	// Read in maxValidKeyframe first
	fscanf(file1, "%d", &maxValidKeyframe1);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe1; i++ ) {
		fscanf(file1, "%d", keyframes[i].getIDPtr());
		fscanf(file1, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file1, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file1);

	// --------------- Dice Values Keyframe File ------------
	
	fscanf(file2, "%d", &maxValidKeyframe2);

	for( int i = 0; i <= maxValidKeyframe2; i++ ) {
		fscanf(file2, "%d", diceValues[i].getIDPtr());
		fscanf(file2, "%f", diceValues[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file2, "%f", diceValues[i].getDOFPtr(j));
	}

	fclose(file2);

	// --------------- Move Pieces Keyframe File ------------

	fscanf(file3, "%d", &maxValidKeyframe3);

	for( int i = 0; i <= maxValidKeyframe3; i++ ) {
		fscanf(file3, "%d", movePieces[i].getIDPtr());
		fscanf(file3, "%f", movePieces[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file3, "%f", movePieces[i].getDOFPtr(j));
	}

	fclose(file3);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}
/*
// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
	// Open file for writing
	FILE* file = fopen(filenameKF, "w");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Write out maxValidKeyframe first
	fprintf(file, "%d\n", maxValidKeyframe);
	fprintf(file, "\n");

	// Now write out all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fprintf(file, "%d\n", keyframes[i].getID());
		fprintf(file, "%f\n", keyframes[i].getTime());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fprintf(file, "%f\n", keyframes[i].getDOF(j));

		fprintf(file, "\n");
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been saved
	sprintf(msg, "Status: Keyframes saved successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
	// Calculate number of frames to generate based on dump frame rate
	int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

	// Generate frames and save to file
	frameToFile = 1;
	for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
	{
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

		// Let the user know which frame is being rendered
		sprintf(msg, "Status: Rendering frame %d...", frameNumber);
		status->set_text(msg);

		// Render the frame
		display();
	}
	frameToFile = 0;

	// Let the user know how many frames were generated
	sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
	status->set_text(msg);
}
*/
// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);


	// Create GLUI window (joint controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    // Create controls to specify root position and orientation
	glui_panel = glui_joints->add_panel("Root");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
	glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(4 * SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
	glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(4 * SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
	glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(4 * SPINNER_SPEED);


	glui_joints->add_column(false);


	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("Dice");
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "Dice x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::DICE_X));
	glui_spinner->set_float_limits(DICE_MIN, DICE_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(25 * SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "Dice y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::DICE_Y));
	glui_spinner->set_float_limits(DICE_MIN, DICE_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(25 * SPINNER_SPEED);

	glui_keyframe->add_button_to_panel(glui_panel, "Roll / Stop Dice", 0, rollDiceButton);


	///////////////////////////////////////////////////////////
	// TODO (for controlling light source position & additional
	//      rendering styles):
	//   Add more UI spinner elements here. Be sure to also
	//   add the appropriate min/max range values to this
	//   file, and to also add the appropriate enums to the
	//   enumeration in the Keyframe class (keyframe.h).
	///////////////////////////////////////////////////////////

	//
	// ***************************************************


	// Create GLUI window (keyframe controls) ************
	//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	//glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	//glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	//glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	//glui_keyframe->add_column_to_panel(glui_panel, false);
	//glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	//glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	//glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************


	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

	// Create control to specify the render style
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Solid w/ outlines");
	glui_render->add_radiobutton_to_group(glui_radio_group, "metallic");
	glui_render->add_radiobutton_to_group(glui_radio_group, "matte");

	//light 
	glui_panel = glui_render->add_panel("Light");
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "angle:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::LIGHT_ANGLE));
	glui_spinner->set_float_limits(LIGHT_ANGLE_MIN, LIGHT_ANGLE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	//
	// ***************************************************


	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe1 && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe1 )
		return keyframes[maxValidKeyframe1].getDOFVector();

	// Need to normalize time to (0, 1]
	float alpha = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	///////////////////////////////////////////////////////////
	// TODO (animation using Catmull-Rom):
	//   Currently the code operates using linear interpolation
    //   Modify this function so it uses Catmull-Rom instead.
	///////////////////////////////////////////////////////////

	// Return the linearly interpolated Vector
	return p0 * (1-alpha) + p1 * alpha;
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    glEnable(GL_DEPTH_TEST);

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);


	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe1].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		///////////////////////////////////////////////////////////
		// README:
		//   This statement loads the interpolated joint DOF vector
		//   into the global 'joint_ui_data' variable. Use the
		//   'joint_ui_data' variable below in your model code to
		//   drive the model for animation.
		///////////////////////////////////////////////////////////
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();
	}


    ///////////////////////////////////////////////////////////
    // TODO:
	//   Modify this function to draw the scene.
	//   This should include function calls that apply
	//   the appropriate transformation matrices and render
	//   the individual body parts.
	//   Use the 'joint_ui_data' data structure to obtain
	//   the joint DOFs to specify your transformations.
	//   Sample code is provided below and demonstrates how
	//   to access the joint DOF values. This sample code
	//   should be replaced with your own.
	//   Use the 'renderStyle' variable and the associated
	//   enumeration to determine how the geometry should be
	//   rendered.
    ///////////////////////////////////////////////////////////

	// SAMPLE CODE **********
	//
	glPushMatrix(); 

		// setup transformation for body part
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z));

		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y), 0.0, 1.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_X), 1.0, 0.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0.0, 0.0, 1.0);

		// determine render style and set glPolygonMode appropriately
		//search
		// draw body part
		
		
		glPushMatrix();
		glTranslatef(0.0, 0.0, -2.0);
		mono_table();

		go_sign();
		jail_four();
		glPushMatrix();
			table_bar_h_l();
			glRotatef(90, 0.0, 0.0, 1.0);
			table_bar_h_l();
			glRotatef(90, 0.0, 0.0, 1.0);
			table_bar_h_l();
			glRotatef(90, 0.0, 0.0, 1.0);
			table_bar_h_l();
		glPopMatrix();
		
		// buildings
		houses();

		glPopMatrix();

		// bars
		glPushMatrix();
			glRotatef(joint_ui_data->getDOF(Keyframe::DICE_X), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::DICE_Y), 0.0, 1.0, 0.0);
			dice();
		glPopMatrix();
		
		


		

	glPopMatrix();
	
	/*if (renderStyle == SOLID) drawpenguin();

	if (renderStyle == WIREFRAME) drawline();

	if (renderStyle == OUTLINED) drawboth();

	if (renderStyle == METALLIC) drawmetal();

	if (renderStyle == MATTE) drawmatte();*/
	//
	// SAMPLE CODE **********

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

	// Dump frame to file, if requested
	if( frameToFile )
	{
		sprintf(filenameF, "frame%03d.ppm", frameNumber);
		writeFrame(filenameF, false, false);
	}

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}


// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube



///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
			/*
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%c",int(buffer[x+y*width];
			}
			*/
		}
	}
	else
	{
		fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%d ",int(buffer[x+y*width]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
									int(pix[GREEN_OFFSET]),
									int(pix[BLUE_OFFSET]));
			}
		}
	}
	else
	{
		fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
									   int(pix[GREEN_OFFSET]),
									   int(pix[BLUE_OFFSET]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
	static GLubyte* frameData = NULL;
	static int currentSize = -1;

	int size = (pgm ? 1 : 4);

	if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
	{
		if (frameData != NULL)
			delete [] frameData;

		currentSize = size*Win[0]*Win[1];

		frameData = new GLubyte[currentSize];
	}

	glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

	if( pgm )
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
		writePGM(filename, frameData, Win[0], Win[1]);
	}
	else
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_RGBA, GL_UNSIGNED_BYTE, frameData);
		writePPM(filename, frameData, Win[0], Win[1]);
	}
}


void mono_table()
{		
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -4.16, 0);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(4.16, 4.16, 0);
	glVertex3f(-4.16, 4.16, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(4.16, -4.16, -1.28);
	glVertex3f(-4.16, -4.16, -1.28);
	glVertex3f(-4.16, 4.16, -1.28);
	glVertex3f(4.16, 4.16, -1.28);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -4.16, -1.28);
	glVertex3f(-4.16, -4.16, 0);
	glVertex3f(-4.16, 4.16, 0);
	glVertex3f(-4.16, 4.16, -1.28);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(4.16, -4.16, -1.28);
	glVertex3f(4.16, 4.16, -1.28);
	glVertex3f(4.16, 4.16, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 4.16, 0);
	glVertex3f(4.16, 4.16, 0);
	glVertex3f(4.16, 4.16, -1.28);
	glVertex3f(-4.16, 4.16, -1.28);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -4.16, -1.28);
	glVertex3f(4.16, -4.16, -1.28);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(-4.16, -4.16, 0);
	glEnd();
}

void table_bar_h_l()
{ 
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -4.16, 0);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(4.16, -4.140000000000001, 0);
	glVertex3f(-4.16, -4.140000000000001, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(4.16, -4.16, 0.1);
	glVertex3f(-4.16, -4.16, 0.1);
	glVertex3f(-4.16, -4.140000000000001, 0.1);
	glVertex3f(4.16, -4.140000000000001, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -4.16, 0.1);
	glVertex3f(-4.16, -4.16, 0);
	glVertex3f(-4.16, -4.140000000000001, 0);
	glVertex3f(-4.16, -4.140000000000001, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(4.16, -4.16, 0.1);
	glVertex3f(4.16, -4.140000000000001, 0.1);
	glVertex3f(4.16, -4.140000000000001, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -4.140000000000001, 0);
	glVertex3f(4.16, -4.140000000000001, 0);
	glVertex3f(4.16, -4.140000000000001, 0.1);
	glVertex3f(-4.16, -4.140000000000001, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -4.16, 0.1);
	glVertex3f(4.16, -4.16, 0.1);
	glVertex3f(4.16, -4.16, 0);
	glVertex3f(-4.16, -4.16, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-2.88, -2.88, 0);
	glVertex3f(2.88, -2.88, 0);
	glVertex3f(2.88, -2.87, 0);
	glVertex3f(-2.88, -2.87, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(2.88, -2.88, 0.1);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(-2.88, -2.87, 0.1);
	glVertex3f(2.88, -2.87, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(-2.88, -2.88, 0);
	glVertex3f(-2.88, -2.87, 0);
	glVertex3f(-2.88, -2.87, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(2.88, -2.88, 0);
	glVertex3f(2.88, -2.88, 0.1);
	glVertex3f(2.88, -2.87, 0.1);
	glVertex3f(2.88, -2.87, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-2.88, -2.87, 0);
	glVertex3f(2.88, -2.87, 0);
	glVertex3f(2.88, -2.87, 0.1);
	glVertex3f(-2.88, -2.87, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(2.88, -2.88, 0.1);
	glVertex3f(2.88, -2.88, 0);
	glVertex3f(-2.88, -2.88, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -2.88, 0);
	glVertex3f(-2.88, -2.88, 0);
	glVertex3f(-2.88, -2.87, 0);
	glVertex3f(-4.16, -2.87, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(-4.16, -2.88, 0.1);
	glVertex3f(-4.16, -2.87, 0.1);
	glVertex3f(-2.88, -2.87, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -2.88, 0.1);
	glVertex3f(-4.16, -2.88, 0);
	glVertex3f(-4.16, -2.87, 0);
	glVertex3f(-4.16, -2.87, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, -2.88, 0);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(-2.88, -2.87, 0.1);
	glVertex3f(-2.88, -2.87, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -2.87, 0);
	glVertex3f(-2.88, -2.87, 0);
	glVertex3f(-2.88, -2.87, 0.1);
	glVertex3f(-4.16, -2.87, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -2.88, 0.1);
	glVertex3f(-2.88, -2.88, 0.1);
	glVertex3f(-2.88, -2.88, 0);
	glVertex3f(-4.16, -2.88, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -2.24, 0);
	glVertex3f(-2.88, -2.24, 0);
	glVertex3f(-2.88, -2.2300000000000004, 0);
	glVertex3f(-4.16, -2.2300000000000004, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, -2.24, 0.1);
	glVertex3f(-4.16, -2.24, 0.1);
	glVertex3f(-4.16, -2.2300000000000004, 0.1);
	glVertex3f(-2.88, -2.2300000000000004, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -2.24, 0.1);
	glVertex3f(-4.16, -2.24, 0);
	glVertex3f(-4.16, -2.2300000000000004, 0);
	glVertex3f(-4.16, -2.2300000000000004, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, -2.24, 0);
	glVertex3f(-2.88, -2.24, 0.1);
	glVertex3f(-2.88, -2.2300000000000004, 0.1);
	glVertex3f(-2.88, -2.2300000000000004, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -2.2300000000000004, 0);
	glVertex3f(-2.88, -2.2300000000000004, 0);
	glVertex3f(-2.88, -2.2300000000000004, 0.1);
	glVertex3f(-4.16, -2.2300000000000004, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -2.24, 0.1);
	glVertex3f(-2.88, -2.24, 0.1);
	glVertex3f(-2.88, -2.24, 0);
	glVertex3f(-4.16, -2.24, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -1.6, 0);
	glVertex3f(-2.88, -1.6, 0);
	glVertex3f(-2.88, -1.59, 0);
	glVertex3f(-4.16, -1.59, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, -1.6, 0.1);
	glVertex3f(-4.16, -1.6, 0.1);
	glVertex3f(-4.16, -1.59, 0.1);
	glVertex3f(-2.88, -1.59, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -1.6, 0.1);
	glVertex3f(-4.16, -1.6, 0);
	glVertex3f(-4.16, -1.59, 0);
	glVertex3f(-4.16, -1.59, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, -1.6, 0);
	glVertex3f(-2.88, -1.6, 0.1);
	glVertex3f(-2.88, -1.59, 0.1);
	glVertex3f(-2.88, -1.59, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -1.59, 0);
	glVertex3f(-2.88, -1.59, 0);
	glVertex3f(-2.88, -1.59, 0.1);
	glVertex3f(-4.16, -1.59, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -1.6, 0.1);
	glVertex3f(-2.88, -1.6, 0.1);
	glVertex3f(-2.88, -1.6, 0);
	glVertex3f(-4.16, -1.6, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -0.96, 0);
	glVertex3f(-2.88, -0.96, 0);
	glVertex3f(-2.88, -0.95, 0);
	glVertex3f(-4.16, -0.95, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, -0.96, 0.1);
	glVertex3f(-4.16, -0.96, 0.1);
	glVertex3f(-4.16, -0.95, 0.1);
	glVertex3f(-2.88, -0.95, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -0.96, 0.1);
	glVertex3f(-4.16, -0.96, 0);
	glVertex3f(-4.16, -0.95, 0);
	glVertex3f(-4.16, -0.95, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, -0.96, 0);
	glVertex3f(-2.88, -0.96, 0.1);
	glVertex3f(-2.88, -0.95, 0.1);
	glVertex3f(-2.88, -0.95, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -0.95, 0);
	glVertex3f(-2.88, -0.95, 0);
	glVertex3f(-2.88, -0.95, 0.1);
	glVertex3f(-4.16, -0.95, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -0.96, 0.1);
	glVertex3f(-2.88, -0.96, 0.1);
	glVertex3f(-2.88, -0.96, 0);
	glVertex3f(-4.16, -0.96, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, -0.32, 0);
	glVertex3f(-2.88, -0.32, 0);
	glVertex3f(-2.88, -0.31, 0);
	glVertex3f(-4.16, -0.31, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, -0.32, 0.1);
	glVertex3f(-4.16, -0.32, 0.1);
	glVertex3f(-4.16, -0.31, 0.1);
	glVertex3f(-2.88, -0.31, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, -0.32, 0.1);
	glVertex3f(-4.16, -0.32, 0);
	glVertex3f(-4.16, -0.31, 0);
	glVertex3f(-4.16, -0.31, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, -0.32, 0);
	glVertex3f(-2.88, -0.32, 0.1);
	glVertex3f(-2.88, -0.31, 0.1);
	glVertex3f(-2.88, -0.31, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, -0.31, 0);
	glVertex3f(-2.88, -0.31, 0);
	glVertex3f(-2.88, -0.31, 0.1);
	glVertex3f(-4.16, -0.31, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, -0.32, 0.1);
	glVertex3f(-2.88, -0.32, 0.1);
	glVertex3f(-2.88, -0.32, 0);
	glVertex3f(-4.16, -0.32, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, 0.32, 0);
	glVertex3f(-2.88, 0.32, 0);
	glVertex3f(-2.88, 0.33, 0);
	glVertex3f(-4.16, 0.33, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, 0.32, 0.1);
	glVertex3f(-4.16, 0.32, 0.1);
	glVertex3f(-4.16, 0.33, 0.1);
	glVertex3f(-2.88, 0.33, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, 0.32, 0.1);
	glVertex3f(-4.16, 0.32, 0);
	glVertex3f(-4.16, 0.33, 0);
	glVertex3f(-4.16, 0.33, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, 0.32, 0);
	glVertex3f(-2.88, 0.32, 0.1);
	glVertex3f(-2.88, 0.33, 0.1);
	glVertex3f(-2.88, 0.33, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 0.33, 0);
	glVertex3f(-2.88, 0.33, 0);
	glVertex3f(-2.88, 0.33, 0.1);
	glVertex3f(-4.16, 0.33, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, 0.32, 0.1);
	glVertex3f(-2.88, 0.32, 0.1);
	glVertex3f(-2.88, 0.32, 0);
	glVertex3f(-4.16, 0.32, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, 0.96, 0);
	glVertex3f(-2.88, 0.96, 0);
	glVertex3f(-2.88, 0.97, 0);
	glVertex3f(-4.16, 0.97, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, 0.96, 0.1);
	glVertex3f(-4.16, 0.96, 0.1);
	glVertex3f(-4.16, 0.97, 0.1);
	glVertex3f(-2.88, 0.97, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, 0.96, 0.1);
	glVertex3f(-4.16, 0.96, 0);
	glVertex3f(-4.16, 0.97, 0);
	glVertex3f(-4.16, 0.97, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, 0.96, 0);
	glVertex3f(-2.88, 0.96, 0.1);
	glVertex3f(-2.88, 0.97, 0.1);
	glVertex3f(-2.88, 0.97, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 0.97, 0);
	glVertex3f(-2.88, 0.97, 0);
	glVertex3f(-2.88, 0.97, 0.1);
	glVertex3f(-4.16, 0.97, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, 0.96, 0.1);
	glVertex3f(-2.88, 0.96, 0.1);
	glVertex3f(-2.88, 0.96, 0);
	glVertex3f(-4.16, 0.96, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, 1.6, 0);
	glVertex3f(-2.88, 1.6, 0);
	glVertex3f(-2.88, 1.61, 0);
	glVertex3f(-4.16, 1.61, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, 1.6, 0.1);
	glVertex3f(-4.16, 1.6, 0.1);
	glVertex3f(-4.16, 1.61, 0.1);
	glVertex3f(-2.88, 1.61, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, 1.6, 0.1);
	glVertex3f(-4.16, 1.6, 0);
	glVertex3f(-4.16, 1.61, 0);
	glVertex3f(-4.16, 1.61, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, 1.6, 0);
	glVertex3f(-2.88, 1.6, 0.1);
	glVertex3f(-2.88, 1.61, 0.1);
	glVertex3f(-2.88, 1.61, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 1.61, 0);
	glVertex3f(-2.88, 1.61, 0);
	glVertex3f(-2.88, 1.61, 0.1);
	glVertex3f(-4.16, 1.61, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, 1.6, 0.1);
	glVertex3f(-2.88, 1.6, 0.1);
	glVertex3f(-2.88, 1.6, 0);
	glVertex3f(-4.16, 1.6, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, 2.24, 0);
	glVertex3f(-2.88, 2.24, 0);
	glVertex3f(-2.88, 2.25, 0);
	glVertex3f(-4.16, 2.25, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, 2.24, 0.1);
	glVertex3f(-4.16, 2.24, 0.1);
	glVertex3f(-4.16, 2.25, 0.1);
	glVertex3f(-2.88, 2.25, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, 2.24, 0.1);
	glVertex3f(-4.16, 2.24, 0);
	glVertex3f(-4.16, 2.25, 0);
	glVertex3f(-4.16, 2.25, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, 2.24, 0);
	glVertex3f(-2.88, 2.24, 0.1);
	glVertex3f(-2.88, 2.25, 0.1);
	glVertex3f(-2.88, 2.25, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 2.25, 0);
	glVertex3f(-2.88, 2.25, 0);
	glVertex3f(-2.88, 2.25, 0.1);
	glVertex3f(-4.16, 2.25, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, 2.24, 0.1);
	glVertex3f(-2.88, 2.24, 0.1);
	glVertex3f(-2.88, 2.24, 0);
	glVertex3f(-4.16, 2.24, 0);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-4.16, 2.88, 0);
	glVertex3f(-2.88, 2.88, 0);
	glVertex3f(-2.88, 2.8899999999999997, 0);
	glVertex3f(-4.16, 2.8899999999999997, 0);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-2.88, 2.88, 0.1);
	glVertex3f(-4.16, 2.88, 0.1);
	glVertex3f(-4.16, 2.8899999999999997, 0.1);
	glVertex3f(-2.88, 2.8899999999999997, 0.1);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-4.16, 2.88, 0.1);
	glVertex3f(-4.16, 2.88, 0);
	glVertex3f(-4.16, 2.8899999999999997, 0);
	glVertex3f(-4.16, 2.8899999999999997, 0.1);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(-2.88, 2.88, 0);
	glVertex3f(-2.88, 2.88, 0.1);
	glVertex3f(-2.88, 2.8899999999999997, 0.1);
	glVertex3f(-2.88, 2.8899999999999997, 0);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-4.16, 2.8899999999999997, 0);
	glVertex3f(-2.88, 2.8899999999999997, 0);
	glVertex3f(-2.88, 2.8899999999999997, 0.1);
	glVertex3f(-4.16, 2.8899999999999997, 0.1);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-4.16, 2.88, 0.1);
	glVertex3f(-2.88, 2.88, 0.1);
	glVertex3f(-2.88, 2.88, 0);
	glVertex3f(-4.16, 2.88, 0);
	glEnd();
}

void dice()
{
	glColor3f(0.90, 0.91, 0.98);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.4, -0.4, 0.4);
	glVertex3f(0.4, -0.4, 0.4);
	glVertex3f(0.4, 0.4, 0.4);
	glVertex3f(-0.4, 0.4, 0.4);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.4, -0.4, -0.4);
	glVertex3f(-0.4, -0.4, -0.4);
	glVertex3f(-0.4, 0.4, -0.4);
	glVertex3f(0.4, 0.4, -0.4);
	// draw left face
	glNormal3f(-1, 0, 0);
	glVertex3f(-0.4, -0.4, -0.4);
	glVertex3f(-0.4, -0.4, 0.4);
	glVertex3f(-0.4, 0.4, 0.4);
	glVertex3f(-0.4, 0.4, -0.4);
	// draw right face
	glNormal3f(1, 0, 0);
	glVertex3f(0.4, -0.4, 0.4);
	glVertex3f(0.4, -0.4, -0.4);
	glVertex3f(0.4, 0.4, -0.4);
	glVertex3f(0.4, 0.4, 0.4);
	// draw top
	glNormal3f(0, 1, 0);
	glVertex3f(-0.4, 0.4, 0.4);
	glVertex3f(0.4, 0.4, 0.4);
	glVertex3f(0.4, 0.4, -0.4);
	glVertex3f(-0.4, 0.4, -0.4);
	// draw bottom
	glNormal3f(0, -1, 0);
	glVertex3f(-0.4, -0.4, -0.4);
	glVertex3f(0.4, -0.4, -0.4);
	glVertex3f(0.4, -0.4, 0.4);
	glVertex3f(-0.4, -0.4, 0.4);
	glEnd();
	glPushMatrix();
		glTranslatef(0.0, 0.0, 0.4);
		glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_QUADS);
			// draw front face
			glNormal3f(0, 0, 1);
			glVertex3f(-0.15, 0.15, 0.01);
			glVertex3f(0.15, 0.15, 0.01);
			glVertex3f(0.15, -0.15, 0.01);
			glVertex3f(-0.15, -0.15, 0.01);
			// draw back face
			glNormal3f(0, 0, -1);
			glVertex3f(-0.15, -0.15, 0);
			glVertex3f(0.15, -0.15, 0);
			glVertex3f(0.15, 0.15, 0);
			glVertex3f(-0.15, 0.15, 0);
			// draw face 0
			glNormal3f(0.0, 0.3, 0);
			glVertex3f(-0.15, 0.15, 0);
			glVertex3f(0.15, 0.15, 0);
			glVertex3f(0.15, 0.15, 0.01);
			glVertex3f(-0.15, 0.15, 0.01);
			// draw face 1
			glNormal3f(0.3, 0.0, 0);
			glVertex3f(0.15, 0.15, 0);
			glVertex3f(0.15, -0.15, 0);
			glVertex3f(0.15, -0.15, 0.01);
			glVertex3f(0.15, 0.15, 0.01);
			// draw face 2
			glNormal3f(0.0, -0.3, 0);
			glVertex3f(0.15, -0.15, 0);
			glVertex3f(-0.15, -0.15, 0);
			glVertex3f(-0.15, -0.15, 0.01);
			glVertex3f(0.15, -0.15, 0.01);
			// draw face 3
			glNormal3f(-0.3, 0.0, 0);
			glVertex3f(-0.15, -0.15, 0);
			glVertex3f(-0.15, 0.15, 0);
			glVertex3f(-0.15, 0.15, 0.01);
			glVertex3f(-0.15, -0.15, 0.01);
			glEnd();
	glPopMatrix();

	glPushMatrix();
		glRotatef(90, 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.0, 0.4);
		glTranslatef(0.2, 0.2, 0.0);
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.07, 0.07, 0.01);
		glVertex3f(0.07, 0.07, 0.01);
		glVertex3f(0.07, -0.07, 0.01);
		glVertex3f(-0.07, -0.07, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(-0.07, 0.07, 0);
		// draw face 0
		glNormal3f(0.0, 0.14, 0);
		glVertex3f(-0.07, 0.07, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(0.07, 0.07, 0.01);
		glVertex3f(-0.07, 0.07, 0.01);
		// draw face 1
		glNormal3f(0.14, 0.0, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(0.07, -0.07, 0.01);
		glVertex3f(0.07, 0.07, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.14, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(-0.07, -0.07, 0.01);
		glVertex3f(0.07, -0.07, 0.01);
		// draw face 3
		glNormal3f(-0.14, 0.0, 0);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(-0.07, 0.07, 0);
		glVertex3f(-0.07, 0.07, 0.01);
		glVertex3f(-0.07, -0.07, 0.01);
		glEnd();
		glTranslatef(-0.4, -0.4, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.07, 0.07, 0.01);
		glVertex3f(0.07, 0.07, 0.01);
		glVertex3f(0.07, -0.07, 0.01);
		glVertex3f(-0.07, -0.07, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(-0.07, 0.07, 0);
		// draw face 0
		glNormal3f(0.0, 0.14, 0);
		glVertex3f(-0.07, 0.07, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(0.07, 0.07, 0.01);
		glVertex3f(-0.07, 0.07, 0.01);
		// draw face 1
		glNormal3f(0.14, 0.0, 0);
		glVertex3f(0.07, 0.07, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(0.07, -0.07, 0.01);
		glVertex3f(0.07, 0.07, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.14, 0);
		glVertex3f(0.07, -0.07, 0);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(-0.07, -0.07, 0.01);
		glVertex3f(0.07, -0.07, 0.01);
		// draw face 3
		glNormal3f(-0.14, 0.0, 0);
		glVertex3f(-0.07, -0.07, 0);
		glVertex3f(-0.07, 0.07, 0);
		glVertex3f(-0.07, 0.07, 0.01);
		glVertex3f(-0.07, -0.07, 0.01);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glRotatef(90, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 0.4);
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(-0.15000000000000002, -0.15000000000000002, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(0.30000000000000004, 0.30000000000000004, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glRotatef(-90, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 0.4);
		glColor3f(1.0, 0.0, 0.0);
		glTranslatef(-0.16, -0.16, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(0.32, 0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(-0.32, 0.32, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(0.32, 0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.0, 0.4);
		glColor3f(0.0, 0.0, 1.0);
		glPushMatrix();
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(-0.15, -0.15, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(0.3, 0.3, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glPopMatrix();
		glPushMatrix();
		glRotatef(90, 0.0, 0.0, 1.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(-0.15, -0.15, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glTranslatef(0.3, 0.3, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		// draw face 0
		glNormal3f(0.0, 0.1, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, 0.05, 0.01);
		glVertex3f(-0.05, 0.05, 0.01);
		// draw face 1
		glNormal3f(0.1, 0.0, 0);
		glVertex3f(0.05, 0.05, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(0.05, -0.05, 0.01);
		glVertex3f(0.05, 0.05, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.1, 0);
		glVertex3f(0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, -0.05, 0.01);
		glVertex3f(0.05, -0.05, 0.01);
		// draw face 3
		glNormal3f(-0.1, 0.0, 0);
		glVertex3f(-0.05, -0.05, 0);
		glVertex3f(-0.05, 0.05, 0);
		glVertex3f(-0.05, 0.05, 0.01);
		glVertex3f(-0.05, -0.05, 0.01);
		glEnd();
		glPopMatrix();
	glPopMatrix();

	glPushMatrix();
		glRotatef(90, 1.0, 0.0, 0.0);
		glRotatef(90, 1.0, 0.0, 0.0);
		glTranslatef(0.0, 0.0, 0.4);
		glColor3f(0.0, 0.0, 1.0);
		glTranslatef(-0.24, 0.16, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(0.48, 0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(-0.48, -0.16, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(0.48, 0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(-0.48, -0.16, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
		glTranslatef(0.48, 0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		// draw face 0
		glNormal3f(0.0, 0.08, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, 0.04, 0.01);
		glVertex3f(-0.04, 0.04, 0.01);
		// draw face 1
		glNormal3f(0.08, 0.0, 0);
		glVertex3f(0.04, 0.04, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(0.04, -0.04, 0.01);
		glVertex3f(0.04, 0.04, 0.01);
		// draw face 2
		glNormal3f(0.0, -0.08, 0);
		glVertex3f(0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, -0.04, 0.01);
		glVertex3f(0.04, -0.04, 0.01);
		// draw face 3
		glNormal3f(-0.08, 0.0, 0);
		glVertex3f(-0.04, -0.04, 0);
		glVertex3f(-0.04, 0.04, 0);
		glVertex3f(-0.04, 0.04, 0.01);
		glVertex3f(-0.04, -0.04, 0.01);
		glEnd();
	glPopMatrix();
}

void condo()
{
	glPushMatrix();
	glTranslatef(0.0, 0.96, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glColor3f(0.90, 0.91, 0.98);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(-0.25, 1.2, 0.32);
	glVertex3f(0.25, 1.2, 0.32);
	glVertex3f(0.25, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0.25, 1.2, 0);
	glVertex3f(-0.25, 1.2, 0);
	glVertex3f(-0.25, 0, 0);
	// draw face 0
	glNormal3f(-1.2, 0.0, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(-0.25, 1.2, 0);
	glVertex3f(-0.25, 1.2, 0.32);
	glVertex3f(-0.25, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0.5, 0);
	glVertex3f(-0.25, 1.2, 0);
	glVertex3f(0.25, 1.2, 0);
	glVertex3f(0.25, 1.2, 0.32);
	glVertex3f(-0.25, 1.2, 0.32);
	// draw face 2
	glNormal3f(1.2, 0.0, 0);
	glVertex3f(0.25, 1.2, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0.25, 0, 0.32);
	glVertex3f(0.25, 1.2, 0.32);
	// draw face 3
	glNormal3f(0, -0.5, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(0.25, 0, 0.32);
	glEnd();
	glColor3f(0.80, 0.498039, 0.196078);
	glTranslatef(0.0, 0.0, 0.32);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.0625, 0, 0.02);
	glVertex3f(-0.0625, 0.3, 0.02);
	glVertex3f(0.0625, 0.3, 0.02);
	glVertex3f(0.0625, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(0.0625, 0.3, 0);
	glVertex3f(-0.0625, 0.3, 0);
	glVertex3f(-0.0625, 0, 0);
	// draw face 0
	glNormal3f(-0.3, 0.0, 0);
	glVertex3f(-0.0625, 0, 0);
	glVertex3f(-0.0625, 0.3, 0);
	glVertex3f(-0.0625, 0.3, 0.02);
	glVertex3f(-0.0625, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.125, 0);
	glVertex3f(-0.0625, 0.3, 0);
	glVertex3f(0.0625, 0.3, 0);
	glVertex3f(0.0625, 0.3, 0.02);
	glVertex3f(-0.0625, 0.3, 0.02);
	// draw face 2
	glNormal3f(0.3, 0.0, 0);
	glVertex3f(0.0625, 0.3, 0);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(0.0625, 0, 0.02);
	glVertex3f(0.0625, 0.3, 0.02);
	// draw face 3
	glNormal3f(0, -0.125, 0);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(-0.0625, 0, 0);
	glVertex3f(-0.0625, 0, 0.02);
	glVertex3f(0.0625, 0, 0.02);
	glEnd();
	glTranslatef(0.0, 0.48, 0.0);
	glTranslatef(-0.15, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.0, 0.24, 0.0);
	glTranslatef(-0.3, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.0, 0.24, 0.0);
	glTranslatef(-0.3, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glTranslatef(0.1, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(-0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.0625, 0);
	glVertex3f(-0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0.18, 0.02);
	glVertex3f(-0.03125, 0.18, 0.02);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.03125, 0.18, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(0.03125, 0, 0.02);
	glVertex3f(0.03125, 0.18, 0.02);
	// draw face 3
	glNormal3f(0, -0.0625, 0);
	glVertex3f(0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0);
	glVertex3f(-0.03125, 0, 0.02);
	glVertex3f(0.03125, 0, 0.02);
	glEnd();
	glPopMatrix();
}

void house()
{	
	glPushMatrix();
	glTranslatef(0.0, 0.96, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(-0.25, 0.5, 0.32);
	glVertex3f(0.25, 0.5, 0.32);
	glVertex3f(0.25, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0.25, 0.5, 0);
	glVertex3f(-0.25, 0.5, 0);
	glVertex3f(-0.25, 0, 0);
	// draw face 0
	glNormal3f(-0.5, 0.0, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(-0.25, 0.5, 0);
	glVertex3f(-0.25, 0.5, 0.32);
	glVertex3f(-0.25, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0.5, 0);
	glVertex3f(-0.25, 0.5, 0);
	glVertex3f(0.25, 0.5, 0);
	glVertex3f(0.25, 0.5, 0.32);
	glVertex3f(-0.25, 0.5, 0.32);
	// draw face 2
	glNormal3f(0.5, 0.0, 0);
	glVertex3f(0.25, 0.5, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0.25, 0, 0.32);
	glVertex3f(0.25, 0.5, 0.32);
	// draw face 3
	glNormal3f(0, -0.5, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(0.25, 0, 0.32);
	glEnd();
	glColor3f(0.80, 0.498039, 0.196078);
	glPushMatrix();
	glTranslatef(0.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(0, 0.2, 0.32);
	glVertex3f(0, 0.2, 0.32);
	glVertex3f(0.25, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(-0.25, 0, 0);
	// draw face 0
	glNormal3f(-0.2, 0.25, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(0, 0.2, 0.32);
	glVertex3f(-0.25, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(0, 0.2, 0.32);
	glVertex3f(0, 0.2, 0.32);
	// draw face 2
	glNormal3f(0.2, 0.25, 0);
	glVertex3f(0, 0.2, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(0.25, 0, 0.32);
	glVertex3f(0, 0.2, 0.32);
	// draw face 3
	glNormal3f(0, -0.5, 0);
	glVertex3f(0.25, 0, 0);
	glVertex3f(-0.25, 0, 0);
	glVertex3f(-0.25, 0, 0.32);
	glVertex3f(0.25, 0, 0.32);
	glEnd();
	glPopMatrix();
	glTranslatef(0.0, 0.0, 0.32);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.0625, 0, 0.02);
	glVertex3f(-0.0625, 0.25, 0.02);
	glVertex3f(0.0625, 0.25, 0.02);
	glVertex3f(0.0625, 0, 0.02);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(0.0625, 0.25, 0);
	glVertex3f(-0.0625, 0.25, 0);
	glVertex3f(-0.0625, 0, 0);
	// draw face 0
	glNormal3f(-0.25, 0.0, 0);
	glVertex3f(-0.0625, 0, 0);
	glVertex3f(-0.0625, 0.25, 0);
	glVertex3f(-0.0625, 0.25, 0.02);
	glVertex3f(-0.0625, 0, 0.02);
	// draw face 1
	glNormal3f(0.0, 0.125, 0);
	glVertex3f(-0.0625, 0.25, 0);
	glVertex3f(0.0625, 0.25, 0);
	glVertex3f(0.0625, 0.25, 0.02);
	glVertex3f(-0.0625, 0.25, 0.02);
	// draw face 2
	glNormal3f(0.25, 0.0, 0);
	glVertex3f(0.0625, 0.25, 0);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(0.0625, 0, 0.02);
	glVertex3f(0.0625, 0.25, 0.02);
	// draw face 3
	glNormal3f(0, -0.125, 0);
	glVertex3f(0.0625, 0, 0);
	glVertex3f(-0.0625, 0, 0);
	glVertex3f(-0.0625, 0, 0.02);
	glVertex3f(0.0625, 0, 0.02);
	glEnd();
	glPopMatrix();
}

void houses()
{
	glPushMatrix();
		glPushMatrix();
			glTranslatef(-0.64 * 4, -4, 0);
			glColor3f(0.74902, 0.847059, 0.847059);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.74902, 0.847059, 0.847059);
			house();
			glTranslatef(0.64, 0, 0);
			question_sign();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.74902, 0.847059, 0.847059);
			house();
			glTranslatef(0.64, 0, 0);
			train();
			glTranslatef(0.64, 0, 0);
			glTranslatef(0.64, 0, 0);
			glColor3f(0.737255, 0.560784, 0.560784);
			house();
			glTranslatef(0.64, 0, 0);
			chest();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.737255, 0.560784, 0.560784);
			house();
			glTranslatef(0.64, 0, 0);

			glTranslatef(joint_ui_data->getDOF(Keyframe::PLAYER1_X), joint_ui_data->getDOF(Keyframe::PLAYER1_Y), 0.0);
			mario();
			glTranslatef(-joint_ui_data->getDOF(Keyframe::PLAYER1_X), -joint_ui_data->getDOF(Keyframe::PLAYER1_Y), 0.0);
			
			glTranslatef(joint_ui_data->getDOF(Keyframe::PLAYER2_X), joint_ui_data->getDOF(Keyframe::PLAYER2_Y), 0.0);
			pikachu();
			glTranslatef(-joint_ui_data->getDOF(Keyframe::PLAYER2_X), -joint_ui_data->getDOF(Keyframe::PLAYER2_Y), 0.0);

		glPopMatrix();

		glPushMatrix();
			glRotatef(90, 0.0, 0.0, 1.0);
			glTranslatef(-0.64 * 4, -4, 0);
			glColor3f(0.137255, 0.137255, 0.556863);
			house();
			glTranslatef(0.64, 0, 0);
			glTranslatef(0.64, 0, 0);
			glColor3f(0.137255, 0.137255, 0.556863);
			house();
			glTranslatef(0.64, 0, 0);
			question_sign();
			glTranslatef(0.64, 0, 0);
			train();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.5, 1.0, 0.5);
			house();
			glTranslatef(0.64, 0, 0);
			chest();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.5, 1.0, 0.5);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.5, 1.0, 0.5);
			house();
		glPopMatrix();

		glPushMatrix();
			glRotatef(90, 0.0, 0.0, 1.0);
			glRotatef(90, 0.0, 0.0, 1.0);
			glTranslatef(-0.64 * 4, -4, 0);
			glColor3f(1.0, 1.0, 0.0);
			house();
			glTranslatef(0.64, 0, 0);
			condo();
			glTranslatef(0.64, 0, 0);
			glColor3f(1.0, 1.0, 0.0);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(1.0, 1.0, 0.0);
			house();
			glTranslatef(0.64, 0, 0);
			train();
			glTranslatef(0.64, 0, 0);
			glColor3f(1.0, 0.0, 0.0);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(1.0, 0.0, 0.0);
			house();
			glTranslatef(0.64, 0, 0);
			question_sign();
			glTranslatef(0.64, 0, 0);
			glColor3f(1.0, 0.0, 0.0);
			house();
		glPopMatrix();

		glPushMatrix();
			glRotatef(-90, 0.0, 0.0, 1.0);
			glTranslatef(-0.64 * 4, -4, 0);
			glColor3f(0.90, 0.91, 0.98);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.90, 0.91, 0.98);
			house();
			glTranslatef(0.64, 0, 0);
			chest();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.90, 0.91, 0.98);
			house();
			glTranslatef(0.64, 0, 0);
			train();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.73, 0.16, 0.96);
			house();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.73, 0.16, 0.96);
			house();
			glTranslatef(0.64, 0, 0);
			condo();
			glTranslatef(0.64, 0, 0);
			glColor3f(0.73, 0.16, 0.96);
			house();
		glPopMatrix();
	glPopMatrix();
}

void question_sign()
{
	glColor3f(0.80, 0.498039, 0.196078);
	glPushMatrix();
	glTranslatef(0.0, 0.8, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(0.0, 0.1, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.04, 0.08, 0.04);
	glVertex3f(0.04, 0.08, 0.04);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0);
	// draw face 0
	glNormal3f(0.0, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(0.04, 0.08, 0.04);
	glVertex3f(-0.04, 0.08, 0.04);
	// draw face 1
	glNormal3f(0.08, 0.0, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(0.04, 0.08, 0.04);
	// draw face 2
	glNormal3f(0, -0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0, 0.04);
	glVertex3f(0.04, 0, 0.04);
	// draw face 3
	glNormal3f(-0.08, 0.0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	glEnd();
	glTranslatef(0.0, 0.12, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.04, 0.12, 0.04);
	glVertex3f(0.04, 0.12, 0.04);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0.12, 0);
	glVertex3f(-0.04, 0.12, 0);
	// draw face 0
	glNormal3f(0.0, 0.08, 0);
	glVertex3f(-0.04, 0.12, 0);
	glVertex3f(0.04, 0.12, 0);
	glVertex3f(0.04, 0.12, 0.04);
	glVertex3f(-0.04, 0.12, 0.04);
	// draw face 1
	glNormal3f(0.12, 0.0, 0);
	glVertex3f(0.04, 0.12, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(0.04, 0.12, 0.04);
	// draw face 2
	glNormal3f(0, -0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0, 0.04);
	glVertex3f(0.04, 0, 0.04);
	// draw face 3
	glNormal3f(-0.12, 0.0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0.12, 0);
	glVertex3f(-0.04, 0.12, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	glEnd();
	glTranslatef(0.12, 0.04, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.08, 0.08, 0.04);
	glVertex3f(0.08, 0.08, 0.04);
	glVertex3f(0.08, 0, 0.04);
	glVertex3f(-0.08, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.08, 0, 0);
	glVertex3f(0.08, 0, 0);
	glVertex3f(0.08, 0.08, 0);
	glVertex3f(-0.08, 0.08, 0);
	// draw face 0
	glNormal3f(0.0, 0.16, 0);
	glVertex3f(-0.08, 0.08, 0);
	glVertex3f(0.08, 0.08, 0);
	glVertex3f(0.08, 0.08, 0.04);
	glVertex3f(-0.08, 0.08, 0.04);
	// draw face 1
	glNormal3f(0.08, 0.0, 0);
	glVertex3f(0.08, 0.08, 0);
	glVertex3f(0.08, 0, 0);
	glVertex3f(0.08, 0, 0.04);
	glVertex3f(0.08, 0.08, 0.04);
	// draw face 2
	glNormal3f(0, -0.16, 0);
	glVertex3f(0.08, 0, 0);
	glVertex3f(-0.08, 0, 0);
	glVertex3f(-0.08, 0, 0.04);
	glVertex3f(0.08, 0, 0.04);
	// draw face 3
	glNormal3f(-0.08, 0.0, 0);
	glVertex3f(-0.08, 0, 0);
	glVertex3f(-0.08, 0.08, 0);
	glVertex3f(-0.08, 0.08, 0.04);
	glVertex3f(-0.08, 0, 0.04);
	glEnd();
	glTranslatef(0.04, 0.08, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.04, 0.24, 0.04);
	glVertex3f(0.04, 0.24, 0.04);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0.24, 0);
	glVertex3f(-0.04, 0.24, 0);
	// draw face 0
	glNormal3f(0.0, 0.08, 0);
	glVertex3f(-0.04, 0.24, 0);
	glVertex3f(0.04, 0.24, 0);
	glVertex3f(0.04, 0.24, 0.04);
	glVertex3f(-0.04, 0.24, 0.04);
	// draw face 1
	glNormal3f(0.24, 0.0, 0);
	glVertex3f(0.04, 0.24, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(0.04, 0.24, 0.04);
	// draw face 2
	glNormal3f(0, -0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0, 0.04);
	glVertex3f(0.04, 0, 0.04);
	// draw face 3
	glNormal3f(-0.24, 0.0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0.24, 0);
	glVertex3f(-0.04, 0.24, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	glEnd();
	glTranslatef(-0.16, 0.16, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.12, 0.08, 0.04);
	glVertex3f(0.12, 0.08, 0.04);
	glVertex3f(0.12, 0, 0.04);
	glVertex3f(-0.12, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.12, 0, 0);
	glVertex3f(0.12, 0, 0);
	glVertex3f(0.12, 0.08, 0);
	glVertex3f(-0.12, 0.08, 0);
	// draw face 0
	glNormal3f(0.0, 0.24, 0);
	glVertex3f(-0.12, 0.08, 0);
	glVertex3f(0.12, 0.08, 0);
	glVertex3f(0.12, 0.08, 0.04);
	glVertex3f(-0.12, 0.08, 0.04);
	// draw face 1
	glNormal3f(0.08, 0.0, 0);
	glVertex3f(0.12, 0.08, 0);
	glVertex3f(0.12, 0, 0);
	glVertex3f(0.12, 0, 0.04);
	glVertex3f(0.12, 0.08, 0.04);
	// draw face 2
	glNormal3f(0, -0.24, 0);
	glVertex3f(0.12, 0, 0);
	glVertex3f(-0.12, 0, 0);
	glVertex3f(-0.12, 0, 0.04);
	glVertex3f(0.12, 0, 0.04);
	// draw face 3
	glNormal3f(-0.08, 0.0, 0);
	glVertex3f(-0.12, 0, 0);
	glVertex3f(-0.12, 0.08, 0);
	glVertex3f(-0.12, 0.08, 0.04);
	glVertex3f(-0.12, 0, 0.04);
	glEnd();
	glTranslatef(-0.08, -0.08, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.04, 0.08, 0.04);
	glVertex3f(0.04, 0.08, 0.04);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0);
	// draw face 0
	glNormal3f(0.0, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(0.04, 0.08, 0.04);
	glVertex3f(-0.04, 0.08, 0.04);
	// draw face 1
	glNormal3f(0.08, 0.0, 0);
	glVertex3f(0.04, 0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(0.04, 0, 0.04);
	glVertex3f(0.04, 0.08, 0.04);
	// draw face 2
	glNormal3f(0, -0.08, 0);
	glVertex3f(0.04, 0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0, 0.04);
	glVertex3f(0.04, 0, 0.04);
	// draw face 3
	glNormal3f(-0.08, 0.0, 0);
	glVertex3f(-0.04, 0, 0);
	glVertex3f(-0.04, 0.08, 0);
	glVertex3f(-0.04, 0.08, 0.04);
	glVertex3f(-0.04, 0, 0.04);
	glEnd();
	glPopMatrix();
}

void go_sign()
{
	glPushMatrix();
	glTranslatef(3.2, -3.95, 0.1);
	glRotatef(90, 1.0, 0.0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.2, 0.2, 0.01);
	glVertex3f(0.2, 0.4, 0.01);
	glVertex3f(0.2, 0, 0.01);
	glVertex3f(-0.2, 0.2, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.2, 0.2, 0);
	glVertex3f(0.2, 0, 0);
	glVertex3f(0.2, 0.4, 0);
	glVertex3f(-0.2, 0.2, 0);
	// draw face 0
	glNormal3f(-0.2, 0.4, 0);
	glVertex3f(-0.2, 0.2, 0);
	glVertex3f(0.2, 0.4, 0);
	glVertex3f(0.2, 0.4, 0.01);
	glVertex3f(-0.2, 0.2, 0.01);
	// draw face 1
	glNormal3f(0.4, 0.0, 0);
	glVertex3f(0.2, 0.4, 0);
	glVertex3f(0.2, 0, 0);
	glVertex3f(0.2, 0, 0.01);
	glVertex3f(0.2, 0.4, 0.01);
	// draw face 2
	glNormal3f(-0.2, -0.4, 0);
	glVertex3f(0.2, 0, 0);
	glVertex3f(-0.2, 0.2, 0);
	glVertex3f(-0.2, 0.2, 0.01);
	glVertex3f(0.2, 0, 0.01);
	// draw face 3
	glNormal3f(0.0, 0.0, 0);
	glVertex3f(-0.2, 0.2, 0);
	glVertex3f(-0.2, 0.2, 0);
	glVertex3f(-0.2, 0.2, 0.01);
	glVertex3f(-0.2, 0.2, 0.01);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(0.2, 0.30000000000000004, 0.01);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0.01);
	glVertex3f(0.6000000000000001, 0.1, 0.01);
	glVertex3f(0.2, 0.1, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.2, 0.1, 0);
	glVertex3f(0.6000000000000001, 0.1, 0);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0);
	glVertex3f(0.2, 0.30000000000000004, 0);
	// draw face 0
	glNormal3f(0.0, 0.4000000000000001, 0);
	glVertex3f(0.2, 0.30000000000000004, 0);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0.01);
	glVertex3f(0.2, 0.30000000000000004, 0.01);
	// draw face 1
	glNormal3f(0.20000000000000004, 0.0, 0);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0);
	glVertex3f(0.6000000000000001, 0.1, 0);
	glVertex3f(0.6000000000000001, 0.1, 0.01);
	glVertex3f(0.6000000000000001, 0.30000000000000004, 0.01);
	// draw face 2
	glNormal3f(0.0, -0.4000000000000001, 0);
	glVertex3f(0.6000000000000001, 0.1, 0);
	glVertex3f(0.2, 0.1, 0);
	glVertex3f(0.2, 0.1, 0.01);
	glVertex3f(0.6000000000000001, 0.1, 0.01);
	// draw face 3
	glNormal3f(-0.20000000000000004, 0.0, 0);
	glVertex3f(0.2, 0.1, 0);
	glVertex3f(0.2, 0.30000000000000004, 0);
	glVertex3f(0.2, 0.30000000000000004, 0.01);
	glVertex3f(0.2, 0.1, 0.01);
	glEnd();
	glPopMatrix();
}

void train()
{		
	glPushMatrix();
	glTranslatef(0.0, 0.96, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.18, 0, 0.32);
	glVertex3f(-0.24, 0.12, 0.32);
	glVertex3f(0.24, 0.12, 0.32);
	glVertex3f(0.18, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.24, 0.12, 0);
	glVertex3f(-0.24, 0.12, 0);
	glVertex3f(-0.18, 0, 0);
	// draw face 0
	glNormal3f(-0.12, -0.06, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.24, 0.12, 0);
	glVertex3f(-0.24, 0.12, 0.32);
	glVertex3f(-0.18, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0.48, 0);
	glVertex3f(-0.24, 0.12, 0);
	glVertex3f(0.24, 0.12, 0);
	glVertex3f(0.24, 0.12, 0.32);
	glVertex3f(-0.24, 0.12, 0.32);
	// draw face 2
	glNormal3f(0.12, -0.06, 0);
	glVertex3f(0.24, 0.12, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.18, 0, 0.32);
	glVertex3f(0.24, 0.12, 0.32);
	// draw face 3
	glNormal3f(0, -0.36, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.18, 0, 0.32);
	glVertex3f(0.18, 0, 0.32);
	glEnd();
	glTranslatef(0, 0.12, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.24, 0, 0.32);
	glVertex3f(-0.24, 0.54, 0.32);
	glVertex3f(0.24, 0.54, 0.32);
	glVertex3f(0.24, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.24, 0, 0);
	glVertex3f(0.24, 0.54, 0);
	glVertex3f(-0.24, 0.54, 0);
	glVertex3f(-0.24, 0, 0);
	// draw face 0
	glNormal3f(-0.54, 0.0, 0);
	glVertex3f(-0.24, 0, 0);
	glVertex3f(-0.24, 0.54, 0);
	glVertex3f(-0.24, 0.54, 0.32);
	glVertex3f(-0.24, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0.48, 0);
	glVertex3f(-0.24, 0.54, 0);
	glVertex3f(0.24, 0.54, 0);
	glVertex3f(0.24, 0.54, 0.32);
	glVertex3f(-0.24, 0.54, 0.32);
	// draw face 2
	glNormal3f(0.54, 0.0, 0);
	glVertex3f(0.24, 0.54, 0);
	glVertex3f(0.24, 0, 0);
	glVertex3f(0.24, 0, 0.32);
	glVertex3f(0.24, 0.54, 0.32);
	// draw face 3
	glNormal3f(0, -0.48, 0);
	glVertex3f(0.24, 0, 0);
	glVertex3f(-0.24, 0, 0);
	glVertex3f(-0.24, 0, 0.32);
	glVertex3f(0.24, 0, 0.32);
	glEnd();
	glTranslatef(0, 0.06, 0.32);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.18, 0, 0.01);
	glVertex3f(-0.18, 0.12, 0.01);
	glVertex3f(-0.06, 0.12, 0.01);
	glVertex3f(-0.06, 0, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.06, 0, 0);
	glVertex3f(-0.06, 0.12, 0);
	glVertex3f(-0.18, 0.12, 0);
	glVertex3f(-0.18, 0, 0);
	// draw face 0
	glNormal3f(-0.12, 0.0, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.18, 0.12, 0);
	glVertex3f(-0.18, 0.12, 0.01);
	glVertex3f(-0.18, 0, 0.01);
	// draw face 1
	glNormal3f(0.0, 0.12, 0);
	glVertex3f(-0.18, 0.12, 0);
	glVertex3f(-0.06, 0.12, 0);
	glVertex3f(-0.06, 0.12, 0.01);
	glVertex3f(-0.18, 0.12, 0.01);
	// draw face 2
	glNormal3f(0.12, 0.0, 0);
	glVertex3f(-0.06, 0.12, 0);
	glVertex3f(-0.06, 0, 0);
	glVertex3f(-0.06, 0, 0.01);
	glVertex3f(-0.06, 0.12, 0.01);
	// draw face 3
	glNormal3f(0, -0.12, 0);
	glVertex3f(-0.06, 0, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.18, 0, 0.01);
	glVertex3f(-0.06, 0, 0.01);
	glEnd();
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(0.06, 0, 0.01);
	glVertex3f(0.06, 0.12, 0.01);
	glVertex3f(0.18, 0.12, 0.01);
	glVertex3f(0.18, 0, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.18, 0.12, 0);
	glVertex3f(0.06, 0.12, 0);
	glVertex3f(0.06, 0, 0);
	// draw face 0
	glNormal3f(-0.12, 0.0, 0);
	glVertex3f(0.06, 0, 0);
	glVertex3f(0.06, 0.12, 0);
	glVertex3f(0.06, 0.12, 0.01);
	glVertex3f(0.06, 0, 0.01);
	// draw face 1
	glNormal3f(0.0, 0.12, 0);
	glVertex3f(0.06, 0.12, 0);
	glVertex3f(0.18, 0.12, 0);
	glVertex3f(0.18, 0.12, 0.01);
	glVertex3f(0.06, 0.12, 0.01);
	// draw face 2
	glNormal3f(0.12, 0.0, 0);
	glVertex3f(0.18, 0.12, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.18, 0, 0.01);
	glVertex3f(0.18, 0.12, 0.01);
	// draw face 3
	glNormal3f(0, -0.12, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.06, 0, 0);
	glVertex3f(0.06, 0, 0.01);
	glVertex3f(0.18, 0, 0.01);
	glEnd();
	glTranslatef(0, 0.18, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.18, 0, 0.01);
	glVertex3f(-0.18, 0.18, 0.01);
	glVertex3f(0.18, 0.18, 0.01);
	glVertex3f(0.18, 0, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.18, 0.18, 0);
	glVertex3f(-0.18, 0.18, 0);
	glVertex3f(-0.18, 0, 0);
	// draw face 0
	glNormal3f(-0.18, 0.0, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.18, 0.18, 0);
	glVertex3f(-0.18, 0.18, 0.01);
	glVertex3f(-0.18, 0, 0.01);
	// draw face 1
	glNormal3f(0.0, 0.36, 0);
	glVertex3f(-0.18, 0.18, 0);
	glVertex3f(0.18, 0.18, 0);
	glVertex3f(0.18, 0.18, 0.01);
	glVertex3f(-0.18, 0.18, 0.01);
	// draw face 2
	glNormal3f(0.18, 0.0, 0);
	glVertex3f(0.18, 0.18, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(0.18, 0, 0.01);
	glVertex3f(0.18, 0.18, 0.01);
	// draw face 3
	glNormal3f(0, -0.36, 0);
	glVertex3f(0.18, 0, 0);
	glVertex3f(-0.18, 0, 0);
	glVertex3f(-0.18, 0, 0.01);
	glVertex3f(0.18, 0, 0.01);
	glEnd();
	glTranslatef(0, 0.24, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.06, 0, 0.01);
	glVertex3f(-0.06, 0.06, 0.01);
	glVertex3f(0.06, 0.06, 0.01);
	glVertex3f(0.06, 0, 0.01);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.06, 0, 0);
	glVertex3f(0.06, 0.06, 0);
	glVertex3f(-0.06, 0.06, 0);
	glVertex3f(-0.06, 0, 0);
	// draw face 0
	glNormal3f(-0.06, 0.0, 0);
	glVertex3f(-0.06, 0, 0);
	glVertex3f(-0.06, 0.06, 0);
	glVertex3f(-0.06, 0.06, 0.01);
	glVertex3f(-0.06, 0, 0.01);
	// draw face 1
	glNormal3f(0.0, 0.12, 0);
	glVertex3f(-0.06, 0.06, 0);
	glVertex3f(0.06, 0.06, 0);
	glVertex3f(0.06, 0.06, 0.01);
	glVertex3f(-0.06, 0.06, 0.01);
	// draw face 2
	glNormal3f(0.06, 0.0, 0);
	glVertex3f(0.06, 0.06, 0);
	glVertex3f(0.06, 0, 0);
	glVertex3f(0.06, 0, 0.01);
	glVertex3f(0.06, 0.06, 0.01);
	// draw face 3
	glNormal3f(0, -0.12, 0);
	glVertex3f(0.06, 0, 0);
	glVertex3f(-0.06, 0, 0);
	glVertex3f(-0.06, 0, 0.01);
	glVertex3f(0.06, 0, 0.01);
	glEnd();
	glTranslatef(0, 0.06, -0.32);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.24, 0, 0.32);
	glVertex3f(-0.18, 0.06, 0.32);
	glVertex3f(0.18, 0.06, 0.32);
	glVertex3f(0.24, 0, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(0.24, 0, 0);
	glVertex3f(0.18, 0.06, 0);
	glVertex3f(-0.18, 0.06, 0);
	glVertex3f(-0.24, 0, 0);
	// draw face 0
	glNormal3f(-0.06, 0.06, 0);
	glVertex3f(-0.24, 0, 0);
	glVertex3f(-0.18, 0.06, 0);
	glVertex3f(-0.18, 0.06, 0.32);
	glVertex3f(-0.24, 0, 0.32);
	// draw face 1
	glNormal3f(0.0, 0.36, 0);
	glVertex3f(-0.18, 0.06, 0);
	glVertex3f(0.18, 0.06, 0);
	glVertex3f(0.18, 0.06, 0.32);
	glVertex3f(-0.18, 0.06, 0.32);
	// draw face 2
	glNormal3f(0.06, 0.06, 0);
	glVertex3f(0.18, 0.06, 0);
	glVertex3f(0.24, 0, 0);
	glVertex3f(0.24, 0, 0.32);
	glVertex3f(0.18, 0.06, 0.32);
	// draw face 3
	glNormal3f(0, -0.48, 0);
	glVertex3f(0.24, 0, 0);
	glVertex3f(-0.24, 0, 0);
	glVertex3f(-0.24, 0, 0.32);
	glVertex3f(0.24, 0, 0.32);
	glEnd();
	glPopMatrix();
}

void jail()
{
	glPushMatrix();
	glColor3f(0.5, 0.5, 0.5);
	glTranslatef(-0.384, -0.384, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glTranslatef(0, 0.128, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glTranslatef(0, 0.128, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glTranslatef(0, 0.128, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glTranslatef(0, 0.128, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glTranslatef(0, 0.128, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	// draw face 0
	glNormal3f(0.0, 0.064, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, 0.032, 0.6);
	glVertex3f(-0.032, 0.032, 0.6);
	// draw face 1
	glNormal3f(0.064, 0.0, 0);
	glVertex3f(0.032, 0.032, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(0.032, -0.032, 0.6);
	glVertex3f(0.032, 0.032, 0.6);
	// draw face 2
	glNormal3f(0.0, -0.064, 0);
	glVertex3f(0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, -0.032, 0.6);
	glVertex3f(0.032, -0.032, 0.6);
	// draw face 3
	glNormal3f(-0.064, 0.0, 0);
	glVertex3f(-0.032, -0.032, 0);
	glVertex3f(-0.032, 0.032, 0);
	glVertex3f(-0.032, 0.032, 0.6);
	glVertex3f(-0.032, -0.032, 0.6);
	glEnd();
	glPopMatrix();
}

void jail_four()
{
	glPushMatrix();
		glTranslatef(-4.0, -4.0, 0.0);
		glTranslatef(0.32, 0.32, 0.0);
		jail();
		glRotatef(90, 0.0, 0.0, 1.0);
		jail();
		glRotatef(90, 0.0, 0.0, 1.0);
		jail();
		glRotatef(90, 0.0, 0.0, 1.0);
		jail();
		glTranslatef(0.0, 0.0, 0.6);
		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0, 0, 1);
		glVertex3f(-0.416, 0.416, 0.1);
		glVertex3f(0.416, 0.416, 0.1);
		glVertex3f(0.416, -0.416, 0.1);
		glVertex3f(-0.416, -0.416, 0.1);
		// draw back face
		glNormal3f(0, 0, -1);
		glVertex3f(-0.416, -0.416, 0);
		glVertex3f(0.416, -0.416, 0);
		glVertex3f(0.416, 0.416, 0);
		glVertex3f(-0.416, 0.416, 0);
		// draw face 0
		glNormal3f(0.0, 0.832, 0);
		glVertex3f(-0.416, 0.416, 0);
		glVertex3f(0.416, 0.416, 0);
		glVertex3f(0.416, 0.416, 0.1);
		glVertex3f(-0.416, 0.416, 0.1);
		// draw face 1
		glNormal3f(0.832, 0.0, 0);
		glVertex3f(0.416, 0.416, 0);
		glVertex3f(0.416, -0.416, 0);
		glVertex3f(0.416, -0.416, 0.1);
		glVertex3f(0.416, 0.416, 0.1);
		// draw face 2
		glNormal3f(0.0, -0.832, 0);
		glVertex3f(0.416, -0.416, 0);
		glVertex3f(-0.416, -0.416, 0);
		glVertex3f(-0.416, -0.416, 0.1);
		glVertex3f(0.416, -0.416, 0.1);
		// draw face 3
		glNormal3f(-0.832, 0.0, 0);
		glVertex3f(-0.416, -0.416, 0);
		glVertex3f(-0.416, 0.416, 0);
		glVertex3f(-0.416, 0.416, 0.1);
		glVertex3f(-0.416, -0.416, 0.1);
		glEnd();
	glPopMatrix();
}

void mario()
{ 
	glPushMatrix();
	glRotatef(90, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glTranslatef(-0.18, 0.0, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.737255, 0.560784, 0.560784);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPopMatrix();
}

void pikachu()
{
	glPushMatrix();
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(-0.16, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPopMatrix();
}

void chest()
{
	glPushMatrix();
	glTranslatef(0.0, 0.96, 0.0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glTranslatef(-0.30, 0.0, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.647059, 0.164706, 0.164706);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(1.0, 0.5, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	// draw front face
	glNormal3f(0, 0, 1);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	// draw back face
	glNormal3f(0, 0, -1);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	// draw face 0
	glNormal3f(0.0, 0.04, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, 0.02, 0.32);
	glVertex3f(-0.02, 0.02, 0.32);
	// draw face 1
	glNormal3f(0.04, 0.0, 0);
	glVertex3f(0.02, 0.02, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(0.02, -0.02, 0.32);
	glVertex3f(0.02, 0.02, 0.32);
	// draw face 2
	glNormal3f(0.0, -0.04, 0);
	glVertex3f(0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, -0.02, 0.32);
	glVertex3f(0.02, -0.02, 0.32);
	// draw face 3
	glNormal3f(-0.04, 0.0, 0);
	glVertex3f(-0.02, -0.02, 0);
	glVertex3f(-0.02, 0.02, 0);
	glVertex3f(-0.02, 0.02, 0.32);
	glVertex3f(-0.02, -0.02, 0.32);
	glEnd();
	glTranslatef(0.04, 0, 0.0);
	glPopMatrix();
	glTranslatef(0, 0.04, 0.0);
	glPopMatrix();
}
// search