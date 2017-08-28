#include <iostream>
#include <gl_core_4_3.h>
#include <glslshader.h>
#include <glutils.hpp>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Camera.h>
#include <vector>


// Function prototypes
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement( );


GLuint VAO,VAO2,VBO,VBO2,EBO,VBO3;
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera( glm::vec3( 0.0f, 1.0f, 5.0f ) );
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

#define NUMNODES 100

// Light attributes
glm::vec3 lightPos( 2.0f, 2.0f, 2.0f );

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

float sqr(float x)
{
    return x*x;
}

float f(float x, float z)
{
    float y;
    y= glm::exp(-sqr(x)-sqr(z));
    return y;
}

float dx(float x,float z)
{
    return -2.0f*x*f(x,z);
}

float dz(float x,float z)
{
    return -2.0f*z*f(x,z);
}

typedef glm::vec3 Point3f;
typedef glm::vec3 Normal3f;

typedef struct{
    std::vector<Point3f> vertices;
    std::vector<int> indices;
    std::vector<Normal3f> normals;
}Surface;

std::vector<Point3f> createGrid(float w,float d,int m, int n)
{
    float dx = w / float(n-1);
    float dz = d/ float(m-1);
    float halfDepth = 0.5f*d;
    float halfWidth = 0.5f*w;
    
    std::vector<Point3f> points;
    for (int i = 0; i<m; i++) {
        float z = halfDepth - i*dz;
        for (int j = 0; j<n; j++) {
            float x = -halfWidth + j*dx;
            Point3f point;
            point.x = x;
            point.z = z;
            point.y = f(x,z);
            points.push_back(point);
        }
    }
    return points;
}

std::vector<int> assignIndices(int m, int n)
{
    int numIndices = (m-1)*(n-1)*2*3;
    std::vector<int> myIndices;
    for (int i = 0; i < m-1; i++) {
        for (int j = 0; j<n-1; j++) {
            myIndices.push_back(i*n+j);
            myIndices.push_back(i*n+j+1);
            myIndices.push_back((i+1)*n+j);
            
            myIndices.push_back((i+1)*n+j);
            myIndices.push_back(i*n+j+1);
            myIndices.push_back((i+1)*n+j+1);
        }
    }
    return myIndices;
}

//implementation to use partial derivatives.
std::vector<Normal3f> assignNormals(std::vector<Point3f> points)
{
    std::vector<Normal3f> myNormals;
    for (int i = 0; i<points.size(); i++) {
        Normal3f normal;
        normal.x = -dx(points[i].x,points[i].z);
        normal.y = 1;
        normal.z = -dz(points[i].x, points[i].z);
        myNormals.push_back(normal);
    }
    return myNormals;
}

//works correctly by taking the cross of the vectors from the first point to the second and third based
//on the triangle indices
std::vector<Normal3f> assignNormals2(std::vector<Point3f> points,std::vector<int> indices)
{
    std::vector<Normal3f> myNormals(points.size());
    for (int i = 0; i<indices.size(); i+=3) {
        
        Normal3f normal;
        glm::vec3 one;
        glm::vec3 two;
        one = points[indices[i+1]]-points[indices[i]];
        two = points[indices[i+2]]-points[indices[i]];
        normal = glm::cross(one,two);
        myNormals[indices[i]] = normal;
        myNormals[indices[i+1]] = normal;
        myNormals[indices[i+2]] = normal;
    }
    return myNormals;
}




Surface createSurface(float w,float d,int m,int n)
{
    Surface mySurface;
    mySurface.vertices = createGrid(w, d, m, n);
    mySurface.indices = assignIndices(m,n);
    mySurface.normals = assignNormals(mySurface.vertices);
    //mySurface.normals = assignNormals2(mySurface.vertices,mySurface.indices);

    return mySurface;
}


