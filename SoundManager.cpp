//SoundManager.cpp
#include "SoundManager.h"

class SoundManager {
	
	SoundManager SoundManager::SoundControl;

	SoundManager::SoundManager(){
		//Load SDL Library
		if(SDL_Init(SDL_INIT_AUDIO) < 0)
			std::string msg = std::string("Could'nt initialize SDL: ") + SDL_get_error();
		SoundList = ;
	}

	int SoundManager::loadWAV(char nym[32]){
		Mix_Chunk* TempSound = NULL;
 
    	if((TempSound = Mix_LoadWAV(std::string("/sound") + nym)) == NULL)
        	return -1;
 
    	SoundList.push_back(TempSound);
 
    	return (SoundList.size() - 1);
	}

	void SoundManager::playAudio(){
		SDL_PauseAudio(false);
	}

	void SoundManager::pauseAudio(){
		SDL_PauseAudio(true);
	}

	void SoundManager::playMusic(const std::string& fileName){
		if (currentState != ERROR){
			// If no music is playing, play it
			if (Mix_PlayingMusic() == 0){
				// Load music
				try{
					Mix_Music* music = ResourceManager::getInstance()->acquireMusic(fileName.c_str());
					//Play music
					Mix_PlayMusic(music, -1);
					currentState = PLAYING;
					m_sCurrentMusicFilename = fileName.c_str();
				}
				catch (const Exception& e){
					std::cerr << e.what() << std::endl;
				}
			}else{
				// If music is playing, pause it
				this->pauseMusic();
			}
		}
	}

	void SoundManager::playClip(int clipID, bool loop){
		//start the clip
		if(clipID >= 0 && clipID < MAX_CLIPS)
			Mix_PlayChannel(-1, SoundList[clipID], loop);
		else
			return;
	}

	void SDLCALL SoundManager::mixAudio(void *unused, Uint8 *stream, Uint32 len){
		Uint32 chunklen;
		for(int slotID = 0; slotID <= MAX_CLIPS; slotID++)

	}

	void SoundManager::cleanup() {
		for(int i = 0;i < SoundList.size();i++)
        	Mix_FreeChunk(SoundList[i]);
 
    	SoundList.clear();
	}
}