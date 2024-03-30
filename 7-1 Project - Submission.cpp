/*------------------------------
Author: Christian Henshaw
Organization: SNHU
Version: 1.0
------------------------------*/


#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions


// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshes.h" // Meshes class
#include "camera.h" // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Christian Henshaw"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    Meshes meshes;

    // Texture id
    GLuint gTextureIdBottomCylinderLiquid;
    GLuint gTextureIdTopCylinderRibbed;
    GLuint gTextureIdCone;
    GLuint gTextureIdPlane;
    GLuint gTextureIdSphere;
    GLuint gTextureIdCubeCards;
    GLuint gTextureIdCoaster;
    bool gIsFruitOn = true;

    // Shader program
    GLuint gProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 1.5f, 10.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Utilized for Perspective/Orthographic changes
    bool perspectiveOrtho = true;

    // Light color, position and scale for overhead light (yellowish-white color)
    glm::vec3 gLightColor(0.90196f, 0.84313f, 0.76863f);
    glm::vec3 gLightPosition(-0.75f, 7.0f, -2.0f);
    glm::vec3 gLightScale(0.1f);

    // Light color, position and scale for window light (white color)
    glm::vec3 gWindowLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gWindowLightPosition(10.0f, 3.0f, -3.25f);
    glm::vec3 gWindowLightScale(0.1f);
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


// Vertex Shader Source Code
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec3 normal; // Normal data from Vertex Attrib Pointer 1
layout(location = 2) in vec2 textureCoordinate; // Texture data from Vertex Attrib Pointer 2

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate; // variable to transfer texture data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties

    vertexTextureCoordinate = textureCoordinate; // references incoming texture data
}
);


// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate; // Variable to hold incoming texture data from vertex shader

out vec4 fragmentColor;

uniform vec3 lightColor; // Uniform variables for light color
uniform vec3 lightPos; // Uniform variables for light position
uniform vec3 windowLightColor; // Uniform variables for window light color
uniform vec3 windowLightPos; // Uniform variables for window light position

uniform vec3 viewPosition; // Uniform variables for camera/view position