int main(int argc, const char * argv[]) {
    // Init GLFW
    glfwInit( );
    
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "Surface Visualizer", nullptr, nullptr );
    
    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        
        return EXIT_FAILURE;
    }
    
    
    glfwMakeContextCurrent( window );
    
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    
    int numInit = ogl_LoadFunctions();
    GLUtils::dumpGLInfo();
    std::cout<<"Falied to initialized "<<numInit<<" functions\n";
    
    // Set the required callback functions
    glfwSetKeyCallback( window, KeyCallback );
    glfwSetCursorPosCallback( window, MouseCallback );
    
    // GLFW Options
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    
    // OpenGL options
    glEnable( GL_DEPTH_TEST );
    
    GLSLProgram lightingShader,lampShader;
    try {
        lightingShader.compileShader("Resources/shaders/light.vert");
        lightingShader.compileShader("Resources/shaders/light.frag");
        lightingShader.link();
        lampShader.compileShader("Resources/shaders/lamp.vert");
        lampShader.compileShader("Resources/shaders/lamp.frag");
        lampShader.link();
        
    } catch (GLSLProgramException &e) {
        std::cerr<<e.what()<<std::endl;
    }
    
    Surface mySurface = createSurface(5,5,NUMNODES,NUMNODES);
    float vertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        
        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1,&VAO2);
    glGenBuffers(1,&VBO3);
    glBindVertexArray(VAO2);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),(GLvoid*)0);
    glBindVertexArray(0);
    
    
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&VBO2);
    glGenBuffers(1,&EBO);
    
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,mySurface.vertices.size()*3*sizeof(float),mySurface.vertices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),(GLvoid*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER,mySurface.normals.size()*3*sizeof(float),mySurface.normals.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),(GLvoid*)0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,mySurface.indices.size()*sizeof(int),mySurface.indices.data(),GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    glm::mat4 model;
    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 1000.0f );
    
    
    
    while ( !glfwWindowShouldClose( window ) )
    {
        
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.008f, glm::vec3(0.0f,1.0f,0.0f));
        lightPos = glm::vec4(lightPos,1.0f)*rot;
        
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime( );
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents( );
        DoMovement( );
        
        
        // Clear the colorbuffer
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
       // Use cooresponding shader when setting uniforms/drawing objects
        lightingShader.use( );
        GLint objectColorLoc = glGetUniformLocation( lightingShader.getHandle(), "wireframe" );
        GLint lightColorLoc = glGetUniformLocation( lightingShader.getHandle(), "lightColor" );
        GLint lightPosLoc = glGetUniformLocation( lightingShader.getHandle(), "lightPos" );
        GLint viewPosLoc = glGetUniformLocation( lightingShader.getHandle(), "viewPos" );
        glUniform1i( objectColorLoc, 0 );
        glUniform3f( lightColorLoc, 1.0f, 1.0f, 1.0f );
        glUniform3f( lightPosLoc, lightPos.x, lightPos.y, lightPos.z );
        glUniform3f( viewPosLoc, camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
        
        // Create camera transformations
        glm::mat4 view;
        view = camera.GetViewMatrix( );
        
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation( lightingShader.getHandle(), "model" );
        GLint viewLoc = glGetUniformLocation( lightingShader.getHandle(), "view" );
        GLint projLoc = glGetUniformLocation( lightingShader.getHandle(), "projection" );
        
        // Pass the matrices to the shader
        glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
        glm::mat4 model;
        glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
        
        // Draw the container (using container's vertex attributes)
        glBindVertexArray( VAO );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
        
        glDrawElements(GL_TRIANGLES,((NUMNODES-1)*(NUMNODES-1)*6),GL_UNSIGNED_INT,NULL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glUniform3f( objectColorLoc, 0.0f, 0.0f, 0.0f );
        glUniform1i( objectColorLoc, 1 );
        glDrawElements(GL_TRIANGLES,((NUMNODES-1)*(NUMNODES-1)*6),GL_UNSIGNED_INT,NULL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        // Also draw the lamp object, again binding the appropriate shader
        lampShader.use( );
        // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
        modelLoc = glGetUniformLocation( lampShader.getHandle(), "model" );
        viewLoc  = glGetUniformLocation( lampShader.getHandle(), "view" );
        projLoc  = glGetUniformLocation( lampShader.getHandle(), "projection" );
        
        // Set matrices
        glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
        
        model = glm::mat4( );
        model = glm::translate( model, lightPos );
        model = glm::scale( model, glm::vec3( 0.1f ) ); // Make it a smaller cube
        glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
        
        // Draw the light object (using light's vertex attributes)
        glBindVertexArray( VAO2 );
        glDrawArrays( GL_TRIANGLES, 0, 36 );
        glBindVertexArray( 0 );
        
        // Swap the screen buffers
        glfwSwapBuffers( window );
    }
    
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate( );
    
    return 0;
}
// Moves/alters the camera positions based on user input
void DoMovement( )
{
    // Camera controls
    if ( keys[GLFW_KEY_W] || keys[GLFW_KEY_UP] )
    {
        camera.ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN] )
    {
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT] )
    {
        camera.ProcessKeyboard( LEFT, deltaTime );
    }
    
    if ( keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT] )
    {
        camera.ProcessKeyboard( RIGHT, deltaTime );
    }
    
    if ( keys[GLFW_KEY_Q] )
    {
        camera.ProcessKeyboard( UP, deltaTime );
    }
    
    if ( keys[GLFW_KEY_E] )
    {
        camera.ProcessKeyboard( DOWN, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if ( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if ( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

void MouseCallback( GLFWwindow *window, double xPos, double yPos )
{
    if ( firstMouse )
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement( xOffset, yOffset );
}
