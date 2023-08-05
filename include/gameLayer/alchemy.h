#pragma once
#include <glm/vec2.hpp>
#include <utility>

enum Elements
{
	Fire=0,
	Snow,
	Soil,
	Water,
	Cloud,
	Wind,
	Stone,
	Mud,
	Electricity,
	Acorn,
	Leaf,
	Tree,
	Forest,
	Mountain,
	River,
	Sea,
	Algae,
	Ash,
	House,
	Ice,
	Iron,
	Sand,
	SnowFall,
	Grass,
	Bricks,
	ELEMENTS_COUNT
};

constexpr char* names[] = {"fire.png", "snow.png", "soil.png", "water.png", "cloud.png", 
"wind.png", "stone.png", "mud.png", "electricity.png", "acorn.png", "leaf.png", "tree.png", "forest.png", "mountain.png",
"river.png", "sea.png", "algae.png", "ash.png", "house.png", "ice.png", "iron.png",
	"sand.png", "snowFall.png", "grass.png", "bricks.png"
};


struct Recepie
{
	int a;
	int b;
	int rez;
};

constexpr Recepie recepies[] = {
	{Fire, Snow, Water},
	{Fire, Water, Cloud},
	{Cloud, Cloud, Wind},
	{Fire, Soil, Stone},
	{Water, Soil, Mud},
	{Water, Cloud, Electricity},
	{Wind, Cloud, Electricity},
	{Electricity, Soil, Acorn},
	{Acorn, Water, Tree},
	{Tree, Electricity, Leaf},
	{Tree, Tree, Forest},
	{Stone, Soil, Mountain},
	{ Water, Wind, River },
	{ Water, Sea, River },
	{Water, Water, Sea},
	{Fire, Mud, Bricks},
	{Bricks, Bricks, House},
	{Fire, Stone, Iron},
	{Water, Snow, Ice},
	{Cloud, Snow, SnowFall},
	{Leaf, Soil, Grass},
	{Wind, Soil, Sand},
	{Fire, Leaf, Ash},
	{Fire, Tree, Ash},
	{Fire, Algae, Ash},
	{Fire, Forest, Ash},
	{Fire, Grass, Ash},
	{Sea, Acorn, Algae},
	{Sea, Leaf, Algae},
	{Water, Stone, Leaf},
	{Ice, Fire, Water}
};



struct Element
{
	glm::vec2 position = {};
	int type = 0;
	float rotationTime = 0;
};


enum Planets
{
	Empty = 0,
	Tundra,
	Plains,
	SnowMountains,
	ForestPlanet,
	ForestWater,
	WasteLand,
	RiverPlanet,
	Delta,
	Litoral,
	Swamp,
	Marea,
	Desert,
	DryLand,
	MountainSmall,
	City,
	Galati,
	PLANETS_SIZE
};

constexpr char* planetNames[] = {"planet.glb", "tundra.glb", "plains.glb", "snowMountains.glb", "Forest.glb", "ForestWater.glb",
"wasteLand.glb", "river.glb", "delta.glb", "litoral.glb", "swamp.glb", "marea.glb", "desert.glb", "dryLand.glb", "mountainSmall.glb",
"city.glb", "galati.glb"};

constexpr char* planetImages[] = {"planet.jpg", "tundra.jpg", "plains.jpg", "snowMountains.jpg", "Forest.png", "ForestWater.jpg",
"wasteLand.png", "river.jpg", "delta.jpg", "litoral.jpg", "swamp.png", "marea.jpg", "desert.jpg", "dryLand.jpg", "mountainSmall.jpg",
"city.jpg", "galati.png"};

constexpr Recepie planetRecepies[] =
{
	{Mountain, Snow, Planets::SnowMountains},
	{Mountain, Ice, Planets::SnowMountains},
	{Mountain, SnowFall, Planets::SnowMountains},
	{Mountain, Wind, Planets::SnowMountains},

	{Mountain, River, Planets::MountainSmall},
	{Mountain, Soil, Planets::MountainSmall},
	{Mountain, Forest, Planets::MountainSmall},
	
	{Soil, Water, Planets::Plains },
	{Grass, Water, Planets::Plains },
	{Soil, Tree, Planets::Plains },
	{Soil, Grass, Planets::Plains },

	{Sand, Grass, Planets::DryLand},
	{Soil, Stone, Planets::DryLand},
	{Grass, Stone, Planets::DryLand},
	
	{Soil, Snow, Planets::Tundra},
	{Tree, Snow, Planets::Tundra},
	
	{Forest, Soil, Planets::ForestPlanet},
	{Tree, Tree, Planets::ForestPlanet},
	{Tree, Grass, Planets::ForestPlanet},
	{Forest, Grass, Planets::ForestPlanet},

	{Forest, Water, Planets::ForestWater},

	{Forest, Fire, Planets::WasteLand},
	{Fire, Fire, Planets::WasteLand},
	{Ash, Fire, Planets::WasteLand},
	{Ash, Soil, Planets::WasteLand},
	{Ash, Ash, Planets::WasteLand},
	
	{River, Forest, Planets::RiverPlanet},
	{River, Soil, Planets::RiverPlanet},

	{Sea, River, Planets::Delta},

	{Soil, Sea, Planets::Litoral},
	{Sand, Sea, Planets::Litoral},

	{Mud, Soil, Planets::Swamp},
	{Mud, Tree, Planets::Swamp},
	{Mud, Forest, Planets::Swamp},
	{Mud, River, Planets::Swamp},
	{Mud, Grass, Planets::Swamp},
	{Mud, Water, Planets::Swamp},

	{Sea, Sea, Planets::Marea},
	{Sea, Algae, Planets::Marea},

	{Sand, Soil, Planets::Desert},
	{Sand, Wind, Planets::Desert},

	{House, House, Planets::City},
	{House, Soil, Planets::City},

	{House, Iron, Planets::Galati},
	{Mountain, Iron, Planets::Galati},

};
constexpr char* planetDescription[PLANETS_SIZE] =
{
	R"(Combine elements outside the planet to make new elements,
than combine elements inside the planet to change the biome!
)",

	R"(The word tundra comes from a Finnish word tunturi, 
which means treeless plain or barren land.

)",


	R"(Plains are one of the major landforms, on Earth,
covering more than one-third of the world's land area!
)",

	R"(Mountains exist on every continent
 and even beneath oceans!)",

	R"(
Forests are home to over 80 percent of land animals and plants and
 cover 31 percent of the world's total land area.
	)",

R"(
Studies show spending time in a forest boosts immune system activity.
https://www.inc.com/jessica-stillman/new-book-trees-are-basically-a-wonder-drug.html
)",

"Deserts take up 1/5 of the total globe space!",

R"(The Nile River is widely accepted as the world's longest river!)",

R"(The shortest river in the world is the Roe River, 61 meters!)",

R"(Sand comes from broken down minerals and rocks!)",

R"(Swamps can be found on all continents except Antarctica.)",

R"(The ocean covers 71% of the Earth's surface.)",

R"(Not all Deserts Have Sand, there are also cold deserts!)",

R"(Drylands are areas that suffer from high water scarcity.
 They cover 41 percent of the planet's land area and are found on every continent)",

R"(Thirty of the world's highest mountains are in the Himalaya.
The summit of Mount Everest, at 29,035 feet (8,850 meters), 
is the highest point on Earth.
The tallest mountain measured from top to bottom is Mauna Kea,
an inactive volcano on the island of Hawaii in the Pacific Ocean.)",

R"(Rio de Janeiro's name means River of January in Portuguese)",

R"(Only my Romanian friends will get this joke.)",



};