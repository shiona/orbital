#define _GLIBCXX_USE_CXX11_ABI 0

//#define GL_GLEXT_PROTOTYPES
//#include <GL/glext.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/OpenGL.hpp>


const double PI = 3.141592654;

#define WIDTH (800)
#define HEIGHT (600)

const int HOR_SLICES = 36;
const int VER_SLICES = 56;

GLuint vao;
GLuint sphere_vao;
GLuint shader_programme;
GLuint view_handle;
GLuint texture_handle;

GLuint genSphere()
{

	sf::Image img_data;
	if(!img_data.loadFromFile("earth_tex.jpg")) {
		printf("Could not load texture 'earth_tex.jpg'\n");
	}
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLfloat rad = 1.0f;

	//GLuint vert_count = 2 + HOR_SLICES * VER_SLICES;
	GLuint vert_count = HOR_SLICES * (VER_SLICES+1) * 2;

	std::vector<GLfloat> vertArray(vert_count*3);
	std::vector<GLfloat> normArray(vert_count*3);
	std::vector<GLfloat> texArray(vert_count*2);

	for(int y = 0; y < HOR_SLICES; y++) {
		float height1 = cos(((float)y/(float)HOR_SLICES)*PI);
		float radius1 = sin(((float)y/(float)HOR_SLICES)*PI);
		float height2 = cos(((float)(y+1)/(float)HOR_SLICES)*PI);
		float radius2 = sin(((float)(y+1)/(float)HOR_SLICES)*PI);

		//printf("ring:\n");
		for(int s = 0; s <= VER_SLICES; s++) {
			float cylinderx = sin(2*PI * ((float)s/(float)VER_SLICES));
			float cylindery = cos(2*PI * ((float)s/(float)VER_SLICES));
			//printf("%f, %f, %f\n", height1, radius1 * cylinderx, radius1 * cylindery);
			//printf("%f, %f, %f\n", height2, radius2 * cylinderx, radius2 * cylindery);
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 0] = radius1 * cylinderx;
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 1] = height1;
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 2] = radius1 * cylindery;
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 3] = radius2 * cylinderx;
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 4] = height2;
			vertArray[y*(VER_SLICES+1)*6 + s*6 + 5] = radius2 * cylindery;

			texArray[y*(VER_SLICES+1)*4 + s*4 + 0] = (float)s/(float)VER_SLICES + 0.5f;
			//texArray[y*(VER_SLICES+1)*4 + s*4 + 1] = 0.5f - height1/2.0f;
			texArray[y*(VER_SLICES+1)*4 + s*4 + 1] = (float)y/(float)HOR_SLICES;
			texArray[y*(VER_SLICES+1)*4 + s*4 + 2] = (float)s/(float)VER_SLICES + 0.5f;
			//texArray[y*(VER_SLICES+1)*4 + s*4 + 3] = 0.5f - height2/2.0f;
			texArray[y*(VER_SLICES+1)*4 + s*4 + 3] = (float)(y+1)/(float)HOR_SLICES;
		}
		/*float cylinderx = sin(2*PI);
		float cylindery = cos(2*PI);
		printf("%f, %f, %f\n", height1, radius1 * cylinderx, radius1 * cylindery);
		printf("%f, %f, %f\n", height2, radius2 * cylinderx, radius2 * cylindery);
		*/
	}


	GLuint vert_vbo = 0;
	glGenBuffers(1, &vert_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vert_vbo);
	glBufferData (GL_ARRAY_BUFFER, 3*vert_count*sizeof(float), &vertArray[0], GL_STATIC_DRAW);

	GLuint tex_vbo = 1;
	glGenBuffers(1, &tex_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, tex_vbo);
	glBufferData (GL_ARRAY_BUFFER, 2*vert_count*sizeof(float), &texArray[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &sphere_vao);
	glBindVertexArray(sphere_vao);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	//glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/*
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	*/

	glBindVertexArray(0);

	return 0;
}

