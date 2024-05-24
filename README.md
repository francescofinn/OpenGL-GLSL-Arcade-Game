----------- Instructions -----------

- Have GLFW, glad and gltext installed and ready.

- If using CMake: 
target_link_libraries(breakout -g -L"C:/path/to/mingw64/lib" -lGL -lglut -lGLU -lglfw -lX11 -lpthread -lXi -lXrandr -ldl)

^ This is how we compiled

You are able to press 1 every 50 score achieved for an enlarged paddle.

The paddle is multi-coloured, so that you can differentiate the sections and their behaviour.

The scoring goes up by 1 for each brick eliminated in the first two rows, then by 3, then by 5. There are 108 bricks (6 * 18).


