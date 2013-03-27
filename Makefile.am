noinst_HEADERS = WallBall.h SoundManager.h GUIManager.h Simulator.h

bin_PROGRAMS = WallBall
WallBall_CPPFLAGS = -I$(top_srcdir)
WallBall_SOURCES = WallBall.cpp SoundManager.cpp GUIManager.cpp Simulator.cpp
WallBall_CXXFLAGS = $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(SDL_CFLAGS) $(SDL_mixer_CFLAGS)
WallBall_LDADD = $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(SDL_LIBS) $(SDL_mixer_LIBS)

EXTRA_DIST = buildit makeit
AUTOMAKE_OPTIONS = foreign
