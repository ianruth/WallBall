#include "Simulator.h"


Simulator::Simulator()
{
	paddleLocation = btVector3(0.f,0.f,0.f);
	world = NULL;
}
//-------------------------------------------------------------------------------------
Simulator::~Simulator(void)
{
}

void Simulator::setupSimulator(void)
{
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	world = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
	world->setGravity(btVector3(0,0,0));

	btCollisionShape* ball = new  btSphereShape(btScalar(1.));
	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	collisionShapes.push_back(ball);
	btTransform ballTransform;
	ballTransform.setIdentity();
	ballTransform.setOrigin(btVector3(0,0,10.f));		//starting ball location
	{
		btScalar mass(1.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			ball->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(ballTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,ball,localInertia);
		ballBody = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(ballBody);
		ballBody->applyCentralForce(btVector3(0,0,20.f));					//to change ball starting velocity
	}
	{
		btCollisionShape* farWall = new  btStaticPlaneShape(btVector3(0.f,0.f,-1.f),btScalar(0.));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(farWall);
		btTransform wallTransform;
		wallTransform.setIdentity();
		wallTransform.setOrigin(btVector3(0,0,100.f));		//far wall location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			farWall->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(wallTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,farWall,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(body);
	}
	{
		btCollisionShape* rightWall = new  btStaticPlaneShape(btVector3(-1.f,0.f,0.f),btScalar(0.));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(rightWall);
		btTransform wallTransform;
		wallTransform.setIdentity();
		wallTransform.setOrigin(btVector3(30.f,0,0));		//right wall location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			rightWall->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(wallTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,rightWall,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(body);
	}
	{
		btCollisionShape* leftWall = new  btStaticPlaneShape(btVector3(1.f,0.f,0.f),btScalar(0.));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(leftWall);
		btTransform wallTransform;
		wallTransform.setIdentity();
		wallTransform.setOrigin(btVector3(-30.f,0,0));		//left wall location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			leftWall->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(wallTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,leftWall,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(body);
	}
	{
		btCollisionShape* bottomWall = new  btStaticPlaneShape(btVector3(0.f,1.f,0.f),btScalar(0.));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(bottomWall);
		btTransform wallTransform;
		wallTransform.setIdentity();
		wallTransform.setOrigin(btVector3(0,-30.f,0));		//bottom wall location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			bottomWall->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(wallTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,bottomWall,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(body);
	}
	{
		btCollisionShape* topWall = new  btStaticPlaneShape(btVector3(0.f,-1.f,0.f),btScalar(0.));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(topWall);
		btTransform wallTransform;
		wallTransform.setIdentity();
		wallTransform.setOrigin(btVector3(0,30.f,0));		//top wall location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			topWall->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(wallTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,topWall,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		//add the body to the dynamics world
		world->addRigidBody(body);
	}
	//make paddle!!!
	{
		btCollisionShape* paddleShape = new  btBoxShape(btVector3(10.f,10.f,3.f));//may need to change normals (depending on camera direction)
		collisionShapes.push_back(paddleShape);
		btTransform paddleTransform;
		paddleTransform.setIdentity();
		paddleTransform.setOrigin(btVector3(0,0,0));		//paddle location
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic){
			paddleShape->calculateLocalInertia(mass,localInertia);}
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(paddleTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,paddleShape,localInertia);
		paddleBody = new btRigidBody(rbInfo);
		paddleBody->setCollisionFlags( paddleBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		paddleBody->setActivationState(DISABLE_DEACTIVATION);
		//add the body to the dynamics world
		world->addRigidBody(paddleBody);

	}
	std::cout << "****************************************************************************************setup complete";
}

void Simulator::handleCollisions(void)
{
	std::cout << "~~~------~~~~~~~-------~~~------~~~~~~~-------got to handle collisions";
	//for(int c=0;c<1000 && simulate;c++)
	while(simulate)
	{
	//	paddleBody->getMotionState()->getWorldTransform()->setOrigin(paddleLocation);
		world->stepSimulation(1.f/60.f,10);
		//check for collisions (Somehow)			---	-		--		--		This may not work... I had trouble running Bullet at home, so this is the extent of my ability. 
		int numManifolds = world->getDispatcher()->getNumManifolds();		//	Try using world->ContactResultCallback().
		for (int i=0;i<numManifolds;i++)
		{
			btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
			btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
			btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
			btRigidBody* first = btRigidBody::upcast(obA);
			btRigidBody* second = btRigidBody::upcast(obB);
			if((first && first == ballBody) || (second && second == ballBody))			//may need two if statements to determine which is the ball (and maybe to tell which wall it is?)
			{
				std::cout << "~~~------~~~~~~~-------ball collision------~~~~~~~-------~~~";	//could also use collision points to determine the side of the ball that hit. (use contactManifold)
				//bounce ball - applycentralforce = -2 x force in wall direction
				//check other body to determine bounce direction 
				//ball sfx
			}
			if(first == paddleBody || second == paddleBody)
			{
				std::cout << "~~~------~~~~~~~-------paddle collision------~~~~~~~-------~~~";
				//paddle sfx
			}
		}
	}
}

bool Simulator::updatePaddle(Ogre::Vector3 location)
{
	paddleLocation = btVector3(location.x,location.y,location.z);
	return true;
}
	

void Simulator::runSimulator(bool sim)
{
	simulate = sim;
}

btVector3 Simulator::getBallLocation(void)			///Discover why this segfaults
{
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~getting ball location";
	btTransform* ballt;
	ballBody->getMotionState()->getWorldTransform(*ballt);
	std::cout << "got ball transform";
	btVector3 temp =ballt->getOrigin();
	return temp;
	//return Ogre::Vector3(temp.x(),temp.y(),temp.z());
}