bool init()
{
	glewInit();

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	float points[] = {
		 0.0f,  0.5f,  0.0f,
		 0.5f, -0.5f,  0.0f,
		-0.5f, -0.5f,  0.0f
	};

	GLuint vbo = 0;
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), points, GL_STATIC_DRAW);

	vao = 0;
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	const char* vertex_shader =
		"#version 400\n"
		//"in vec3 vp;"
		"uniform mat4 proj_mat;"
		"uniform mat4 view_mat;"
		"layout(location = 0) in vec3 vp;"
		"layout(location = 1) in vec2 tex;"
		"out vec2 tex_coord;"
		"void main () {"
		"  tex_coord = tex;"
		"  gl_Position = proj_mat * view_mat * vec4 (vp, 1.0);"
		//"  gl_Position = mat4(1.0) * vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (MVP * vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 400\n"
		//"uniform mat4 MVP;"
		"in vec2 tex_coord;"
		//"out vec4 frag_colour;"
		"out vec3 frag_colour;"
		"uniform sampler2D tex_sampler;"
		"void main () {"
		//"  frag_colour = vec4 (tex_coordI.y, tex_coord.y, tex_coord.y, 1.0);"
		"  frag_colour = texture(tex_sampler, tex_coord).rgb;"
		"}";

	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL);
	glCompileShader (vs);
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL);
	glCompileShader (fs);

	GLint compileStatus;
	GLchar errorLog[512];
	glGetObjectParameterivARB(vs,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "VS Compilation error\n");
		glGetShaderInfoLog(vs, 512, NULL, errorLog);
		printf("%s\n", errorLog);
	}
	glGetObjectParameterivARB(fs,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "FS Compilation error\n");
		glGetShaderInfoLog(fs, 512, NULL, errorLog);
		printf("%s\n", errorLog);
	}

	shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glLinkProgram (shader_programme);

	glGetObjectParameterivARB(shader_programme,GL_OBJECT_LINK_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "Link error\n");
	}
	// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 proj_mat = glm::perspective(glm::radians(35.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	/*
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(-7,0.2,0.2), // Camera is at (4,3,3), in World Space
		glm::vec3(0,0,0), // and looks at the origin
		glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
	*/

	GLuint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
	view_handle = glGetUniformLocation(shader_programme, "view_mat");
	if(proj_handle == -1 || view_handle == -1) {
		printf("Illegal uniform:\n");
	}
	glUseProgram (shader_programme);
	glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);

	genSphere();
}

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 4;
	settings.minorVersion = 4;

	sf::Window window (sf::VideoMode(WIDTH, HEIGHT), "Orbital Challenge", sf::Style::Default, settings);
	//window.setVerticalSyncEnabled(true);

	init();

    bool running = true;

	GLfloat theta = 0.0f;
	GLfloat sigma = 0.0f;
    while (running)
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                // end the program
                running = false;
            }
            else if (event.type == sf::Event::Resized)
            {
                // adjust the viewport when the window is resized
                glViewport(0, 0, event.size.width, event.size.height);
            }
			else if (event.type == sf::Event::KeyPressed) {
				switch(event.key.code) {
					case sf::Keyboard::W:
						sigma += 0.04f;
						break;
					case sf::Keyboard::S:
						sigma -= 0.04f;
						break;
					case sf::Keyboard::D:
						theta += 0.04f;
						break;
					case sf::Keyboard::A:
						theta -= 0.04f;
						break;
					default:
						break;
				}
			}
        }

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram (shader_programme);

		glm::mat4 view_mat = glm::lookAt(
			glm::vec3(5*sin(theta),5*sin(sigma),5*cos(theta)), // Camera is at (4,3,3), in World Space
			glm::vec3(0,0,0), // and looks at the origin
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		glUniformMatrix4fv(view_handle, 1, GL_FALSE, &view_mat[0][0]);

		//glUseProgram (0);
		//glBindVertexArray (vao);
		// draw points 0-3 from the currently bound VAO with current in-use shader
		//glDrawArrays (GL_TRIANGLES, 0, 3);

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		//glBindTexture(texture_handle);
		glBindVertexArray (sphere_vao);
		for(int i = 0; i < HOR_SLICES; i++) { // For every horizontal slice..
			//int i = 2;
			glDrawArrays (GL_TRIANGLE_STRIP, i*(2*(VER_SLICES+1)), 2*(VER_SLICES+1));
		}

        // draw...

        // end the current frame (internally swaps the front and back buffers)
        window.display();
    }

    // release resources...

    return 0;
}

