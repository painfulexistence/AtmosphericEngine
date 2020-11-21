#!/bin/zsh


SRC_DIR="src"
DIST_DIR="build"
EXT_DIR="external"
RES_DIR="resources"

COMPLETE="false"
if [ $# -ne 0 ]
then
    while getopts "bcfh" opts
    do
        case $opts in
            [fc])
                COMPLETE="true"
                echo "This is a complete build which may take a little longer"
                echo ""
                ;;
            *)
                echo "Usage: run '$0 -c' or '$0 -f' for a complete build"
                exit 1
                ;;
        esac
    done
fi

echo "Building source files..."
g++ -std=c++17 $SRC_DIR/*.cpp $SRC_DIR/*/*.cpp $EXT_DIR/*.cpp -o $DIST_DIR/AtmosphericEngine -I$EXT_DIR -I$EXT_DIR/entt/src -I/usr/local/include/bullet -I/opt/local/include -L/opt/local/lib -lGLEW -lGLFW -lbulletsoftbody -lbulletdynamics -llinearmath -lbulletcollision -llua -framework OpenGL
if [ $COMPLETE = "true" ]
then
    echo ""
    echo "Copying resources..."
    cp -r $RES_DIR $DIST_DIR
fi

echo ""
echo ""

exit 0