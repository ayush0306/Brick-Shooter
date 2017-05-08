#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<time.h>
#include<stdlib.h>
#include<set>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

VAO *rectangle[100]  ;
VAO *cannon ;
VAO *bucket[2];
VAO *line[5] ; int nlines ; float mirrorx[5]; float mirrory[5];
VAO *mirror[5];
VAO *battery; VAO *nose; VAO *charge ;
float mirrorAng[5];
set<pair<pair<float,float> ,int> > rect;
int BlColour[100];

float BucShift[2];
int redStatus = 0; int greenStatus = 0;
float cannonShift=0; int cannonShiftStatus = 0 ;
float cannonAngle=0; int cannonRotStatus = 0 ;
float shootStatus = 0; int score = 0;
float fallRate = 0.03f ;
float canshoot=0 ; double lastShoot ;
int width = 600;
int height = 600; int keyright = 0; int keyleft = 0 ; int keyup = 0 ; int keydown = 0;
float maxCoord = 4 ; float xpan = 0; float ypan = 0;
int gameon = 1;

int blackhits = 0, wronghits = 0, collected[2]={0,0} ;

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

float minf(float a, float b)
{
	if(a<b)
		return a;
	return b;
}

float checkRange(float val, float low, float high)
{
	if(val>high)
		return high;
	if(val<low)
		return low;
	return val;
}

