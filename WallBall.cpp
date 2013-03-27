/*
-----------------------------------------------------------------------------
Filename:    WallBall.cpp
-----------------------------------------------------------------------------
 
This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "WallBall.h"

Ogre::Vector3 velocity(0,0,0);
btDynamicsWorld* world;	//every physical object go to the world
btDispatcher* dispatcher;	//what collision algorithm to use?
btCollisionConfiguration* collisionConfig;	//what collision algorithm to use?
btBroadphaseInterface* broadphase;	//should Bullet examine every object, or just what close to each other
btConstraintSolver* solver;					//solve collisions, apply forces, impulses
std::vector<btRigidBody*> bodies; 
btRigidBody* gameBall;
 
//-------------------------------------------------------------------------------------
WallBall::WallBall(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    singleplayer(false),
    multiplayer(false)
{}
//-------------------------------------------------------------------------------------
WallBall::~WallBall(void)
{
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
   
    Mix_FreeMusic(music);
    SoundManager::SoundControl.cleanup();
 
    Mix_CloseAudio();
 
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}
 
btRigidBody* addPlane(float x,float y,float z,btVector3 normal)
{
	//similar to createSphere
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(x,y,z));
	btStaticPlaneShape* plane=new btStaticPlaneShape(normal,0);
	btMotionState* motion=new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(0.0,motion,plane);
	btRigidBody* body=new btRigidBody(info);
    body->setRestitution(1.0);
	world->addRigidBody(body);
	bodies.push_back(body);
	return body;
}

btRigidBody* addSphere(float rad,float x,float y,float z,float mass)
{
	btTransform t;	//position and rotation
	t.setIdentity();
	t.setOrigin(btVector3(x,y,z));	//put it to x,y,z coordinates
	btSphereShape* sphere=new btSphereShape(rad);	//it's a sphere, so use sphereshape
	btVector3 inertia(0,0,0);	//inertia is 0,0,0 for static object, else
	if(mass!=0.0)
		sphere->calculateLocalInertia(mass,inertia);	//it can be determined by this function (for all kind of shapes)
	
	btMotionState* motion=new btDefaultMotionState(t);	//set the position (and motion)
	btRigidBody::btRigidBodyConstructionInfo info(mass,motion,sphere,inertia);	//create the constructioninfo, you can create multiple bodies with the same info
	btRigidBody* body=new btRigidBody(info);	//let's create the body itself
  body->setRestitution(1);
	body->setLinearVelocity(btVector3(0,0,-300));
	world->addRigidBody(body);	//and let the world know about it
	bodies.push_back(body);	//to be easier to clean, I store them a vector
	return body;
}

void initPhysics(void)
{
	//pretty much initialize everything logically
	collisionConfig=new btDefaultCollisionConfiguration();
	dispatcher=new btCollisionDispatcher(collisionConfig);
	broadphase=new btDbvtBroadphase();
	solver=new btSequentialImpulseConstraintSolver();
	world=new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
	world->setGravity(btVector3(0,0,0));	//gravity on Earth

	addPlane(-100,0,0,btVector3(1,0,0));
	addPlane(100,0,0,btVector3(-1,0,0));
    addPlane(0,-100,0,btVector3(0,1,0));
	addPlane(0,100,0,btVector3(0,-1,0));
	addPlane(0,0,-300,btVector3(0,0,1));
	//addPlane(0,0,300,btVector3(0,0,-1));

	gameBall = addSphere(10,0,0,0,5);
}

bool WallBall::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif
    hasPowerUp = false;
    score=0;
    effect=0;
    // construct Ogre::Root
    mRoot = new Ogre::Root(mPluginsCfg);
 
//-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
 
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
//-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->restoreConfig() || mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "WallBall Render Window");
    }
    else
    {
        return false;
    }
//-------------------------------------------------------------------------------------
    // choose scenemanager
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
//-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
 
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,50,600));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,-10,0));
    mCamera->setNearClipDistance(5);
 
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
//-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
 
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
//-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
//-------------------------------------------------------------------------------------
    // Create any resource listeners (for loading screens)
    //createResourceListener();
//-------------------------------------------------------------------------------------
    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
//-------------------------------------------------------------------------------------
    // Create the scene
    inPlay = true;
    Ogre::Entity* ball = mSceneMgr->createEntity("Ball", "sphere.mesh");
    mSceneMgr->createEntity("PowerUpSphere", "sphere.mesh");
    mSceneMgr->createEntity("PowerUpCube", "cube.mesh");
    Ogre::SceneNode* ballNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("BallNode");
    ballNode->attachObject(ball);
    ballNode->scale(.1,.1,.1);
    mCamera->setPolygonMode(Ogre::PM_WIREFRAME);
    Ogre::Entity* paddle = mSceneMgr->createEntity("Paddle", "cube.mesh");
    Ogre::SceneNode* paddleNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PaddleNode");
    paddleNode->attachObject(paddle);
    paddleNode->scale(.50,.50,.50);
    paddleNode->scale(1.5,.8,.2);
  //  paddleNode->roll(Ogre::Degree(90));
    paddleNode->setPosition(0, -50, 300);
   // paddle->setMaterialName("paddleTexture.tga");				Try to make transparent
    
    srand ( time(NULL) );
    velocity.x = rand() % 10*20;
    velocity.y = rand() % 10*20;
    velocity.z = -200;

    Ogre::Plane roofFront(Ogre::Vector3::NEGATIVE_UNIT_Y, -100);
    Ogre::Plane roofBack(Ogre::Vector3::UNIT_Y, 100);
    Ogre::Plane floorFront(Ogre::Vector3::UNIT_Y, -100);
    Ogre::Plane floorBack(Ogre::Vector3::NEGATIVE_UNIT_Y, 100);
    Ogre::Plane rightWallFront(Ogre::Vector3::NEGATIVE_UNIT_X, -100);
    Ogre::Plane rightWallBack(Ogre::Vector3::UNIT_X, 100);
    Ogre::Plane leftWallFront(Ogre::Vector3::UNIT_X, -100);
    Ogre::Plane leftWallBack(Ogre::Vector3::NEGATIVE_UNIT_X, 100);
    //Ogre::Plane backWallFront(Ogre::Vector3::NEGATIVE_UNIT_Z, -300);
    //Ogre::Plane backWallBack(Ogre::Vector3::UNIT_Z, 300);
    Ogre::Plane frontWallFront(Ogre::Vector3::UNIT_Z, -300);
    Ogre::Plane frontWallBack(Ogre::Vector3::NEGATIVE_UNIT_Z, 300);
    
    Ogre::MeshManager::getSingleton().createPlane("roofFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    roofFront, 200, 600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);   
    Ogre::MeshManager::getSingleton().createPlane("roofBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    roofBack, 200, 600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
    Ogre::MeshManager::getSingleton().createPlane("floorFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    floorFront, 200, 600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
    Ogre::MeshManager::getSingleton().createPlane("floorBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    floorBack, 200, 600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
    Ogre::MeshManager::getSingleton().createPlane("rightWallFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    rightWallFront, 600, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
    Ogre::MeshManager::getSingleton().createPlane("rightWallBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    rightWallBack, 600, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
    Ogre::MeshManager::getSingleton().createPlane("leftWallFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    leftWallFront, 600, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
    Ogre::MeshManager::getSingleton().createPlane("leftWallBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    leftWallBack, 600, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
    /*Ogre::MeshManager::getSingleton().createPlane("backWallFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    backWallFront, 200, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_X);
    Ogre::MeshManager::getSingleton().createPlane("backWallBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    backWallBack, 200, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_X);*/
    Ogre::MeshManager::getSingleton().createPlane("frontWallFront", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    frontWallFront, 200, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_X);
    Ogre::MeshManager::getSingleton().createPlane("frontWallBack", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    frontWallBack, 200, 200, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_X); 
    
    Ogre::Entity* entRoofFront = mSceneMgr->createEntity("RoofFrontEntity", "roofFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entRoofFront);
    entRoofFront->setMaterialName("Examples/Rockwall");
    entRoofFront->setCastShadows(false);

    Ogre::Entity* entRoofBack = mSceneMgr->createEntity("RoofBackEntity", "roofBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entRoofBack);
    entRoofBack->setMaterialName("Examples/Rockwall");
    entRoofBack->setCastShadows(false);

    Ogre::Entity* entFloorFront = mSceneMgr->createEntity("FloorFrontEntity", "floorFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entFloorFront);
    entFloorFront->setMaterialName("Examples/Rockwall");
    entFloorFront->setCastShadows(false);

    Ogre::Entity* entFloorBack = mSceneMgr->createEntity("FloorBackEntity", "floorBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entFloorBack);
    entFloorBack->setMaterialName("Examples/Rockwall");
    entFloorBack->setCastShadows(false);

    Ogre::Entity* entRightWallFront = mSceneMgr->createEntity("RightWallFront", "rightWallFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entRightWallFront);
    entRightWallFront->setMaterialName("Examples/Rockwall");
    entRightWallFront->setCastShadows(false);

    Ogre::Entity* entRightWallBack = mSceneMgr->createEntity("RightWallBack", "rightWallBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entRightWallBack);
    entRightWallBack->setMaterialName("Examples/Rockwall");
    entRightWallBack->setCastShadows(false);

    Ogre::Entity* entLeftWallFront = mSceneMgr->createEntity("LefttWallFront", "leftWallFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entLeftWallFront);
    entLeftWallFront->setMaterialName("Examples/Rockwall");
    entLeftWallFront->setCastShadows(false);

    Ogre::Entity* entLeftWallBack = mSceneMgr->createEntity("LeftWallBack", "leftWallBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entLeftWallBack);
    entLeftWallBack->setMaterialName("Examples/Rockwall");
    entLeftWallBack->setCastShadows(false);
/*
    Ogre::Entity* entBackWallFront = mSceneMgr->createEntity("BackWallFront", "backWallFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entBackWallFront);
    entBackWallFront->setMaterialName("Examples/Rockwall");
    entBackWallFront->setCastShadows(false);

    Ogre::Entity* entBackWallBack = mSceneMgr->createEntity("BackWallBack", "backWallBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entBackWallBack);
    entBackWallBack->setMaterialName("Examples/Rockwall");
    entBackWallBack->setCastShadows(false);
*/
    Ogre::Entity* entFrontWallFront = mSceneMgr->createEntity("FrontWallFront", "frontWallFront");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entFrontWallFront);
    entFrontWallFront->setMaterialName("Examples/Rockwall");
    entFrontWallFront->setCastShadows(false);

    Ogre::Entity* entFrontWallBack = mSceneMgr->createEntity("FrontWallBack", "frontWallBack");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entFrontWallBack);
    entFrontWallBack->setMaterialName("Examples/Rockwall");
    entFrontWallBack->setCastShadows(false);

    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25, 0.25, 0.25));
 
    // Create a light
    Ogre::Light* spotLight1 = mSceneMgr->createLight("spotLight1");
    spotLight1->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight1->setDiffuseColour(0, 0, 1);
    spotLight1->setSpecularColour(0, 0, 1);
    spotLight1->setDirection(0, -1, 0);
    spotLight1->setPosition(0,15,0);

    Ogre::Light* spotLight2 = mSceneMgr->createLight("spotLight2");
    spotLight2->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight2->setDiffuseColour(1, 0, 0);
    spotLight2->setSpecularColour(1, 0, 0);
    spotLight2->setDirection(0, 1, 0);
    spotLight2->setPosition(0,-15,0);

    Ogre::Light* spotLight3 = mSceneMgr->createLight("spotLight3");
    spotLight3->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight3->setDiffuseColour(0, 1, 0);
    spotLight3->setSpecularColour(0, 1, 0);
    spotLight3->setDirection(-1, 0, 0);
    spotLight3->setPosition(15,0,0);

    Ogre::Light* spotLight4 = mSceneMgr->createLight("spotLight4");
    spotLight4->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight4->setDiffuseColour(.5, .5, .5);
    spotLight4->setSpecularColour(.5, .5, .5);
    spotLight4->setDirection(1, 0, 0);
    spotLight4->setPosition(-15,0,0);

    Ogre::Light* spotLight5 = mSceneMgr->createLight("spotLight5");
    spotLight5->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight5->setDiffuseColour(1, 1, 0);
    spotLight5->setSpecularColour(1, 1, 0);
    spotLight5->setDirection(0, 0, -1);
    spotLight5->setPosition(0,0,15);

    Ogre::Light* spotLight6 = mSceneMgr->createLight("spotLight6");
    spotLight6->setType(Ogre::Light::LT_SPOTLIGHT);
    spotLight6->setDiffuseColour(0, 1, 1);
    spotLight6->setSpecularColour(0, 1, 1);
    spotLight6->setDirection(0, 0, 1);
    spotLight6->setPosition(0,0,-15);
    
    ballNode->attachObject(spotLight1);
    ballNode->attachObject(spotLight2);
    ballNode->attachObject(spotLight3);
    ballNode->attachObject(spotLight4);
    ballNode->attachObject(spotLight5);
    ballNode->attachObject(spotLight6);

	initPhysics();

//-------------------------------------------------------------------------------------
    //load Sounds
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
        return false;
    
    if((ballBounceWall = SoundManager::SoundControl.loadWAV("sound/slap2.wav")) == -1)
        return false;
    if((ballBouncePaddle = SoundManager::SoundControl.loadWAV("sound/slap1.wav")) == -1)
        return false;
    if((ballReset = SoundManager::SoundControl.loadWAV("sound/sword_hit_single2.wav")) == -1)
        return false;
    if((ballPowerUp = SoundManager::SoundControl.loadWAV("sound/object_being_squash.wav")) == -1)
        return false;
    SoundManager::SoundControl.playAudio();

    music = Mix_LoadMUS("sound/risveglio.wav");
    Mix_PlayMusic(music, -1);
    
//-------------------------------------------------------------------------------------
    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
 
    mInputManager = OIS::InputManager::createInputSystem( pl );
 
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
 
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
 
    //Set initial mouse clipping size
    windowResized(mWindow);
 
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
 
    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    GUIManager::GUIControl.setup(mTrayMgr);
 
    mRoot->addFrameListener(this);
//-------------------------------------------------------------------------------------
    mRoot->startRendering();
 
    return true;
}
 
bool WallBall::frameRenderingQueued(const Ogre::FrameEvent& evt)
{

    if(mWindow->isClosed())
        return false;
 
    if(mShutDown)
        return false;
 
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();
    
    GUIManager::GUIControl.frameRenderingQueued(evt); 

    if(singleplayer || multiplayer){
        if (!GUIManager::GUIControl.isDialogVisible())
        {
            mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
            if (GUIManager::GUIControl.isScoreboardVisible())   // if details panel is visible, then update its contents
            {
                GUIManager::GUIControl.inPlay(inPlay, effect, timer, score);
            }
        }
    
		Ogre::SceneNode* ball = mSceneMgr->getSceneNode("BallNode");

		world->stepSimulation(1/60.0);
        btVector3 v = gameBall->getLinearVelocity();
        if(v[2] < 250 && v[2] >= 0)
			gameBall->setLinearVelocity(btVector3(v[0],v[1],250));
        else if(v[2] > -250 && v[2] < 0)
            gameBall->setLinearVelocity(btVector3(v[0],v[1],-250));

        v = gameBall->getLinearVelocity();
        if(v[1] < 100 && v[1] >= 0)
			gameBall->setLinearVelocity(btVector3(v[0],100,v[2]));
        else if(v[1] > -250 && v[1] < 0)
            gameBall->setLinearVelocity(btVector3(v[0],-100,v[2]));

        v = gameBall->getLinearVelocity();
        if(v[0] < 250 && v[0] >= 0)
			gameBall->setLinearVelocity(btVector3(100,v[1],v[2]));
        else if(v[0] > -250 && v[0] < 0)
            gameBall->setLinearVelocity(btVector3(-100,v[1],v[2]));
		
		btTransform t;
	    gameBall->getMotionState()->getWorldTransform(t);
        btVector3 position = t.getOrigin();
        ball->setPosition(Ogre::Vector3((float)position[0],(float)position[1],(float)position[2]));

        btVector3 btVelocity = gameBall->getLinearVelocity();

		Ogre::SceneNode* paddle = mSceneMgr->getSceneNode("PaddleNode");
        Ogre::Vector3 paddlePosition = paddle->getPosition();


        Ogre::Vector3 ballPosition = ball->getPosition();
        if(ballPosition.x>90) {
            ballPosition.x = 90;
            velocity.x *= -1;
			gameBall->setLinearVelocity(btVector3(-1*btVelocity[0],btVelocity[1],btVelocity[2]));
            SoundManager::SoundControl.playClip(ballBounceWall, 0);
        } else if(ballPosition.x<-90) {
            ballPosition.x = -90;
            velocity.x *= -1;
			gameBall->setLinearVelocity(btVector3(-1*btVelocity[0],btVelocity[1],btVelocity[2]));
            SoundManager::SoundControl.playClip(ballBounceWall, 0);
        }
        if(ballPosition.y>90) {
            ballPosition.y=90;
            velocity.y *= -1;
			gameBall->setLinearVelocity(btVector3(btVelocity[0],-1*btVelocity[1],btVelocity[2]));
            SoundManager::SoundControl.playClip(ballBounceWall, 0);
        } else if(ballPosition.y<-90) {
            ballPosition.y = -90;
            velocity.y *= -1;
			gameBall->setLinearVelocity(btVector3(btVelocity[0],-1*btVelocity[1],btVelocity[2]));
            SoundManager::SoundControl.playClip(ballBounceWall, 0);
        } 
        if(ballPosition.z>290 && ballPosition.z<300) {
            /*  if( (ballPosition.x <= paddlePosition.x && ballPosition.x >= (paddlePosition.x - 100)) && (ballPosition.y >= (paddlePosition.y - 20) && ballPosition.y <= (paddlePosition.y + 20)) ){
            ballPosition.z = 290;
            velocity.x += 100 * (-(paddlePosition.x - ballPosition.x)/100);
            velocity.y += 100 * (-(paddlePosition.y - ballPosition.y)/20);
            velocity.z *= -1;
            SoundManager::SoundControl.playClip(ballBouncePaddle, 0);
            }*/
	        Ogre::SceneNode* paddleNode = mSceneMgr->getSceneNode("PaddleNode");
	        paddleNode->_update(false,false);
	        Ogre::AxisAlignedBox bounds = paddleNode->_getWorldAABB();
	        Ogre::Vector3 max = bounds.getMaximum();
	        Ogre::Vector3 min = bounds.getMinimum();
	        if(ballPosition.x-10 <= max.x && ballPosition.y-10 <= max.y && ballPosition.x+10 >= min.x && ballPosition.y+10 >= min.y)
	        {
		        ballPosition.z = 290;
		        velocity.x += 100 * (-(paddlePosition.x - ballPosition.x)/(bounds.getSize().x/2 + 10));
		        velocity.y += 100 * (-(paddlePosition.y - ballPosition.y)/(bounds.getSize().y/2 + 10));
		        velocity.z *= -1;
		        gameBall->setLinearVelocity(btVector3(btVelocity[0]+ (100 * (-(paddlePosition.x - ballPosition.x)/(bounds.getSize().x/2 + 10))),
			    btVelocity[1]+ (100 * (-(paddlePosition.y - ballPosition.y)/(bounds.getSize().y/2 + 10))),-1*btVelocity[2]));
		        SoundManager::SoundControl.playClip(ballBouncePaddle, 0);
	        }
        } else if(ballPosition.z<-290) {
            ballPosition.z = -290;
            velocity.z *= -1;
			gameBall->setLinearVelocity(btVector3(btVelocity[0],btVelocity[1],-1*btVelocity[2]));
            SoundManager::SoundControl.playClip(ballBounceWall, 0);
        }
        if(ballPosition.z>300)
	       inPlay=false;
    		
        ball->setPosition(ballPosition);
        //ball->translate(evt.timeSinceLastFrame*velocity);

	   t.setOrigin(btVector3(ballPosition.x,ballPosition.y,ballPosition.z));
	   gameBall->getMotionState()->setWorldTransform(t);
		

	   if(hasPowerUp)				///Powerup Collisions (if no bullet)
	   {
		    Ogre::SceneNode* powerupNode = mSceneMgr->getSceneNode("PowerUpNode");
		    powerupNode->_update(false,false);
		    Ogre::AxisAlignedBox bounds =powerupNode->_getWorldAABB();
		    Ogre::Vector3 max = bounds.getMaximum() - 100;
		    Ogre::Vector3 min = bounds.getMinimum() + 100;
		    if(ballPosition.x <= max.x && ballPosition.y <= max.y && ballPosition.z <= max.z && ballPosition.x >= min.x && ballPosition.y >= min.y && ballPosition.z >= min.z)
		    {
			    hasPowerUp=false;
			    if(effect<=1)
			    {
				    score+=100;
				    velocity += Ogre::Vector3((velocity.x/Ogre::Math::Abs(velocity.x))*100.f,(velocity.y/Ogre::Math::Abs(velocity.y))*100.f,(velocity.z/Ogre::Math::Abs(velocity.z))*100.f); 
                    gameBall->setLinearVelocity(btVector3((velocity.x/Ogre::Math::Abs(velocity.x))*100.f,(velocity.y/Ogre::Math::Abs(velocity.y))*100.f,(velocity.z/Ogre::Math::Abs(velocity.z))*100.f));
			    }
		      	if(effect>=2)
		      	{
				    score+=100;
				    velocity /= 1.5;
				    gameBall->setLinearVelocity(btVector3(btVelocity[0]/1.5,btVelocity[1]/1.5,-1*btVelocity[2]/1.5));
			    }	
			    mSceneMgr->destroySceneNode("PowerUpNode");	
		    }
	    }
        ball->pitch( Ogre::Degree( evt.timeSinceLastFrame*velocity.x ) );
        ball->yaw( Ogre::Degree( evt.timeSinceLastFrame*velocity.y ) );
        ball->roll( Ogre::Degree( evt.timeSinceLastFrame*velocity.z ) );
        generatePowerup();

	   //velocity += Ogre::Vector3((velocity.x/Ogre::Math::Abs(velocity.x))*.1f,(velocity.y/Ogre::Math::Abs(velocity.y))*.1f,(velocity.z/Ogre::Math::Abs(velocity.z))*.1f);
    } 
    return true;
}
//-------------------------------------------------------------------------------------
bool WallBall::keyPressed( const OIS::KeyEvent &arg )
{
    if (GUIManager::GUIControl.isDialogVisible()) return true;   // don't process any more keys if dialog is up
    
    else if (arg.key == OIS::KC_T)   // cycle texture filtering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;
 
        switch (GUIManager::GUIControl.getScoreboardParam(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }
 
        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        GUIManager::GUIControl.setScoreboardParam(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;
 
        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }
 
        mCamera->setPolygonMode(pm);
        GUIManager::GUIControl.setScoreboardParam(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        GUIManager::GUIControl.pause();
    }

    else if (arg.key == OIS::KC_RETURN)
    {       
		SoundManager::SoundControl.playClip(ballReset, 0);
        mSceneMgr->getSceneNode("BallNode")->setPosition(0,0,0);
	    velocity = Ogre::Vector3(0.f,0.f,300.f);

    	world->removeRigidBody(gameBall);
	    gameBall = addSphere(10,0,0,0,5);


    	timer.reset();
    	effect=0;
    	hasPowerUp=false;
    	mSceneMgr->destroySceneNode("PowerUpNode");
    	score=0;
    	inPlay = true;
        velocity.z *= -1;
    }

    //mCameraMan->injectKeyDown(arg);
    return true;
}
 
bool WallBall::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}
 
bool WallBall::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    //mCameraMan->injectMouseMove(arg);
    
    Ogre::SceneNode* paddleNode = mSceneMgr->getSceneNode("PaddleNode");
    paddleNode->translate(arg.state.X.rel/2, -(arg.state.Y.rel)/2, 0);
    Ogre::Vector3 position = paddleNode->getPosition();
    paddleNode->_update(false,false);
    Ogre::AxisAlignedBox bounds = paddleNode->_getWorldAABB();
    Ogre::Vector3 max = bounds.getMaximum();
    Ogre::Vector3 min = bounds.getMinimum();
    if(max.x > 100)
        position.x -= max.x - 100;
    else if(min.x < -100)
        position.x -= min.x + 100;
    if(max.y > 100)
        position.y -= max.y - 100;
    else if(min.y < -100)
        position.y -= min.y + 100;

    paddleNode->setPosition(position);
    mSceneMgr->getCamera("PlayerCam")->lookAt(position/2);
    return true;
}

void WallBall::buttonHit(OgreBites::Button* button){
    if (button->getName().compare("Singleplayer") == 0){
        singleplayer = true;
        GUIManager::GUIControl.end_MainScreen();
    }
    else if (button->getName().compare("Multiplayer") == 0){
        multiplayer = true;
        GUIManager::GUIControl.end_MainScreen();
        GUIManager::GUIControl.begin_MultiplayerScreen();
    }
    else if (button->getName().compare("Exit") == 0 || button->getName().compare("PauseExit") == 0)
        mShutDown = true;
    else if(button->getName().compare("Resume") == 0)
        GUIManager::GUIControl.pause();
    else if(button->getName().compare("MainMenu") == 0){
        GUIManager::GUIControl.pause();
        GUIManager::GUIControl.begin_MainScreen();
    }
    else if(button->getName().compare("Back") == 0){
        GUIManager::GUIControl.end_MultiplayerScreen();
        GUIManager::GUIControl.begin_MainScreen();
    }
    else if(button->getName().compare("Host") == 0)
        GUIManager::GUIControl.end_MultiplayerScreen();
    else if(button->getName().compare("Client") == 0)
        GUIManager::GUIControl.end_MultiplayerScreen();
}
 
bool WallBall::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}
 
bool WallBall::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}
 
