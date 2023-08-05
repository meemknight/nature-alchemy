#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include <raudio.h>
#include "gl3d/gl3d.h"
#include "alchemy.h"
#include <vector>
#include "myMath.h"
#include <sstream>

gl2d::Renderer2D renderer2d;
gl3d::Renderer3D renderer3d;
gl2d::Font font;

gl3d::Model planets[PLANETS_SIZE];
gl3d::Model cloudsModel;
//gl3d::Model birbModel;
gl3d::Entity currentEntity;
gl3d::Entity clouds;
//gl3d::Entity birb;
gl3d::SpotLight light;
//gl3d::DirectionalLight light;

Music music;

gl2d::Texture elementSprites[Elements::ELEMENTS_COUNT] = {};
gl2d::Texture bookSprite;
gl2d::Texture panda;
gl2d::Texture planetSprites[Planets::PLANETS_SIZE] = {};
gl2d::Texture soundOnSprite;
gl2d::Texture soundOffSprite;

std::vector<Element> droppedElements;
std::vector<int> planetElements;

Element currentElement;
bool holdingElement = 0;

int currentPlannetType = 0;
bool readingMode = 0;
bool mute = 0;

gl2d::ParticleSystem ps;
gl2d::ParticleSettings particle;

struct GameData
{
	unsigned char unlockedElements[Elements::ELEMENTS_COUNT] = {1, 1, 1};
	unsigned char unlockedBiomes[Planets::PLANETS_SIZE] = {1, 0};
	unsigned char readBiomes[Planets::PLANETS_SIZE] = {0};
	float angle = glm::radians(20.f);
	float angle2 = glm::radians(10.f);

}gameData;



#define USE_GPU_ENGINE 1

#undef min
#undef max

#pragma region gpu
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}
#pragma endregion


glm::vec3 sunPosition;

