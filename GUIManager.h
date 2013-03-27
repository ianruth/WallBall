//GUIManager.h

#ifndef _GUIMANAGER_H_
    #define _GUIMANAGER_H_
 
#include <OISMouse.h>
#include <SdkTrays.h>

class GUIManager{
    private:
        OgreBites::SdkTrayManager *mTrayMgr;
        OgreBites::ParamsPanel* mScoreBoard;
        bool main;
        bool mPause;
        void createScoreBoard();
        void createMainScreen();
        void removeMainScreen();
        void createPauseScreen();
        void removePauseScreen();
    public:
        GUIManager();
        ~GUIManager();
        void setup(OgreBites::SdkTrayManager* trayMgr);
        void begin_mainScreen();
        void end_mainScreen();
        void pause();
        void inPlay(bool inPlay, int effect, Ogre::Timer timer, int score);
        bool isScoreboardVisible();
        Ogre::DisplayString getScoreboardParam(unsigned int i);
        void setScoreboardParam(unsigned int i, Ogre::String name);
        bool isDialogVisible();
        void frameRenderingQueued(const Ogre::FrameEvent& evt);

        static GUIManager GUIControl;
};

#endif