void createMirror (int index,float a1,float b1,float angleMir)
{
	glLineWidth(10);
	mirrorAng[index]=angleMir;
	mirrorx[index] = a1;
	mirrory[index] = b1;
	const GLfloat vertex_buffer_data [] = {
		a1,b1,0, // vertex 0
		a1+1.5*cosf(angleMir*M_PI/180.0f),b1+1.5*sinf(angleMir*M_PI/180.0f),0 // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0,0,0, // color 0
		0,0,0, // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	mirror[index] = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createLine (int index,float a1,float b1,float a2,float b2)
{
	glLineWidth(10); nlines++;
	const GLfloat vertex_buffer_data [] = {
		a1-0.02,b1-0.02,0, // vertex 1
		a2-0.02,b2-0.02,0, // vertex 2
		a2+0.02,b2+0.02,0,//vertex3

		a2+0.02,b2+0.02,0,//vertex3
		a1+0.02,b1+0.02,0,//vertex4
		a1-0.02,b1-0.02,0//vertex1
	};

	const GLfloat color_buffer_data [] = {
		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	line[index] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

int updatable(float x1, float x2, float xstart, int x4)
{
	if(x4==1)
		if(x1<xstart)
			return 0;
	if(x4==-1)
		if(x1>xstart)
			return 0;
	if(abs(x1-xstart)<abs(x2-xstart))
		return 1;
	return 0;
}

int find_mirror(float *xbound, float *ybound, float xstart, float ystart, float slope,int xinc,int premirr)
{
	int toret = 0 ;
	for(int i=0;i<3;i++)
	{
		if(i!=premirr)
		{
			float mirrorSlope = tanf(mirrorAng[i]*M_PI/180.0f);
			float xinter = ( slope*xstart - mirrorSlope*mirrorx[i] - ystart + mirrory[i] ) / (slope - mirrorSlope);
			float yinter = slope*(xinter - xstart) + ystart ;
			// printf("intersection point with mirror are %f and %f\n",xinter,yinter);
			if(updatable(xinter,*xbound,xstart,xinc))
			{
				// printf("Half check\n");
				if(xinter > mirrorx[i] && xinter < mirrorx[i]+1.5*cosf(mirrorAng[i]*M_PI/180.0f) && yinter > mirrory[i] && yinter < mirrory[i] + 1.5*sinf(mirrorAng[i]*M_PI/180.0f))
				{
					// printf("updating\n");
					*xbound = xinter;
					*ybound = yinter;
					toret = i+1;
				}
			}
		}
	}
	return toret;
}

void find_boundary(float *xbound, float *ybound, float xstart, float ystart, float slope,int xinc)
{
	*xbound = 4*xinc;
	*ybound = (*xbound - xstart)*slope + ystart ;
	return;
}

void shootLaser(int lineInd, float xstart, float ystart,float angle,int xinc,int mirrornum){                   //more tells if the blocks should have a higer x-cord or not
	float finalx=0.0,finaly=0.0 ;
	float slope = tanf(angle*M_PI/180.0f);
	if(lineInd==0)
	{
		xstart = -4 + 0.5*cosf(angle*M_PI/180.0f);
		ystart = ystart + 0.5*sinf(angle*M_PI/180.0f);
	}
	// printf("Starting points are %f %f\n",xstart,ystart);
	find_boundary(&finalx,&finaly,xstart,ystart,slope,xinc);
	int ifmirror = find_mirror(&finalx,&finaly,xstart,ystart,slope,xinc,mirrornum);
	// printf("Boundary points are %f %f\n",finalx,finaly);
	set<pair<pair<float,float> ,int> >::iterator it;
	set<pair<pair<float,float> ,int> >::iterator removeindex = rect.end();
	float x1,y1,tmp;
	int toadd = 0;
	for(it=rect.begin();it!=rect.end();it++)
	{
		x1 = ((*it).first).first;
		y1 = ((*it).first).second;
		int blInd = (*it).second;
		tmp = slope*(x1-xstart) + ystart ;
		if(abs(y1-tmp)<=0.20){
			if(updatable(x1,finalx,xstart,xinc))
			{
				finalx=x1;
				finaly = tmp ;
				ifmirror=0;
				removeindex = it;
				if(BlColour[blInd]>=1)
					toadd = 20;
				else
					toadd = -10;
			}
		}
	}

	if(toadd == 20)
		blackhits++;
	else if(toadd == -10)
		wronghits++;
	score += toadd*100*fallRate;
	printf("score is %d\n",score);
	if(removeindex!=rect.end())
		rect.erase(removeindex);
	// printf("End points before mirrors are %f %f\n",finalx,finaly);
	// int ifmirror = find_mirror(&finalx,&finaly,xstart,ystart,slope,xinc,mirrornum);
	// printf("Is there a mirror : %d\n",ifmirror);
	if(ifmirror > 0)
	{
		// printf("End points after mirrors are %f %f\n",finalx,finaly);
		createLine(lineInd,xstart,ystart,finalx,finaly);
		float newangle = 2*mirrorAng[ifmirror-1] - angle ;
		// printf("angle of line is %f\n",newangle);
		if(cosf(newangle*M_PI/180.0f) >= 0)
			return shootLaser(lineInd+1,finalx,finaly,2*mirrorAng[ifmirror-1] - angle ,1,ifmirror-1);
		// printf("Decreasing x\n");
		return shootLaser(lineInd+1,finalx,finaly,2*mirrorAng[ifmirror-1] - angle,-1,ifmirror-1);
	}
	createLine(lineInd,xstart,ystart,finalx,finaly);
	shootStatus = 1;
	lastShoot = glfwGetTime();
	return;
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_SPACE:
				shootStatus = 0;
				nlines = 0;
				break;
			case GLFW_KEY_A:
				cannonRotStatus = 0;
				break;
			case GLFW_KEY_D:
				cannonRotStatus = 0;
				break;
			case GLFW_KEY_S:
				cannonShiftStatus = 0;
				break;
			case GLFW_KEY_F:
				cannonShiftStatus = 0;
				break;
			case GLFW_KEY_N:
				fallRate += 0.005 ;
				fallRate = checkRange(fallRate,0.01,0.05);
				break;
			case GLFW_KEY_M:
				fallRate -= 0.005 ;
				fallRate = checkRange(fallRate,0.01,0.05);
				break;
			case GLFW_KEY_RIGHT: {
						     redStatus=0;
						     greenStatus=0;
						     keyright = 0;
					     }
					     break;
			case GLFW_KEY_LEFT: {
						    redStatus=0;
						    greenStatus=0;
						    keyleft = 0;
					    }
					    break;
			case GLFW_KEY_UP: {
						  keyup = 0;
					  }
					  break;
			case GLFW_KEY_DOWN: {
						    keydown = 0;
					    }
					    break;
			default:
					    break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_SPACE:
				if(canshoot>=1)
					shootLaser(0,-3.5,cannonShift,cannonAngle,1,5);
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_A:
				cannonRotStatus = 1 ;
				break;
			case GLFW_KEY_D:
				cannonRotStatus = -1;
				break;
			case GLFW_KEY_S:
				cannonShiftStatus = 1;
				break;
			case GLFW_KEY_F:
				cannonShiftStatus = -1;
				break;
			case GLFW_KEY_RIGHT: {
						     keyright = 1;
						     if((glfwGetKey(window,GLFW_KEY_RIGHT_ALT)||glfwGetKey(window,GLFW_KEY_LEFT_ALT)))
							     redStatus=1;
						     if((glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL)||glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)))
							     greenStatus=1;
					     }
					     break;
			case GLFW_KEY_LEFT: {
						    keyleft = 1;
						    if((glfwGetKey(window,GLFW_KEY_RIGHT_ALT)||glfwGetKey(window,GLFW_KEY_LEFT_ALT)))
							    redStatus=-1;
						    if((glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL)||glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)))
							    greenStatus=-1;
					    }
					    break;
			case GLFW_KEY_UP: {
						  keyup = 1;
					  }
					  break;
			case GLFW_KEY_DOWN: {
						    keydown = 1 ;
					    }
			default:
					    break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			gameon = 0;
			break;
		default:
			break;
	}
}