bool initGame()
{
	renderer2d.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	renderer3d.init(1, 1, RESOURCES_PATH"/BRDFintegrationMap.png");
	//renderer3d.skyBox = renderer3d.loadSkyBox(RESOURCES_PATH "sky.png", 0);
	//renderer3d.skyBox.color = {1,1,1};

	sunPosition = gl3d::fromAnglesToDirection(gameData.angle, gameData.angle2);
	renderer3d.skyBox = renderer3d.atmosfericScattering(sunPosition,
		glm::vec3(0.255f, 0.425f, 0.441f),
		glm::vec3(0.098f, 0.091f, 0.014f), {}, false,
		0.7);

	renderer3d.internal.lightShader.lightPassUniformBlockCpuData.bloomTresshold = 0.85;

	for (int i = 0; i < PLANETS_SIZE; i++)
	{
		std::string path = RESOURCES_PATH "/biomes/";
		planets[i] = renderer3d.loadModel((path + planetNames[i]).c_str(), gl3d::maxQuality, 1.f);

	}

	
	currentEntity = renderer3d.createEntity(planets[0]);
	cloudsModel = renderer3d.loadModel(RESOURCES_PATH "/clouds.glb", gl3d::maxQuality, 1.f);
	clouds = renderer3d.createEntity(cloudsModel);

	//birbModel = renderer3d.loadModel(RESOURCES_PATH "/birb.glb");
	//birb = renderer3d.createEntity(birbModel);
	//renderer3d.setEntityAnimate(birb, 1);
	//renderer3d.setEntityAnimationSpeed(birb, 1.f);


	light = renderer3d.createSpotLight(sunPosition*6.f, glm::radians(80.f), -sunPosition, 25, 1, {10,10,10}, 1.3f);
	//light = renderer3d.createDirectionalLight(-sunPosition, {10,10,10}, 1.3f);


	//renderer3d.directionalShadows.CASCADES


	for (int i = 0; i < Elements::ELEMENTS_COUNT; i++)
	{
		std::string path = RESOURCES_PATH "/icons/";
		elementSprites[i].loadFromFile((path + names[i]).c_str());
	}

	for (int i = 0; i < Planets::PLANETS_SIZE; i++)
	{
		std::string path = RESOURCES_PATH "/photos/";
		planetSprites[i].loadFromFile((path + planetImages[i]).c_str());
	}


	bookSprite.loadFromFile(RESOURCES_PATH "icons/book.png");
	panda.loadFromFile(RESOURCES_PATH "wwf.png");

	soundOnSprite.loadFromFile(RESOURCES_PATH "on.png");
	soundOffSprite.loadFromFile(RESOURCES_PATH "off.png");

	//renderer3d.setExposure(1.8);

	planetElements.resize(2, - 1);

	music = LoadMusicStream(RESOURCES_PATH "Cody_G_-_ukulele_and_chill.ogg");
	UpdateMusicStream(music);
	PlayMusicStream(music);
	SetMusicVolume(music, 0.3);

	ps.initParticleSystem(50);

	particle.positionX = {-40,40};
	particle.positionY = {-40,40};
	particle.particleLifeTime = {1,2};
	particle.directionX = {-50,50};
	particle.directionY = {-50,50};
	particle.createApearence.size = {25, 25};
	particle.dragX = {0,0};
	particle.dragY = {0,0};
	particle.rotation = {0, 360};
	particle.rotationSpeed = {-50, 50};
	particle.rotationDrag = {0, 0};
	particle.createApearence.color1 = {1, 1, 1, 0.9};
	particle.createApearence.color2 = {1, 1, 1, 1};
	particle.createEndApearence.color1 = {1, 1, 1, 0.8};
	particle.createEndApearence.size = {40,45};
	particle.tranzitionType = gl2d::TRANZITION_TYPES::abruptCurbe;
	particle.onCreateCount = 5;
	particle.texturePtr = &panda;

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w= platform::getWindowSizeX();
	h = platform::getWindowSizeY();
	
	renderer2d.updateWindowMetrics(w, h);
	renderer2d.clearScreen();
	renderer3d.updateWindowMetrics(w, h);

	UpdateMusicStream(music);
#pragma endregion

	const glm::vec2 currentMousePos = platform::getRelMousePosition();


	float posCumulated = 0.f;
	constexpr float sizePerc = 0.049;
	constexpr float sizePercDelim = 0.02;
	float size = sizePerc * h;
	float sizeDelim = sizePercDelim * h;
	int mouseIn = -1;
	int mouseInRight = -1;
	float barSize = 2 * size + sizeDelim * 3.f;
	float posXCumulated = sizeDelim;

	glm::vec4 bookPos = {sizeDelim, h - size - sizeDelim ,size,size};
	glm::vec4 mutePos = {sizeDelim + sizeDelim + size, h - size - sizeDelim ,size,size};
	bool recalculateBiome = 0;
	static glm::vec2 velocity = {};


	auto cloudTransform = renderer3d.getEntityTransform(clouds);
	cloudTransform.rotation.y += 0.1 * deltaTime;
	renderer3d.setEntityTransform(clouds, cloudTransform);

	//auto birbTransform = renderer3d.getEntityTransform(birb);
	//birbTransform.position.y = 2;
	//renderer3d.setEntityTransform(birb, birbTransform);


	{
		static float skyTime = 0;
		skyTime -= deltaTime;

		if (skyTime < 2)
		{
			skyTime += 2;
			

			gameData.angle += glm::radians(2.f);
			gameData.angle2 += glm::radians(0.5f);

			sunPosition = gl3d::fromAnglesToDirection(gameData.angle, gameData.angle2);

			if (gameData.angle > glm::radians(75.f))
			{
				gameData.angle = -glm::radians(75.f);
			}

			float lightPower = glm::dot(sunPosition, glm::vec3{0,1,0});

			glm::vec3 color1 = glm::vec3(0.255f, 0.425f, 0.441f) * lightPower;
			glm::vec3 color2 = glm::vec3(0.228f, 0.083f, 0.392f) * lightPower;

			auto lerp = [](glm::vec3 x, glm::vec3 y, float a)
			{
				return x * (1 - a) + (y * a);
			};

			renderer3d.skyBox.clearTextures();
			renderer3d.skyBox = renderer3d.atmosfericScattering(sunPosition,
				lerp(color2, color1, lightPower),
				glm::vec3(0.098f, 0.091f, 0.014f) * lightPower, {}, 0, 
				0.7);

			renderer3d.setSpotLightPosition(light, sunPosition * 6.f);
			renderer3d.setSpotLightDirection(light, -sunPosition);
		}


	}


	auto rotate = []()
	{
		return std::sin(clock() / (CLOCKS_PER_SEC * 0.1f)) * 25.f;
	};

	auto rotateFast = []()
	{
		return std::sin(clock() / (CLOCKS_PER_SEC * 0.02f)) * 15.f;
	};

#pragma region input
	{
		float speed = 12 * deltaTime;
		auto& camera = renderer3d.camera;
		camera.aspectRatio = (float)w / h;

		glm::vec2 drag = {0.85, 0.85};

		static glm::vec2 angleFromPlanet = {};
		static float distanceFromPlanet = 3.f;

		if (platform::isKeyHeld(platform::Button::W))
		{
			distanceFromPlanet -= deltaTime*7;
		}
		else if (platform::isKeyHeld(platform::Button::S))
		{
			distanceFromPlanet += deltaTime*7;
		}

		distanceFromPlanet = std::min(distanceFromPlanet, 10.f);
		distanceFromPlanet = std::max(distanceFromPlanet, 2.f);

		glm::vec2 delta = {};
		{
			static glm::vec2 lastMousePos = {};
			if (platform::isRMouseHeld() || platform::isRMousePressed() || platform::isRMouseReleased())
			{

				delta = lastMousePos - currentMousePos;
				
			}
			lastMousePos = currentMousePos;
		}

		if (glm::length(velocity) < 1.f)
		{
			float speed = 0.1f;
			velocity += delta * speed * deltaTime;
		}
		
		velocity *= drag;
		
		angleFromPlanet -= velocity;

		angleFromPlanet.y = std::min(angleFromPlanet.y, 3.141592f / 2.f);
		angleFromPlanet.y = std::max(angleFromPlanet.y, -3.141592f / 2.f);

		camera.position.x = std::cos(angleFromPlanet.x) * std::cos(angleFromPlanet.y);
		camera.position.z = std::sin(angleFromPlanet.x) * std::cos(angleFromPlanet.y);
		camera.position.y = std::sin(angleFromPlanet.y);
		camera.position = glm::normalize(camera.position);
		camera.position *= distanceFromPlanet;

		camera.viewDirection = glm::normalize(-camera.position);

		//auto t = renderer3d.getEntityTransform(currentEntity);
		//t.rotation.y -= velocity.x;
		//t.rotation.x += velocity.y;
		//renderer3d.setEntityTransform(currentEntity, t);

	}

	if (platform::isKeyPressedOn(platform::Button::F))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}