//Adjust mouse clipping area
void WallBall::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);
 
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}
 
//Unattach OIS before window shutdown (very important under Linux)
void WallBall::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );
 
            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

void NetworkHost(void)      //http://content.gpwiki.org/index.php/SDL:Tutorial:Using_SDL_net <- good code here, adapt messages accordingly
{                           //http://jcatki.no-ip.org:8080/SDL_net/SDL_net.html <- good documentation here
  TCPsocket sd, csd;      /* Socket descriptor, Client socket descriptor */
  IPaddress ip, *remoteIP;
  int quit, quit2;
  char buffer[512];

	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	/* Resolving the host using NULL make network interface to listen */
	if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(sd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	/* Wait for a connection, send data and term */
	quit = 0;
	while (!quit)
	{
		/* This check the sd if there is a pending connection.
		* If there is one, accept that, and open a new socket for communicating */
		if ((csd = SDLNet_TCP_Accept(sd)))
		{
			/* Now we can communicate with the client using csd socket
			* sd will remain opened waiting other connections */
 
			/* Get the remote address */
			if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
				/* Print the address, converting in the host format */
				printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
			else
				fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
 
			quit2 = 0;
			while (!quit2)
			{
				if (SDLNet_TCP_Recv(csd, buffer, 512) > 0)
				{
					printf("Client say: %s\n", buffer);
 
					if(strcmp(buffer, "exit") == 0)	/* Terminate this connection */
					{
						quit2 = 1;
						printf("Terminate connection\n");
					}
					if(strcmp(buffer, "quit") == 0)	/* Quit the program */
					{
						quit2 = 1;
						quit = 1;
						printf("Quit program\n");
					}
				}
				//printf("Write something:\n>");
				//scanf("%s", buffer);
		 						//put message in buffer
				len = strlen(buffer) + 1;
				if (SDLNet_TCP_Send(csd, (void *)buffer, len) < len)
				{
					fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
					exit(EXIT_FAILURE);
				}

			}
 
			/* Close the client socket */
			SDLNet_TCP_Close(csd);
		}
	}
 
	SDLNet_TCP_Close(sd);
	SDLNet_Quit();
}

void NetworkClient(void)    //http://content.gpwiki.org/index.php/SDL:Tutorial:Using_SDL_net <- good code here, adapt messages accordingly
{                           //http://jcatki.no-ip.org:8080/SDL_net/SDL_net.html <- good documentation here
    IPaddress ip;           /* Server address */
    TCPsocket sd;           /* Socket descriptor */
    int quit, len;
    char buffer[512];
}

void WallBall::generatePowerup(void)
{
	if(/*Ogre::Math::RangeRandom(0,5000) > 10 ||*/ hasPowerUp)
		return;
	//PowerUp spawned = PowerUp::PowerUp(Ogre::Math::RangeRandom(1,4));
	effect = Ogre::Math::RangeRandom(0,3);
	Ogre::Entity* power;
	if(effect <= 1)
	{
		power = mSceneMgr->getEntity("PowerUpSphere");
	}
	if(effect >= 2)
	{
		power = mSceneMgr->getEntity("PowerUpSphere");
	}
	Ogre::SceneNode* powerupNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PowerUpNode");
	powerupNode->scale(0.4f,0.4f,0.4f);
	float Powx = Ogre::Math::RangeRandom(0,200)-100;
	float Powy = Ogre::Math::RangeRandom(0,200)-100;
	float Powz = Ogre::Math::RangeRandom(0,600)-300;
	powerupNode->translate(Ogre::Vector3(Powx,Powy,Powz));
	powerupNode->attachObject(power);
	powerupNode->_update(false,false);
	hasPowerUp=true;
/*	if(effect == 3)
	{
		power = mSceneMgr->createEntity("PowerUp", "sphere.mesh");
	}
	if(effect == 4)
	{
		power = mSceneMgr->createEntity("PowerUp", "sphere.mesh");
	}*/
	
}
 
 
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        WallBall app;
 
        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif
