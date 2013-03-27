//GUIManager.cpp

#include "GUIManager.h"

GUIManager GUIManager::GUIControl;

    GUIManager::GUIManager() :
    main(true),
    mPause(true)
    {}

    GUIManager::~GUIManager() {
        mTrayMgr->destroyAllWidgets();
    }

    void GUIManager::createScoreBoard(){
        // create a params panel for displaying score details
        Ogre::StringVector items;    
        items.push_back("Score");
        items.push_back("");
        items.push_back("");
        items.push_back("");
        items.push_back("");
        items.push_back("");
        items.push_back("Power Up");
        items.push_back("");
        items.push_back("");
        items.push_back("Filtering");
        items.push_back("Poly Mode");
        mScoreBoard = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "ScoreBoard", 200, items);
        mScoreBoard->setParamValue(9, "Bilinear");
        mScoreBoard->setParamValue(10, "Solid");
    }

    void GUIManager::createMainScreen(){
        mTrayMgr->createLabel(OgreBites::TL_CENTER, "MainScreen", "Main Menu");
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Singleplayer", "Singleplayer", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Multiplayer", "Multiplayer", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Exit", "Exit", 250);
        mTrayMgr->showCursor();
        mTrayMgr->showBackdrop("Examples/Chrome");
    }

    void GUIManager::removeMainScreen(){
        mTrayMgr->destroyWidget("MainScreen");
        mTrayMgr->destroyWidget("Singleplayer");
        mTrayMgr->destroyWidget("Multiplayer");
        mTrayMgr->destroyWidget("Exit");
        mTrayMgr->hideCursor();
        mTrayMgr->hideBackdrop();
    }

    void GUIManager::createMultiplayerScreen(){
        mTrayMgr->createLabel(OgreBites::TL_CENTER, "MultiScreen", "Multiplayer");
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Host", "Host", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Client", "Client", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Back", "Back", 250);
        mTrayMgr->showCursor();
        mTrayMgr->showBackdrop("Examples/BeachStones");
    }

    void GUIManager::removeMultiplayerScreen(){
        mTrayMgr->destroyWidget("MultiScreen");
        mTrayMgr->destroyWidget("Host");
        mTrayMgr->destroyWidget("Client");
        mTrayMgr->destroyWidget("Back");
        mTrayMgr->hideCursor();
        mTrayMgr->hideBackdrop();
    }

    void GUIManager::createPauseScreen(){
        mTrayMgr->createLabel(OgreBites::TL_CENTER, "PauseScreen", "Pause");
        mTrayMgr->createButton(OgreBites::TL_CENTER, "Resume", "Resume", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "MainMenu", "Main Menu", 250);
        mTrayMgr->createButton(OgreBites::TL_CENTER, "PauseExit", "Exit", 250);
    }

    void GUIManager::removePauseScreen(){
        mTrayMgr->destroyWidget("PauseScreen");
        mTrayMgr->destroyWidget("Resume");
        mTrayMgr->destroyWidget("MainMenu");
        mTrayMgr->destroyWidget("PauseExit");
    }

    void GUIManager::setup(OgreBites::SdkTrayManager* trayMgr){
	    mTrayMgr = trayMgr;

        createMainScreen();
        createScoreBoard();
        mScoreBoard->hide();

        Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();

    }

    void GUIManager::begin_MainScreen(){
        if(!main)
            mScoreBoard->hide();
        main = true;
        createMainScreen();
    }

    void GUIManager::end_MainScreen(){
        main = false;
        removeMainScreen();
        mScoreBoard->show();
    }

    void GUIManager::begin_MultiplayerScreen(){
        mScoreBoard->hide();
        createMultiplayerScreen();
    }
    
    void GUIManager::end_MultiplayerScreen(){
        removeMultiplayerScreen();
        mScoreBoard->show();
    }

    void GUIManager::pause(){
        if(mPause){
            mTrayMgr->showCursor();
            createPauseScreen();
            mPause = false;
        }else{
            mTrayMgr->hideCursor();
            removePauseScreen();
            mPause = true;
        }
    }

    void GUIManager::inPlay(bool inPlay, int effect, Ogre::Timer timer, int score){
		if(inPlay){
        	mScoreBoard->setParamValue(0, Ogre::StringConverter::toString((timer.getMilliseconds()/100)+score));
		    mScoreBoard->setParamValue(1, "");
		}else
            mScoreBoard->setParamValue(1, "GAME OVER");

        mScoreBoard->setParamValue(2, "Bounce the ball for");
        mScoreBoard->setParamValue(3, "as long as possible");
        mScoreBoard->setParamValue(4, "to win!");
        mScoreBoard->setParamValue(6, "");

	    if(effect<=1)
          	mScoreBoard->setParamValue(6, "Accelerate");
	    else if(effect>=2)
        	mScoreBoard->setParamValue(6, "Slow");

	    mScoreBoard->setParamValue(7, "Press Enter to Restart");
    }

    bool GUIManager::isScoreboardVisible(){
        return mScoreBoard->isVisible();
    }

    Ogre::DisplayString GUIManager::getScoreboardParam(unsigned int i){
        return mScoreBoard->getParamValue(i);
    }

    void GUIManager::setScoreboardParam(unsigned int i, Ogre::String newVal){
        mScoreBoard->setParamValue(i, newVal);
    }

    bool GUIManager::isDialogVisible(){
        return mTrayMgr->isDialogVisible();
    }

    void GUIManager::frameRenderingQueued(const Ogre::FrameEvent& evt){
        mTrayMgr->frameRenderingQueued(evt);
        mTrayMgr->adjustTrays();
    }