uniform sampler2D uTextureBase;
uniform sampler2D uTextureExtra;
uniform vec2 uvScale;
uniform bool multipleTextures;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.8f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    float windowAmbientStrength = 0.3f; // Set ambient or global lighting strength for key light
    vec3 windowAmbient = windowAmbientStrength * windowLightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    vec3 windowLightDirection = normalize(windowLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float windowImpact = max(dot(norm, windowLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 windowDiffuse = windowImpact * windowLightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.35f; // Set specular light strength
    float highlightSize = 8.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;
    vec3 windowSpecular = specularIntensity * specularComponent * windowLightColor;

    // Calculate attenuation for overhead light (distance of 100)
    float distance = length(lightPos - vertexFragmentPos);
    float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTextureBase, vertexTextureCoordinate * uvScale);
    if (multipleTextures)
    {
        vec4 extraTexture = texture(uTextureExtra, vertexTextureCoordinate * uvScale);
        if (extraTexture.a != 0.0)
            textureColor = extraTexture;
    }

    // Calculate phong result
    vec3 phong = (ambient + windowAmbient + diffuse + windowDiffuse + specular + windowSpecular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    meshes.CreateMeshes();

    // Verify the shader program can be created
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    //--------------------------------------------------
    // Load textures
    const char* texFilename = "bottomcylinderliquid3.jpg";
    if (!UCreateTexture(texFilename, gTextureIdBottomCylinderLiquid))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "topcylinderribbed.jpg";
    if (!UCreateTexture(texFilename, gTextureIdTopCylinderRibbed))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "cone.jpg";
    if (!UCreateTexture(texFilename, gTextureIdCone))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "plane.jpg";
    if (!UCreateTexture(texFilename, gTextureIdPlane))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "tennisball.jpg";
    if (!UCreateTexture(texFilename, gTextureIdSphere))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "playingcards.png";
    if (!UCreateTexture(texFilename, gTextureIdCubeCards))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "coaster2.jpg";
    if (!UCreateTexture(texFilename, gTextureIdCoaster))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //--------------------------------------------------

    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureBase"), 0);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureExtra"), 1);

    // Sets the background color of the window to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop until exit key is used
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        UProcessInput(gWindow);

        // Render each frame continuously
        URender();

        // Check for user inputs of exit keys
        glfwPollEvents();
    }

    // Release mesh data
    meshes.DestroyMeshes();

    // Release texture
    UDestroyTexture(gTextureIdBottomCylinderLiquid);
    UDestroyTexture(gTextureIdTopCylinderRibbed);
    UDestroyTexture(gTextureIdCone);
    UDestroyTexture(gTextureIdPlane);
    UDestroyTexture(gTextureIdSphere);
    UDestroyTexture(gTextureIdCubeCards);
    UDestroyTexture(gTextureIdCoaster);

    // Release shader program resources
    UDestroyShaderProgram(gProgramId);

    // Terminates the program
    exit(EXIT_SUCCESS);
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW: verifying window can be created
    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    // key to close window - escape
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // key to move camera forward - W
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    // key to move camera backward - S
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    // key to move camera left - A
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    // key to move camera right - D
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    // key to move camera up - Q
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    // key to move camera down - E
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    // key to change between perspective and orthographic - P
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        // change to perspective
        perspectiveOrtho = true;
    // change to true so perspective runs
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        // change to orthographic
        perspectiveOrtho = false;

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !gIsFruitOn)
        gIsFruitOn = true;
    else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && gIsFruitOn)
        gIsFruitOn = false;
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render frames
void URender()
{
    glm::mat4 scale;
    glm::mat4 rotation;
    glm::mat4 translation;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    GLint modelLoc;
    GLint viewLoc;
    GLint projLoc;

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Obtain the camera matrix
    view = gCamera.GetViewMatrix();

    if (perspectiveOrtho == true) {
        // Creates a perspective projection
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the shader program for the cub color, light color, light position, and camera position
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint windowLightColorLoc = glGetUniformLocation(gProgramId, "windowLightColor");
    GLint windowLightPositionLoc = glGetUniformLocation(gProgramId, "windowLightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the shader program's corresponding uniforms
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(windowLightColorLoc, gWindowLightColor.r, gWindowLightColor.g, gWindowLightColor.b);
    glUniform3f(windowLightPositionLoc, gWindowLightPosition.x, gWindowLightPosition.y, gWindowLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCylinderMesh.vao);

    // Scales the cylinder
    scale = glm::scale(glm::vec3(0.85f, 2.5f, 0.85f));
    // Rotates cylinder one full time
    rotation = glm::rotate(3.1415f, glm::vec3(1.0f, 0.0f, 0.0f));
    // Place cylinder
    translation = glm::translate(glm::vec3(-0.75f, 0.501f, -5.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Tile the textures
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(glm::vec2(0.80f, 1.0f)));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBottomCylinderLiquid);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCylinderMesh.vao);

    // Scales the cylinder top
    scale = glm::scale(glm::vec3(0.85f, 0.75f, 0.85f));
    // Rotates cylinder top one full time
    rotation = glm::rotate(3.1415f, glm::vec3(1.0f, 0.0f, 0.0f));
    // Place cylinder top
    translation = glm::translate(glm::vec3(-0.75f, 1.25f, -5.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTopCylinderRibbed);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gConeMesh.vao);

    // Scales the cone
    scale = glm::scale(glm::vec3(0.85f, 0.5f, 0.85f));
    // Rotates cone half a rotation
    rotation = glm::rotate(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    // Place cone
    translation = glm::translate(glm::vec3(-0.75f, 1.25f, -5.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCone);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPlaneMesh.vao);

    // Scales the plane
    scale = glm::scale(glm::vec3(2.5f, 1.0f, 2.5f));
    // Rotates plane one full time
    rotation = glm::rotate(3.1415f, glm::vec3(1.0f, 0.0f, 0.0f));
    // Place plane in the middle of the screen
    translation = glm::translate(glm::vec3(0.0f, 3.0f, -3.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Tile the texture
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(glm::vec2(1.0f, 1.2f)));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPlane);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gSphereMesh.vao);

    // Scales the sphere
    scale = glm::scale(glm::vec3(1.01f, 1.1f, 1.1f));
    // Rotates sphere one full time
    rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    // Place sphere
    translation = glm::translate(glm::vec3(2.25f, -0.9f, -3.25f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdSphere);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCubeMesh.vao);

    // Scales the cube (playing cards)
    scale = glm::scale(glm::vec3(3.25f, 0.75f, 2.1f));
    // Rotates cube (playing cards) half a rotation
    rotation = glm::rotate(1.5707963f, glm::vec3(0.0f, 1.0f, 0.0f));
    // Place cube (playing cards)
    translation = glm::translate(glm::vec3(-3.25f, -1.615f, -1.75f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Tile the texture
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(glm::vec2(1.0f, 1.0f)));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCubeCards);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gCubeMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // 
    //------------------------------------------------------------------------------------
        // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gHexagonMesh.vao);

    // Scales the hexagon
    scale = glm::scale(glm::vec3(0.4f, 0.6f, 0.4f));
    // Rotates hexagon
    rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    // Place hexagon
    translation = glm::translate(glm::vec3(1.5f, -1.95f, 1.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdCoaster);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gHexagonMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    //------------------------------------------------------------------------------------
    // glfw: swap buffers and poll IO events
    // Flips the the back buffer with the front buffer every frame.
    glfwSwapBuffers(gWindow);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId);
    // check for shader compile errors and report if errors exist
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors and report if errors exist
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors and report if errors exist
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    // Delete shader programs to unallocate resources
    glDeleteProgram(programId);
}