g++ -std=c++17 src/*.cpp include/*.cpp -o build/AtmosphericEngine -Iinclude -I/usr/local/include/bullet -lGLEW -lGLFW -lbulletdynamics -lbulletcollision -llinearmath -framework OpenGL
cp -r shaders build
cp -r resources build