int mouse_press=0 ; int mouse_right_click = 0; int working = 0 ; float xpre,ypre;

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE){
				mouse_press = 0;
				working = 0;
			}
			else if (action == GLFW_PRESS) {
				mouse_press = 1;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if(action == GLFW_RELEASE){
				mouse_right_click = 0;
				working = 0;
			}
			else if(action == GLFW_PRESS)
				mouse_right_click = 1;
		default:
			break;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset==-1){
		maxCoord+=0.05;
		maxCoord = checkRange(maxCoord,1,4);
	}
	else if(yoffset==1){
		maxCoord-=0.05;
		maxCoord = checkRange(maxCoord,1,4);
	}
	return ;
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int wid, int ht)
{
	int fbwidth=wid, fbheight=ht;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	width = wid; height = ht;

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-maxCoord+xpan, maxCoord+xpan,-maxCoord+ypan, maxCoord+ypan,0.1f, 500.0f);
}


// Creates the rectangle object used in this sample code
void createRectangle (int index,int colour)
{
	int forblack = rand()%3;
	forblack=(1-forblack%2);
	forblack = 1;
	// GL3 accepts only Triangles. Quads are not supported
	const GLfloat vertex_buffer_data [] = {
		-0.1,-0.1,0, // vertex 1
		0.1,-0.1,0, // vertex 2
		0.1, 0.1,0, // vertex 3

		0.1, 0.1,0, // vertex 3
		-0.1, 0.1,0, // vertex 4
		-0.1,-0.1,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		(1-colour)*forblack,colour*forblack,0, // color 1
		(1-colour)*forblack,colour*forblack,0,  // color 2
		(1-colour)*forblack,colour*forblack,0,  // color 3

		(1-colour)*forblack,colour*forblack,0,  // color 3
		(1-colour)*forblack,colour*forblack,0,  // color 4
		(1-colour)*forblack,colour*forblack,0,  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	BlColour[index]=colour+2*(1-forblack);
	rectangle[index] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannon ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,-0.2,0, // vertex 1
		0,0.2,0, // vertex 2
		0.5, 0.1,0, // vertex 3

		0.5, 0.1,0, // vertex 3
		0.5, -0.1,0, // vertex 4
		0,-0.2,0 // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 2
		0,0,0, // color 3

		0,0,0, // color 3
		0,0,0, // color 4
		0,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRedBucket (int index)
{
	// printf("Creating bucket\n");
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-0.3,-4.0,0, // vertex 1
		0.3,-4.0,0, // vertex 2
		0.5, -3.5,0, // vertex 3

		0.5, -3.5,0, // vertex 3
		-0.5, -3.5,0, // vertex 4
		-0.3,-4.0,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		1,0,0, // color 2
		1,0,0, // color 3

		1,0,0, // color 3
		1,0,0, // color 4
		1,0,0  // color 1
	};
	bucket[0] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	// printf("Created bucket\n");
}

void createGreenBucket (int index)
{
	// printf("Creating bucket\n");
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-0.3,-4.0,0, // vertex 1
		0.3,-4.0,0, // vertex 2
		0.5, -3.5,0, // vertex 3

		0.5, -3.5,0, // vertex 3
		-0.5, -3.5,0, // vertex 4
		-0.3,-4.0,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,1,0, // color 1
		0,1,0, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0,1,0, // color 4
		0,1,0  // color 1
	};
	bucket[1] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	// printf("Created bucket\n");
}

