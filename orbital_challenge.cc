#ifdef __MINGW32__
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#define GLM_ENABLE_EXPERIMENTAL
//#define GL_GLEXT_PROTOTYPES
//#include <GL/glext.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>
#include <fstream>

#include "models.hh"


#define WIDTH (1200)
#define HEIGHT (1000)

GLuint shader_programme;
GLint view_handle;
GLint model_handle;

bool init()
{
	glewInit();

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.01f,0.05f,0.1,1.0f);

	const char* vertex_shader =
		"#version 130\n"
		//"in vec3 vp;"
		"uniform mat4 proj_mat;"
		"uniform mat4 model_mat;"
		"uniform mat4 view_mat;"
		//"layout(location = 0) in vec3 vp;"
		//"layout(location = 1) in vec2 tex;"
		//                           right up    towards
		"const vec3 light_pos = vec3(0.0, 0.0, 100.0);"
		"in vec3 vp;"
		"in vec2 tex;"
		"in vec3 norm;"
		"out vec2 tex_coord;"
		"out vec3 nrm;"
		"out vec4 light;"
		"void main () {"
		"  tex_coord = tex;"
		"  nrm = norm;"
		"  mat4 mvp = proj_mat* view_mat * model_mat;"
		"  gl_Position = mvp * vec4 (vp, 1.0);"

		"  if(norm != vec3(1.0)) {"
		"    vec3 normalDirection = normalize(norm);"
		"    vec3 lightDirection = normalize(light_pos);"
		"    vec3 diffuseReflection = max(vec3(0.0), dot(normalDirection, lightDirection));"
		"    light = vec4(diffuseReflection, 1.0);"
		"  } else {"
		"    light = vec4(1.0);"
		"  }"
		//"  gl_Position = mat4(1.0) * vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (MVP * vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 130\n"
		"in vec2 tex_coord;"
		"in vec3 nrm;"
		"in vec4 light;"
		"out vec4 frag_colour;"
		"uniform sampler2D tex_sampler;"
		"void main () {"
		"  vec4 texel = texture(tex_sampler, tex_coord);"
		"  if(texel.a < 0.5) { discard; } "
		"  frag_colour = texel * light;"
		"}";

	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL);
	glCompileShader (vs);
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL);
	glCompileShader (fs);

	glVertexAttrib3f(2, 1.0, 1.0, 1.0);

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

	GLint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
	view_handle = glGetUniformLocation(shader_programme, "view_mat");
	model_handle = glGetUniformLocation(shader_programme, "model_mat");
	if(proj_handle == -1 || view_handle == -1) {
		printf("Illegal uniform:\n");
	}
	glUseProgram (shader_programme);
	// Set projection matrix. View matrix is set in main loop and model matrices on every object draw.
	glm::mat4 proj_mat = glm::perspective(glm::radians(30.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);


	return true;
}