#pragma endregion

	if (readingMode)
	{
		gameData.readBiomes[currentPlannetType] = 1;

		renderer2d.renderRectangle({0,0,w,h}, {0.1,0.1,0.1,0.1});


		renderer2d.renderRectangle(bookPos, {}, 0.f, bookSprite);

		glm::vec2 size = planetSprites[currentPlannetType].GetSize();
		renderer2d.renderRectangle({100, h / 3, size.x / 2, size.y / 2}, {}, {}, planetSprites[currentPlannetType]);

		renderer2d.renderText({10,50}, planetDescription[currentPlannetType], font, Colors_White, w * 0.0003, 4.0, 3, false);


		if (pointInRect(bookPos, currentMousePos) && platform::isLMouseReleased())
		{
			readingMode = 0;
		}
		

		//planetSprites

		{
			int foundCount = 0;
			for (int i = 0; i < ELEMENTS_COUNT; i++)
			{
				foundCount += gameData.unlockedElements[i];
			}

			std::string text = "Elements found: " + std::to_string(foundCount) + "/" + std::to_string(ELEMENTS_COUNT);

			auto textSize = renderer2d.getTextSize(text.c_str(), font, w * 0.0003, 4.0, 3);
			renderer2d.renderText({w - textSize.x,h - textSize.y}, text.c_str(), font, Colors_White, w * 0.0003, 4.0, 3, false);

			foundCount = 0;
			for (int i = 0; i < PLANETS_SIZE; i++)
			{
				foundCount += gameData.unlockedBiomes[i];
			}

			text = "Discovered biomes: " + std::to_string(foundCount) + "/" + std::to_string(PLANETS_SIZE);

			auto textSize2 = renderer2d.getTextSize(text.c_str(), font, w * 0.0003, 4.0, 3);
			renderer2d.renderText({w - textSize2.x,h - textSize.y - textSize2.y},
				text.c_str(), font, Colors_White, w * 0.0003, 4.0, 3, false);


		}


	}
	else
	{


		renderer2d.renderRectangle({0,0, barSize, h}, {0.1f,0.1f,0.1f,0.1f});

		int showCount = 0;
		for (int i = 0; i < Elements::ELEMENTS_COUNT; i++)
		{

			if (gameData.unlockedElements[i])
			{
				showCount++;
				if (showCount >= 14)
				{
					showCount = 0;
					posXCumulated += size + sizeDelim;
					posCumulated = 0;
				}

				posCumulated += sizeDelim;

				glm::vec4 rect{posXCumulated, posCumulated, size, size};

				renderer2d.renderRectangle(rect, {}, 0.f, elementSprites[i]);
				posCumulated += size;

				if (pointInRect(rect, currentMousePos))
				{
					mouseIn = i;
				}
			}


		}

		{

			renderer2d.renderRectangle({w / 2 - size - sizeDelim*2, h - sizeDelim*2 - size, size * planetElements.size() + sizeDelim*3, size + sizeDelim*2},
				{0.1f,0.1f,0.1f,0.1f});
			float posY = h - sizeDelim - size;
			float posX = w / 2 - size - sizeDelim;

			for (int i = 0; i < planetElements.size(); i++)
			{

				glm::vec4 rect{posX, posY, size, size};
				glm::vec4 rect2{posX, posY, size+sizeDelim, size + sizeDelim};
				if (pointInRect(rect2, currentMousePos))
				{
					mouseInRight = i;
				}

				if (planetElements[i] >= 0)
				{ 

					renderer2d.renderRectangle(rect, {}, 0.f, elementSprites[planetElements[i]]);
				
				}

				posX += sizeDelim + size;

			}
		}

		{
			if (gameData.readBiomes[currentPlannetType] == 0)
			{
				renderer2d.renderRectangle(bookPos, {}, rotate(), bookSprite);
			}
			else
			{
				renderer2d.renderRectangle(bookPos, {}, 0.f, bookSprite);
			}


			if (pointInRect(bookPos, currentMousePos) && platform::isLMouseReleased())
			{
				readingMode = 1;
			}
		}

		//if(holdingElement)
		//{
		//	renderer2d.renderRectangle({w / 4.f, h / 4.f, w / 2.f, h / 2.f}, {0.1,0.1,0.1,0.1});
		//}

		{

			if (mute)
			{
				renderer2d.renderRectangle(mutePos, {}, 0.f, soundOffSprite);
			}
			else
			{
				renderer2d.renderRectangle(mutePos, {}, 0.f, soundOnSprite);
			}


			if (pointInRect(mutePos, currentMousePos) && platform::isLMouseReleased())
			{
				if (mute)
				{
					PlayMusicStream(music);
				}
				else
				{
					PauseMusicStream(music);
				}

				mute = !mute;
			}
		}


		for (int i = 0; i < droppedElements.size(); i++)
		{
			if (droppedElements[i].rotationTime > 0)
			{
				droppedElements[i].rotationTime -= deltaTime;

				renderer2d.renderRectangle({droppedElements[i].position.x, droppedElements[i].position.y, size, size},
					{}, rotateFast(), elementSprites[droppedElements[i].type]);
			}
			else
			{
				renderer2d.renderRectangle({droppedElements[i].position.x, droppedElements[i].position.y, size, size},
					{}, 0.f, elementSprites[droppedElements[i].type]);
			}


		}

	#pragma region block move logic
		static bool droppedHere = 0;

		if (!holdingElement)
		{
			if (platform::isLMousePressed())
			{
				if (mouseIn > -1)
				{
					holdingElement = true;
					currentElement.type = mouseIn;
				}
				else if (mouseInRight > -1)
				{
					if (planetElements[mouseInRight] > -1)
					{
						holdingElement = true;
						currentElement.type = planetElements[mouseInRight];
						planetElements[mouseInRight] = -1;
						recalculateBiome = true;
					}
				}
				else
				{
					//search from board
					for (int i = droppedElements.size() - 1; i >= 0; i--)
					{
						glm::vec4 rect = {droppedElements[i].position, size, size};
						if (pointInRect(rect, currentMousePos))
						{
							holdingElement = true;
							currentElement.type = droppedElements[i].type;
							droppedElements.erase(droppedElements.begin() + i);
							break;
						}
					}
				}
			}
		}

		bool droppedThisFrame = 0;

		if (holdingElement)
		{
			if (!platform::isLMouseHeld())
			{
				holdingElement = 0;

				if (mouseInRight > -1)
				{
					planetElements[mouseInRight] = currentElement.type;
					recalculateBiome = true;
					droppedHere = false;
				}
				else if (pointInRect({w / 4.f,h / 4.f,w / 2.f,h / 2.f}, currentMousePos))
				{
					if (droppedHere || (planetElements[1] < 0 && planetElements[0] >= 0))
					{
						planetElements[1] = currentElement.type;
						recalculateBiome = true;
					}else
					{
						planetElements[0] = currentElement.type;
						planetElements[1] = -1;
						droppedHere = true;
					}
				}
				else
				{
					if (currentMousePos.x - size / 2.f < barSize)
					{
						//delete item
					}
					else
					{
						droppedElements.push_back(currentElement);
						droppedThisFrame = true;
						droppedHere = false;

					}
				}

				

			}
			else
			{

				currentElement.position = currentMousePos;
				currentElement.position -= glm::vec2{size / 2.f, size / 2.f};

			}


			renderer2d.renderRectangle({currentElement.position.x, currentElement.position.y, size * 1.1, size * 1.1},
				{}, 0.f, elementSprites[currentElement.type]);

		}


		if (droppedThisFrame)
		{
			auto element = droppedElements.back();
			bool foundOne = false;
			bool match = false;
			int index = 0;

			for (int i = 0; i < droppedElements.size() - 1; i++)
			{
				if (aabb({element.position,size * 0.9, size * 0.9}, {droppedElements[i].position,size * 0.9, size * 0.9}))
				{
					foundOne = true;

					for (int r = 0; r < sizeof(recepies) / sizeof(recepies[0]); r++)
					{
						if
							(
							(recepies[r].a == element.type && recepies[r].b == droppedElements[i].type) ||
							(recepies[r].b == element.type && recepies[r].a == droppedElements[i].type)
							)
						{
							droppedElements[i].type = recepies[r].rez;
							droppedElements.pop_back();
							if (!gameData.unlockedElements[recepies[r].rez])
							{
								//todo particles
								gameData.unlockedElements[recepies[r].rez] = 1;
								
								
							}

							ps.emitParticleWave(&particle, currentMousePos);
							//todo add particles
							match = true;
							break;
						}
					}
				}
			}

			if (foundOne && !match)
			{
				droppedElements.back().rotationTime = 0.2f;
			}
		}



		if (recalculateBiome)
		{
			droppedHere = false;

			int type = 0;
			for (int i = 0; i < sizeof(planetRecepies) / sizeof(planetRecepies[0]); i++)
			{

				if (
					(planetRecepies[i].a == planetElements[0] &&
					planetRecepies[i].b == planetElements[1]) ||
					(planetRecepies[i].b == planetElements[0] &&
					planetRecepies[i].a == planetElements[1])
					)
				{
					type = planetRecepies[i].rez;
					
					if(gameData.unlockedBiomes[type] == false)
					{
						gameData.unlockedBiomes[type] = true;
					}
					
					break;
				}

			}

			if (!type)
			{

			}
			else
			{
				renderer3d.setEntityModel(currentEntity, planets[type]);
				renderer3d.setEntityAnimate(currentEntity, true);
				renderer3d.setEntityAnimationIndex(currentEntity, 0);

				currentPlannetType = type;
				velocity.x -= deltaTime * 100;
			}

		}


	#pragma endregion

	}


#pragma region set finishing stuff
	
	renderer3d.render(deltaTime);

	
	gl2d::enableNecessaryGLFeatures();
	
	
	ps.applyMovement(deltaTime);
	//renderer2d.flush();
	ps.postProcessing = false;
	ps.draw(renderer2d);
	renderer2d.flush();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