void createBattery()
{
	static const GLfloat vertex_buffer_data [] = {
		-3.7,3.7,0, // vertex 1
		-3.0,3.7,0, // vertex 2
		-3.0, 3.2,0, // vertex 3

		-3.0, 3.2,0, // vertex 3
		-3.7,3.2,0, // vertex 4
		-3.7,3.7,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 1
		0,0,0, // color 1

		0,0,0, // color 1
		0,0,0, // color 1
		0,0,0 // color 1
	};
	battery = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createNose()
{
	static const GLfloat vertex_buffer_data [] = {
		-3.0,3.55,0, // vertex 1
		-2.9,3.55,0, // vertex 2
		-2.9,3.40,0, //vertex 3

		-2.9,3.40,0, // vertex 3
		-3.0,3.40,0, // vertex 4
		-3.0,3.55,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 1
		0,0,0, // color 1

		0,0,0, // color 1
		0,0,0, // color 1
		0,0,0 // color 1
	};
	nose = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCharge(float length)
{
	const GLfloat vertex_buffer_data [] = {
		-3.6,3.6,0, // vertex 1
		-3.6+length*0.5,3.6,0, // vertex 2
		-3.6+length*0.5,3.3,0, // vertex 3

		-3.6+length*0.5,3.3,0, // vertex 3
		-3.6,3.3,0, // vertex 4
		-3.6,3.6,0  // vertex 1
	};

	const GLfloat color_buffer_data [] = {
		0,1,0, // color 1
		0,1,0, // color 1
		0,1,0, // color 1

		0,1,0, // color 1
		0,1,0, // color 1
		0,1,0 // color 1
	};
	charge = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

int checkBucket(float xcord,int colour)
{
	if(xcord<0.5+BucShift[colour] && xcord>BucShift[colour]-0.5)
		return 1;
	return 0;
	// printf("%d\n",score);
}

float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (int count)
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	for(int i=0;i<3;i++)
	{
		Matrices.model = glm::mat4(1.0f);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(mirror[i]);
	}

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(battery);

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(nose);

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(charge);

	for(int i=0;i<2;i++)
	{
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBucket = glm::translate (glm::vec3(BucShift[i],0.0f, 0.0f));
		Matrices.model *= translateBucket ;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(bucket[i]);
	}

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 rotateCannon = glm::rotate((float)(cannonAngle*M_PI/180.0f), glm::vec3(0,0,1));
	glm::mat4 translateCannon = glm::translate (glm::vec3(-4.0f,cannonShift, 0.0f)); // glTranslatef
	// rotate about vector (1,0,0)
	glm::mat4 cannonTransform = translateCannon*rotateCannon;
	Matrices.model *= cannonTransform;
	MVP = VP * Matrices.model; // MVP = p * V * M
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(cannon);

	if(shootStatus>=0)
	{
		for(int i=0;i<nlines;i++)
		{
			Matrices.model = glm::mat4(1.0f);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(line[i]);
		}
		shootStatus -= 0.5 ;
		if(shootStatus <= 0)
			nlines = 0;
	}
	// Load identity to model matrix

	/* Render your scene */
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	set<pair<pair<float, float> ,int> >::iterator it ;
	for(it = rect.begin(); it != rect.end(); it++)
	{
		float f1 = ((*it).first).first;
		float f2 = ((*it).first).second;
		int ind = (*it).second;
		if(f2<=-3.4)
		{
			int tmp = 0;
			// printf("reached\n");
			if(BlColour[ind]>1)
			{
				tmp = checkBucket(f1,0) + checkBucket(f1,1);
				if(tmp>0)
					gameon = 0;
			}
			else{
				if(checkBucket(f1,BlColour[ind]) == 1){
					score += 1000*fallRate;
					collected[BlColour[ind]]++;
				}
			}
			rect.erase(it);
			printf("score is %d\n",score);
		}
		else
		{
			//printf("Drawing for %d with ycord as %f\n",ind,f2);
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 translateRectangle = glm::translate (glm::vec3(f1,f2, 0));        // glTranslatef
			Matrices.model *= (translateRectangle);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// draw3DObject draws the VAO given to it using current MVP matrix
			draw3DObject(rectangle[ind]);
		}
	}

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, scroll_callback);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models

	createCannon();
	createRedBucket(0);
	createGreenBucket(1);
	createBattery();
	createNose();
	BucShift[0]-=1;
	BucShift[1]+= 1;
	createMirror(0,-2,0,rand()%89+1);
	//printf("xstart is %f ystart is %f and slope is %f",mirrorx[0],mirrory[0],mirrorAng[0]);
	createMirror(1,2.5,-2,rand()%89+1) ;
	createMirror(2,-0.5,-3,rand()%89+1);
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int findObject(float xcord,float ycord)
{
	if(ycord<-3.6){
		if(checkBucket(xcord,0))
			return 1;
		if(checkBucket(xcord,1))
			return 2;
	}
	if(xcord<-3.4){
		if(abs(ycord - cannonShift ) <= 0.5)
			return 3;
	}
	return 0;
}

void moveObject(int obnumber,float xcord,float ycord)
{
	//printf("Moving object %d at coord %f %f\n",obnumber,xcord,ycord);
	if(obnumber==1 || obnumber==2 ){
		BucShift[obnumber-1]=(xcord);
		BucShift[obnumber-1] = checkRange(BucShift[obnumber-1],-4,4);
	}
	if(obnumber==3){
		cannonShift=(ycord);
		cannonShift = checkRange(cannonShift,-3.4,4);
	}
	if(obnumber==0){
		cannonAngle = atanf((ycord-cannonShift)/(xcord+4))*180.0f/M_PI;
	}
	return;
}

void pushDown(){
	set<pair<pair<float, float> ,int> >::iterator it ;
	for(it = rect.begin(); it != rect.end(); it++){
		float tmp1 = ((*it).first).first ;
		float tmp2 = (((*it).first).second-fallRate);
		int tmp3 = (*it).second;
		rect.erase(it);
		rect.insert(make_pair(make_pair(tmp1,tmp2),tmp3));
	}
}

void makeChanges()
{
	if(cannonShiftStatus != 0)
	{
		cannonShift += ((float)cannonShiftStatus)*0.02 ;
		cannonShift = checkRange(cannonShift,-3.4,4);
	}
	if(cannonRotStatus != 0)
	{
		cannonAngle += ((float)cannonRotStatus)*0.2;
		cannonAngle = checkRange(cannonAngle,-90,90);
	}
	if(redStatus!=0)
	{
		BucShift[0] += ((float)redStatus)*0.02;
		BucShift[0] = checkRange(BucShift[0],-4,4);
	}
	if(greenStatus!=0)
	{
		BucShift[1] += ((float)greenStatus)*0.02;
		BucShift[1] = checkRange(BucShift[1],-4,4);
	}

	if(mouse_right_click && keyright){
		xpan+=0.05;
		if(xpan + maxCoord > 4)
			xpan -= 0.05;
	}
	if(mouse_right_click && keyleft){
		xpan-=0.05;
		if(xpan - maxCoord < -4)
			xpan += 0.05;
	}
	if(mouse_right_click && keyup){
		ypan+=0.05;
		if(ypan + maxCoord > 4)
			ypan -= 0.05;
	}
	if(mouse_right_click && keydown){
		ypan-=0.05;
		if(ypan - maxCoord < -4)
			ypan += 0.05;
	}
	if(keydown && !mouse_right_click){
		maxCoord+=0.05;
		maxCoord = checkRange(maxCoord,1,4);
	}

	if(keyup && !mouse_right_click){
		maxCoord-=0.05;
		maxCoord = checkRange(maxCoord,1,4);
	}

}

int main (int argc, char** argv)
{
	nlines = 0;
	srand ( time(NULL) );
	int count_rectangles = 0;
	int objSelect = -1;
	GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);
	double newRec_time = glfwGetTime(), current_time,fallRec_time = glfwGetTime() ;
	while (!glfwWindowShouldClose(window) && gameon) {
		current_time = glfwGetTime();
		canshoot = minf((float)(current_time - lastShoot),1.0f);
		createCharge(canshoot);
		if ((current_time - fallRec_time) >= 0.01) {
				pushDown();
				fallRec_time = current_time ;
		}
		double tmpx,tmpy;
		float mouse_x,mouse_y;
		if(mouse_press==1)
		{
			// printf("press detected\n");
			glfwGetCursorPos(window,&tmpx,&tmpy);
			// printf("tmox and tmpy %f %f\n",tmpx,tmpy);
			mouse_x = ((float)tmpx - (float)width/2.0f )*8.0f/(float)width;
			mouse_y = ((float)height/2.0f - (float)tmpy)*8.0f/(float)height;
			// printf("Cursor at %f %f\n",mouse_x,mouse_y);
			if(working==0){
				objSelect = findObject(mouse_x,mouse_y);
				// printf("Selected the following obj : %d\n",objSelect);
				working = 1;
			}
			else
				moveObject(objSelect,mouse_x,mouse_y);
		}
		makeChanges();
		reshapeWindow (window, width, height);
		draw(count_rectangles);
		glfwSwapBuffers(window);
		glfwPollEvents();
		if ((current_time - newRec_time) >= 0.02/fallRate ) { // atleast 0.5s elapsed since last frame
			createRectangle (count_rectangles%100,rand()%2);
			float xshift = ((float)(400 - (rand() % 700)))/100;
			float yshift = 3.9;
			rect.insert(make_pair(make_pair(xshift,yshift),count_rectangles%100));
			count_rectangles++;
			newRec_time = current_time;
		}
	}
	printf("******************GAME OVER**************************\n");
	printf("Final Score : %d\nTotal Red Bricks collected : %d\nTotal Green Bricks collected : %d\nNo. of shots at black bricks : %d\nNo. of miss targets : %d\n",score,collected[0],collected[1],blackhits,wronghits);
	while(glfwGetTime()-newRec_time < 2);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
}