int main(int argc, char **argv)
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	GLuint window_width = WIDTH;
	GLuint window_height = HEIGHT;

	sf::Window window (sf::VideoMode(window_width, window_height), "Orbital Challenge", sf::Style::Default, settings);

	settings = window.getSettings();
	window.setVerticalSyncEnabled(true);

	std::cout << "depth bits:" << settings.depthBits << std::endl;
	std::cout << "stencil bits:" << settings.stencilBits << std::endl;
	std::cout << "antialiasing level:" << settings.antialiasingLevel << std::endl;
	std::cout << "version:" << settings.majorVersion << "." << settings.minorVersion << std::endl;

	std::string datafilename = "origdata";

	if(argc > 1) {
		datafilename = std::string(argv[1]);
	}

	init();

	bool running = true;

	GLfloat theta = 0.0f;
	GLfloat sigma = 0.0f;


	std::vector<Position> satellite_positions;
	std::vector<Position> ground_station_positions;

	{
		std::cout<<"Opening file\n";
		std::ifstream datafile(datafilename);
		std::string tmp;
		getline(datafile, tmp);

		window.setTitle("Orbital Challenge, seed: " + std::string(tmp.c_str()+7));

		GLfloat lat, lon, alt;

		while( getline(datafile, tmp) ) {
			if(tmp.compare(0, 3, "SAT") == 0) {
				std::size_t comma_pos = tmp.find_first_of(',');
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				alt = std::stof(tmp.substr(comma_pos+1));
				//std::cout<<lat << " " << lon << " " << alt <<"\n";

				satellite_positions.push_back(Position(lat, lon, alt));
			} else if(tmp.compare(0,5, "ROUTE") == 0) {
				std::size_t comma_pos = tmp.find_first_of(',');
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				ground_station_positions.push_back(Position(lat,lon,0));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				ground_station_positions.push_back(Position(lat,lon,0));
			}
		}
	}

	Sphere earth = Sphere(36, 56, 1.0f);
	earth.setTexture("earth_tex.jpg");
	earth.setShader(shader_programme);
	std::cout<<"Planet done\n";

	Tower tower;
	tower.setTexture("tower_tex_transparent.png");
	tower.setShader(shader_programme);
	std::cout<<"Tower done\n";

	Satellite sat;
	sat.setTexture("satellite_tex_transparent.png");
	sat.setShader(shader_programme);
	std::cout<<"Satellite done\n";

	Connections conns = Connections(ground_station_positions[0], ground_station_positions[1], satellite_positions);
	conns.setShader(shader_programme);

	sf::Clock fps_clock;
	GLuint frame_counter = 0;

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
				window_width = event.size.width;
				window_height = event.size.height;
				// adjust the viewport when the window is resized
				glViewport(0, 0, window_width, window_height);

				// Reset projection matrix to keep aspect ratio.
				glUseProgram (shader_programme);
				GLint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
				glm::mat4 proj_mat = glm::perspective(glm::radians(30.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
				glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);
			}
			else if (event.type == sf::Event::KeyPressed) {
				if(event.key.code == sf::Keyboard::Q) {
					running = false;
				}
			}
		}
		GLfloat moveSpeed = 0.01f;
		if(sf::Keyboard::isKeyPressed( sf::Keyboard::LShift )) {
			moveSpeed *= 5.0f;
		} else if(sf::Keyboard::isKeyPressed( sf::Keyboard::LControl )) {
			moveSpeed /= 5.0f;
		}

		if(sf::Keyboard::isKeyPressed( sf::Keyboard::W )) {
			sigma += moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::S )) {
			sigma -= moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::D )) {
			theta += moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::A )) {
			theta -= moveSpeed;
		}

		glm::vec3 lookatpos(0.0f, 0.0f, 0.0f);

		GLfloat zoomfactor = 1.0f;

		for(int i = 0; i < sf::Joystick::Count; i++) {
			if(sf::Joystick::isConnected(i)) {

				float y,x,u,v,z;
				sf::Joystick::Identification joy_id = sf::Joystick::getIdentification(i);
				if(joy_id.vendorId == 1356 && joy_id.productId == 1476) {
					// PS4
					y = - sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
					x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
					u = - sf::Joystick::getAxisPosition(0, sf::Joystick::U);
					v = sf::Joystick::getAxisPosition(0, sf::Joystick::V);
					z = - sf::Joystick::getAxisPosition(0, sf::Joystick::R);
				} else {
					// Xbox one
					y = - sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
					x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
					u = - sf::Joystick::getAxisPosition(0, sf::Joystick::U);
					v = sf::Joystick::getAxisPosition(0, sf::Joystick::V);
					z = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);
				}
				//printf("Joy %i, x: %0.3f, y: %0.3f, u: %0.3f, v: %0.3f, z: %0.3f\n", i, x, y, u, v, z);
				const float DEADZONE = 5.0f;
				if( x < DEADZONE && x > -DEADZONE ) { x = 0; }
				if( y < DEADZONE && y > -DEADZONE ) { y = 0; }
				/*if( u < DEADZONE && u > -DEADZONE ) { u = 0; }
				if( r < DEADZONE && r > -DEADZONE ) { r = 0; }*/
				if(z > 0) { z = 0; }
				sigma += y*0.0002f;
				theta += x*0.0002f;
				lookatpos[0] = -u*0.01;
				lookatpos[1] = -v*0.01;

				zoomfactor = 1 + z*0.0075;
			}
		}
		if(sigma > 1.4) { sigma = 1.4f; }
		if(sigma < -1.4) { sigma = -1.4f; }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram (shader_programme);

		glm::vec3 campos = glm::vec3(5*sin(theta)*cos(sigma),5*sin(sigma),5*cos(theta)*cos(sigma));

		//glm::vec3 lookatpos = glm::vec3(0.0f, 1.0f, 0.0f);

		lookatpos = glm::rotateX(lookatpos, (float)(-sigma));
		lookatpos = glm::rotateY(lookatpos, (float)(theta));

		glm::mat4 view_mat = glm::lookAt(
			//glm::vec3(5*sin(theta)*cos(sigma),5*sin(sigma),5*cos(theta)*cos(sigma)),
			//glm::vec3(0,0,0), // and looks at the origin
			campos,
			lookatpos,
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		glUniformMatrix4fv(view_handle, 1, GL_FALSE, &view_mat[0][0]);

		GLint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
		glm::mat4 proj_mat = glm::perspective(glm::radians(zoomfactor*30.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
		glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);

		glLineWidth(1/zoomfactor);

		earth.draw();

		for(auto &sat_pos : satellite_positions) {
			sat.draw(sat_pos);
		}

		conns.draw();

		tower.draw(ground_station_positions[0]);
		tower.draw(ground_station_positions[1]);

		window.display();

		if(++frame_counter == 10) {
			printf("FPS: %f\n", 10000.0f / fps_clock.getElapsedTime().asMilliseconds());
			frame_counter = 0;
			fps_clock.restart();
		}

		//sf::sleep(sf::milliseconds(1));
	}
	return 0;
}

